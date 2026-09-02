// Copyright Epic Games, Inc. All Rights Reserved.

#include "CentralTower.h"
#include "GADE7322POE.h"
#include "HealthComponent.h"
#include "TowerDefenseGameMode.h"
#include "TowerDefenseGameState.h"
#include "TowerDefenseTypes.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Math/NumericLimits.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ACentralTower::ACentralTower()
{
	PrimaryActorTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	AttackRange = 800.0f;
	AttackDamage = 25.0f;
	AttackCooldown = 1.0f;
	bDrawAttackRange = true;

	Tags.Add(TowerDefenseTags::Tower);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMesh.Object);
	}

	const FVector MeshScale(1.6f, 1.6f, 2.8f);
	MeshComponent->SetRelativeScale3D(MeshScale);
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f * MeshScale.Z));

	AttackRangeSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AttackRangeSphere"));
	AttackRangeSphere->SetupAttachment(SceneRoot);
	AttackRangeSphere->SetSphereRadius(AttackRange);
	AttackRangeSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AttackRangeSphere->SetCollisionObjectType(ECC_WorldDynamic);
	AttackRangeSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	AttackRangeSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	AttackRangeSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	AttackRangeSphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	AttackRangeSphere->SetGenerateOverlapEvents(true);
	AttackRangeSphere->SetHiddenInGame(true);

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void ACentralTower::BeginPlay()
{
	Super::BeginPlay();

	if (AttackRangeSphere)
	{
		AttackRangeSphere->SetSphereRadius(AttackRange);
		AttackRangeSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::HandleAttackRangeOverlap);
		AttackRangeSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::HandleAttackRangeEndOverlap);
	}

	if (HealthComponent)
	{
		HealthComponent->InitializeHealth(MaxHealth);
		HealthComponent->OnDeath.AddDynamic(this, &ThisClass::Die);
		HealthComponent->OnHealthChanged.AddDynamic(this, &ThisClass::HandleHealthChanged);
		SyncHealthToGameState();
	}

	StartAttackTimer();

	if (bDrawAttackRange)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), AttackRange, 24, FColor::Red, true, -1.0f, 0, 4.0f);
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Central tower spawned at %s with %.0f health and %.0f attack range."),
		*GetActorLocation().ToCompactString(), MaxHealth, AttackRange);
}

void ACentralTower::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCombat();
	Super::EndPlay(EndPlayReason);
}

void ACentralTower::FindTarget()
{
	CurrentTarget = nullptr;

	float ClosestDistanceSq = TNumericLimits<float>::Max();
	for (int32 Index = TargetsInRange.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = TargetsInRange[Index].Get();
		if (!IsValidTarget(Candidate))
		{
			TargetsInRange.RemoveAtSwap(Index);
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			CurrentTarget = Candidate;
		}
	}

	if (CurrentTarget.IsValid())
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("Tower acquired target '%s'."), *GetNameSafe(CurrentTarget.Get()));
	}
}

void ACentralTower::AttackTarget()
{
	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		if (GameState->GetMatchState() != ETowerDefenseMatchState::InProgress)
		{
			return;
		}
	}

	if (!IsValidTarget(CurrentTarget.Get()) || !IsTargetInRange(CurrentTarget.Get()))
	{
		FindTarget();
	}

	AActor* Target = CurrentTarget.Get();
	if (!IsValidTarget(Target))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, GetInstigatorController(), this, UDamageType::StaticClass());
	UE_LOG(LogTowerDefense, Log, TEXT("Tower attacked '%s' for %.0f damage."), *GetNameSafe(Target), AttackDamage);
}

void ACentralTower::Die(AActor* DeadActor)
{
	UE_LOG(LogTowerDefense, Log, TEXT("Central tower has been destroyed."));
	StopCombat();

	if (UWorld* World = GetWorld())
	{
		if (ATowerDefenseGameMode* GameMode = World->GetAuthGameMode<ATowerDefenseGameMode>())
		{
			GameMode->HandleTowerDestroyed();
		}
	}
}

void ACentralTower::HandleHealthChanged(float CurrentHealth, float MaxHealthValue)
{
	SyncHealthToGameState();
	UE_LOG(LogTowerDefense, Log, TEXT("Tower health: %.0f / %.0f"), CurrentHealth, MaxHealthValue);
}

void ACentralTower::HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidTarget(OtherActor))
	{
		return;
	}

	TargetsInRange.AddUnique(OtherActor);
	if (!CurrentTarget.IsValid())
	{
		FindTarget();
	}
}

void ACentralTower::HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TargetsInRange.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Target)
	{
		return !Target.IsValid() || Target.Get() == OtherActor;
	});

	if (CurrentTarget.Get() == OtherActor)
	{
		CurrentTarget = nullptr;
		FindTarget();
	}
}

void ACentralTower::StopCombat()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}

	CurrentTarget = nullptr;
	TargetsInRange.Reset();
}

bool ACentralTower::IsValidTarget(AActor* Actor) const
{
	if (!IsValid(Actor) || Actor == this)
	{
		return false;
	}

	if (!Actor->ActorHasTag(TowerDefenseTags::Enemy))
	{
		return false;
	}

	const UHealthComponent* TargetHealth = Actor->FindComponentByClass<UHealthComponent>();
	return TargetHealth && !TargetHealth->IsDead();
}

bool ACentralTower::IsTargetInRange(AActor* Actor) const
{
	if (!IsValid(Actor))
	{
		return false;
	}

	return FVector::DistSquared(GetActorLocation(), Actor->GetActorLocation()) <= FMath::Square(AttackRange);
}

void ACentralTower::StartAttackTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AttackTimerHandle, this, &ThisClass::AttackTarget, AttackCooldown, true);
	}
}

void ACentralTower::SyncHealthToGameState() const
{
	if (!HealthComponent)
	{
		return;
	}

	if (ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		GameState->SetTowerHealth(HealthComponent->GetCurrentHealth(), HealthComponent->GetMaxHealth());
	}
}
