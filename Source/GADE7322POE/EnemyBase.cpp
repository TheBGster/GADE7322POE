// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemyBase.h"
#include "GADE7322POE.h"
#include "CentralTower.h"
#include "CombatStatusWidget.h"
#include "HealthComponent.h"
#include "TowerDefenseGameMode.h"
#include "TowerDefenseGameState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
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
	ResourceReward = 10;
	CurrentWaypointIndex = 0;
	BehaviorState = EEnemyBehaviorState::Moving;
	bHasReachedDestination = false;
	bHasGrantedKillReward = false;
	EnemyKind = EEnemyKind::JungleScout;
	DisplayName = FText::FromString(TEXT("Jungle Scout"));
	BodyColor = FLinearColor(0.18f, 0.38f, 0.16f, 1.0f);
	SlowMultiplier = 1.0f;
	SpeedBurstMultiplier = 1.0f;

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

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarComponent"));
	HealthBarComponent->SetupAttachment(SceneRoot);
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawAtDesiredSize(false);
	HealthBarComponent->SetDrawSize(FVector2D(110.0f, 28.0f));
	HealthBarComponent->SetPivot(FVector2D(0.5f, 1.0f));
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 95.0f));
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (AttackRangeSphere)
	{
		AttackRangeSphere->SetSphereRadius(AttackRange);
		AttackRangeSphere->OnComponentBeginOverlap.AddUniqueDynamic(this, &ThisClass::HandleAttackRangeOverlap);
		AttackRangeSphere->OnComponentEndOverlap.AddUniqueDynamic(this, &ThisClass::HandleAttackRangeEndOverlap);
	}

	if (HealthComponent)
	{
		HealthComponent->InitializeHealth(MaxHealth);
		HealthComponent->OnDeath.AddUniqueDynamic(this, &ThisClass::Die);
		HealthComponent->OnDamaged.AddUniqueDynamic(this, &ThisClass::HandleDamaged);
	}

	ApplyBodyColor();
	SetupHealthBar();
	StartAttackTimer();
}

void AEnemyBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopBehavior();

	if (AttackRangeSphere)
	{
		AttackRangeSphere->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::HandleAttackRangeOverlap);
		AttackRangeSphere->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::HandleAttackRangeEndOverlap);
	}

	if (HealthComponent)
	{
		HealthComponent->OnDeath.RemoveDynamic(this, &ThisClass::Die);
		HealthComponent->OnDamaged.RemoveDynamic(this, &ThisClass::HandleDamaged);
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
		for (TPair<int32, FTimerHandle>& Pair : SlowTimers)
		{
			World->GetTimerManager().ClearTimer(Pair.Value);
		}
	}

	SlowSources.Reset();
	SlowTimers.Reset();
	Super::EndPlay(EndPlayReason);
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsCombatAllowed())
	{
		return;
	}

	const bool bShouldMove = BehaviorState == EEnemyBehaviorState::Moving
		|| (ShouldKeepMovingWhileAttacking() && !bHasReachedDestination);
	if (bShouldMove)
	{
		MoveAlongPath(DeltaTime);
	}
}

void AEnemyBase::SetPath(const FGeneratedPath& Path)
{
	AssignedPath = Path;
	CurrentWaypointIndex = 0;
	bHasReachedDestination = false;
	TargetActor = nullptr;

	if (AssignedPath.Waypoints.Num() == 0)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Enemy '%s' received an empty path and will be removed."), *GetName());
		StopBehavior();
		Destroy();
		return;
	}

	BehaviorState = EEnemyBehaviorState::Moving;
	SetActorTickEnabled(true);
	SetActorLocation(GetWaypointWorldLocation(0));

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
	AddActorWorldOffset(Direction * GetEffectiveMoveSpeed() * DeltaTime, false);

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
	if (!IsCombatAllowed())
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

		if (!bHasReachedDestination && !ShouldAttackDefendersWhileMoving()
			&& Candidate->ActorHasTag(TowerDefenseTags::Defender))
		{
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
	if (!IsCombatAllowed())
	{
		return;
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
	GrantKillReward();
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

bool AEnemyBase::IsCombatAllowed() const
{
	if (IsDead())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const ATowerDefenseGameState* GameState = World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
	return GameState && GameState->IsMatchInProgress();
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
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(AttackTimerHandle);
	const float Interval = FMath::Max(AttackCooldown, 0.1f);
	World->GetTimerManager().SetTimer(AttackTimerHandle, this, &ThisClass::AttackTarget, Interval, true);
}

void AEnemyBase::GrantKillReward()
{
	if (bHasGrantedKillReward)
	{
		return;
	}

	bHasGrantedKillReward = true;

	ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
	if (!GameState)
	{
		return;
	}

	GameState->HandleEnemyKilled(ResourceReward);
}

void AEnemyBase::ApplyMovementSlow(int32 SourceId, float SlowPercent, float Duration)
{
	const float ClampedPercent = FMath::Clamp(SlowPercent, 0.0f, 0.85f);
	SlowSources.Add(SourceId, ClampedPercent);
	RecalculateSlow();

	UWorld* World = GetWorld();
	if (!World || Duration <= 0.0f)
	{
		return;
	}

	FTimerHandle& Timer = SlowTimers.FindOrAdd(SourceId);
	World->GetTimerManager().ClearTimer(Timer);
	World->GetTimerManager().SetTimer(Timer, FTimerDelegate::CreateWeakLambda(this, [this, SourceId]()
	{
		ClearMovementSlow(SourceId);
	}), Duration, false);
}

void AEnemyBase::ClearMovementSlow(int32 SourceId)
{
	SlowSources.Remove(SourceId);
	if (UWorld* World = GetWorld())
	{
		if (FTimerHandle* Timer = SlowTimers.Find(SourceId))
		{
			World->GetTimerManager().ClearTimer(*Timer);
		}
	}

	SlowTimers.Remove(SourceId);
	RecalculateSlow();
}

bool AEnemyBase::IsSlowed() const
{
	return GetMaxSlowPercent() > 0.0f;
}

float AEnemyBase::GetEffectiveMoveSpeed() const
{
	return MoveSpeed * SlowMultiplier * SpeedBurstMultiplier;
}

float AEnemyBase::GetMaxSlowPercent() const
{
	float MaxSlow = 0.0f;
	for (const TPair<int32, float>& Pair : SlowSources)
	{
		MaxSlow = FMath::Max(MaxSlow, Pair.Value);
	}

	return MaxSlow;
}

void AEnemyBase::RecalculateSlow()
{
	SlowMultiplier = 1.0f - GetMaxSlowPercent();
	if (MeshComponent && BodyMaterialInstance && IsSlowed())
	{
		BodyMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.75f, 0.95f, 1.0f));
		BodyMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.35f, 0.75f, 0.95f, 1.0f));
	}
	else
	{
		RestoreBodyColor();
	}
}

void AEnemyBase::HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy)
{
	PlayDamageFlash();
}

void AEnemyBase::PlayDamageFlash()
{
	if (!BodyMaterialInstance)
	{
		return;
	}

	BodyMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.15f, 0.10f, 1.0f));
	BodyMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.95f, 0.15f, 0.10f, 1.0f));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
		World->GetTimerManager().SetTimer(DamageFlashTimerHandle, this, &ThisClass::RestoreBodyColor, 0.12f, false);
	}
}

void AEnemyBase::RestoreBodyColor()
{
	if (!BodyMaterialInstance)
	{
		return;
	}

	const FLinearColor Color = IsSlowed()
		? FLinearColor(0.35f, 0.75f, 0.95f, 1.0f)
		: BodyColor;
	BodyMaterialInstance->SetVectorParameterValue(TEXT("Color"), Color);
	BodyMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), Color);
}

void AEnemyBase::ApplyBodyColor()
{
	if (!MeshComponent)
	{
		return;
	}

	UMaterialInterface* SourceMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!SourceMaterial)
	{
		return;
	}

	BodyMaterialInstance = UMaterialInstanceDynamic::Create(SourceMaterial, this);
	if (BodyMaterialInstance)
	{
		RestoreBodyColor();
		MeshComponent->SetMaterial(0, BodyMaterialInstance);
	}
}

void AEnemyBase::SetupHealthBar()
{
	if (!HealthBarComponent)
	{
		return;
	}

	HealthBarComponent->SetWidgetClass(UCombatStatusWidget::StaticClass());
	HealthBarComponent->InitWidget();
	if (UCombatStatusWidget* Status = Cast<UCombatStatusWidget>(HealthBarComponent->GetWidget()))
	{
		Status->BindToHealth(HealthComponent, DisplayName);
	}
}
