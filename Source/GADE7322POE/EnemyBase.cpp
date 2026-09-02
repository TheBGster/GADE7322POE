// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyBase.h"
#include "GADE7322POE.h"
#include "CentralTower.h"
#include "HealthComponent.h"
#include "TowerDefenseGameMode.h"
#include "TowerDefenseGameState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Math/NumericLimits.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	MaxHealth = 50.0f;
	MoveSpeed = 280.0f;
	WaypointAcceptanceRadius = 40.0f;
	AttackDamage = 8.0f;
	AttackRange = 180.0f;
	AttackCooldown = 1.0f;
	CurrentWaypointIndex = 0;
	BehaviorState = EEnemyBehaviorState::Moving;
	bHasReachedDestination = false;

	Tags.Add(TowerDefenseTags::Enemy);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
	}

	MeshComponent->SetRelativeScale3D(FVector(0.55f, 0.55f, 0.8f));
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 40.0f));

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

void AEnemyBase::BeginPlay()
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
	}

	StartAttackTimer();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBehavior();
	Super::EndPlay(EndPlayReason);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BehaviorState == EEnemyBehaviorState::Moving)
	{
		MoveAlongPath(DeltaTime);
	}
}

void AEnemyBase::SetPath(const FGeneratedPath& Path)
{
	AssignedPath = Path;
	CurrentWaypointIndex = 0;
	bHasReachedDestination = false;
	BehaviorState = EEnemyBehaviorState::Moving;
	TargetActor = nullptr;

	if (AssignedPath.Waypoints.Num() > 0)
	{
		SetActorLocation(GetWaypointWorldLocation(0));
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy '%s' assigned path %d with %d waypoints."),
		*GetName(), AssignedPath.PathID, AssignedPath.Waypoints.Num());
}

void AEnemyBase::StopBehavior()
{
	BehaviorState = EEnemyBehaviorState::Dead;
	TargetActor = nullptr;
	TargetsInRange.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}

	SetActorTickEnabled(false);
}

void AEnemyBase::MoveAlongPath(float DeltaTime)
{
	if (AssignedPath.Waypoints.Num() == 0 || !AssignedPath.Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		return;
	}

	const FVector CurrentLocation = GetActorLocation();
	const FVector TargetLocation = GetWaypointWorldLocation(CurrentWaypointIndex);
	FVector Direction = TargetLocation - CurrentLocation;
	Direction.Z = 0.0f;

	const float Distance = Direction.Size();
	if (Distance <= WaypointAcceptanceRadius)
	{
		AdvanceToNextWaypoint();
		return;
	}

	Direction.Normalize();
	AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, false);

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void AEnemyBase::AdvanceToNextWaypoint()
{
	++CurrentWaypointIndex;

	if (!AssignedPath.Waypoints.IsValidIndex(CurrentWaypointIndex))
	{
		bHasReachedDestination = true;
		CurrentWaypointIndex = AssignedPath.Waypoints.Num() - 1;
		BehaviorState = EEnemyBehaviorState::Attacking;
		TargetActor = GetCentralTowerActor();
		UE_LOG(LogTowerDefense, Log, TEXT("Enemy '%s' reached the tower and will start attacking."), *GetName());
	}
}

void AEnemyBase::FindTarget()
{
	if (IsDead())
	{
		return;
	}

	AActor* BestTarget = nullptr;
	float ClosestDistanceSq = TNumericLimits<float>::Max();

	for (int32 Index = TargetsInRange.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = TargetsInRange[Index].Get();
		if (!IsValidAttackTarget(Candidate))
		{
			TargetsInRange.RemoveAtSwap(Index);
			continue;
		}

		const float DistanceSq = FVector::DistSquared(GetActorLocation(), Candidate->GetActorLocation());
		if (DistanceSq < ClosestDistanceSq)
		{
			ClosestDistanceSq = DistanceSq;
			BestTarget = Candidate;
		}
	}

	if (BestTarget)
	{
		TargetActor = BestTarget;
		BehaviorState = EEnemyBehaviorState::Attacking;
		return;
	}

	if (bHasReachedDestination)
	{
		TargetActor = GetCentralTowerActor();
		BehaviorState = EEnemyBehaviorState::Attacking;
		return;
	}

	TargetActor = nullptr;
	BehaviorState = EEnemyBehaviorState::Moving;
}

void AEnemyBase::AttackTarget()
{
	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		if (GameState->GetMatchState() != ETowerDefenseMatchState::InProgress)
		{
			return;
		}
	}

	if (!IsValidAttackTarget(TargetActor.Get()))
	{
		FindTarget();
	}

	AActor* Target = TargetActor.Get();
	if (!IsValidAttackTarget(Target))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, nullptr, this, UDamageType::StaticClass());
	UE_LOG(LogTowerDefense, Log, TEXT("Enemy '%s' attacked '%s' for %.0f damage."),
		*GetName(), *GetNameSafe(Target), AttackDamage);
}

void AEnemyBase::Die(AActor* DeadActor)
{
	UE_LOG(LogTowerDefense, Log, TEXT("Enemy '%s' died on path %d."), *GetName(), AssignedPath.PathID);
	StopBehavior();
	Destroy();
}

void AEnemyBase::HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!IsValidAttackTarget(OtherActor))
	{
		return;
	}

	TargetsInRange.AddUnique(OtherActor);
	if (BehaviorState == EEnemyBehaviorState::Moving)
	{
		FindTarget();
	}
}

void AEnemyBase::HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	TargetsInRange.RemoveAll([OtherActor](const TWeakObjectPtr<AActor>& Target)
	{
		return !Target.IsValid() || Target.Get() == OtherActor;
	});

	if (TargetActor.Get() == OtherActor)
	{
		TargetActor = nullptr;
		FindTarget();
	}
}

bool AEnemyBase::IsDead() const
{
	return BehaviorState == EEnemyBehaviorState::Dead || (HealthComponent && HealthComponent->IsDead());
}

bool AEnemyBase::IsValidAttackTarget(AActor* Actor) const
{
	if (!IsValid(Actor) || Actor == this)
	{
		return false;
	}

	const bool bIsTower = Actor->ActorHasTag(TowerDefenseTags::Tower);
	const bool bIsDefender = Actor->ActorHasTag(TowerDefenseTags::Defender);
	if (!bIsTower && !bIsDefender)
	{
		return false;
	}

	const UHealthComponent* TargetHealth = Actor->FindComponentByClass<UHealthComponent>();
	return TargetHealth && !TargetHealth->IsDead();
}

AActor* AEnemyBase::GetCentralTowerActor() const
{
	const UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	if (const ATowerDefenseGameMode* GameMode = World->GetAuthGameMode<ATowerDefenseGameMode>())
	{
		return GameMode->GetCentralTower();
	}

	return nullptr;
}

FVector AEnemyBase::GetWaypointWorldLocation(int32 WaypointIndex) const
{
	if (!AssignedPath.Waypoints.IsValidIndex(WaypointIndex))
	{
		return GetActorLocation();
	}

	FVector Waypoint = AssignedPath.Waypoints[WaypointIndex];
	Waypoint.Z += 50.0f;
	return Waypoint;
}

void AEnemyBase::StartAttackTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AttackTimerHandle, this, &ThisClass::AttackTarget, AttackCooldown, true);
	}
}
