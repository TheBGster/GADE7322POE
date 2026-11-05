// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefenderBase.h"
#include "GADE7322POE.h"
#include "CombatStatusWidget.h"
#include "DefenderPlacementPoint.h"
#include "HealthComponent.h"
#include "JungleVisuals.h"
#include "TowerDefenseGameState.h"
#include "TowerDefenseTypes.h"
#include "TowerDefenseUpgrade.h"
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

ADefenderBase::ADefenderBase()
{
	PrimaryActorTick.bCanEverTick = true;
	AppliedUpgradeTier = 0;
	VisualSwayAmount = 0.0f;
	BaseMeshScale = FVector(1.0f, 1.0f, 1.5f);

	MaxHealth = 80.0f;
	AttackRange = 700.0f;
	AttackDamage = 20.0f;
	AttackCooldown = 0.8f;
	PlacementCost = 25;
	DefenderKind = EDefenderKind::JungleScout;
	DisplayName = FText::FromString(TEXT("Jungle Scout"));
	BodyColor = FLinearColor(0.62f, 0.48f, 0.16f, 1.0f);

	Tags.Add(TowerDefenseTags::Defender);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMesh.Object);
	}

	MeshComponent->SetRelativeScale3D(BaseMeshScale);
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f * BaseMeshScale.Z));

	CollarMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollarMesh"));
	CollarMesh->SetupAttachment(SceneRoot);
	CollarMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CollarMesh->SetHiddenInGame(true);
	CollarMesh->SetVisibility(false);
	CollarMesh->SetRelativeScale3D(FVector(1.35f, 1.35f, 0.18f));
	CollarMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 118.0f));
	if (CylinderMesh.Succeeded())
	{
		CollarMesh->SetStaticMesh(CylinderMesh.Object);
	}

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
	HealthBarComponent->SetDrawSize(FVector2D(120.0f, 28.0f));
	HealthBarComponent->SetPivot(FVector2D(0.5f, 1.0f));
	HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 170.0f));
	HealthBarComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADefenderBase::BeginPlay()
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

	if (MeshComponent)
	{
		BaseMeshScale = MeshComponent->GetRelativeScale3D();
	}

	ApplyBodyColor();
	SetupHealthBar();
	StartAttackTimer();

	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		SyncUpgradesToTier(GameState->GetUpgradeTier(TowerDefenseUpgrade::TrackFromDefenderKind(DefenderKind)));
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Defender '%s' placed at %s."), *GetName(), *GetActorLocation().ToCompactString());
}

void ADefenderBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (BodyMaterialInstance && GetWorld())
	{
		BodyMaterialInstance->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds());
		BodyMaterialInstance->SetScalarParameterValue(TEXT("SwayAmount"), VisualSwayAmount);
		BodyMaterialInstance->SetScalarParameterValue(TEXT("SwaySpeed"), 1.6f + AppliedUpgradeTier * 0.35f);
		BodyMaterialInstance->SetScalarParameterValue(TEXT("UpgradeTier"), static_cast<float>(AppliedUpgradeTier));
	}
}

void ADefenderBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopCombat();

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
	}

	if (ADefenderPlacementPoint* PlacementPoint = OwningPlacementPoint.Get())
	{
		PlacementPoint->NotifyDefenderDestroyed();
		OwningPlacementPoint = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ADefenderBase::SetOwningPlacementPoint(ADefenderPlacementPoint* PlacementPoint)
{
	OwningPlacementPoint = PlacementPoint;
}

void ADefenderBase::FindTarget()
{
	if (!IsCombatAllowed())
	{
		CurrentTarget = nullptr;
		return;
	}

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
}

void ADefenderBase::AttackTarget()
{
	if (!IsCombatAllowed())
	{
		return;
	}

	if (!IsValidTarget(CurrentTarget.Get()))
	{
		FindTarget();
	}

	AActor* Target = CurrentTarget.Get();
	if (!IsValidTarget(Target))
	{
		return;
	}

	UGameplayStatics::ApplyDamage(Target, AttackDamage, nullptr, this, UDamageType::StaticClass());
	UE_LOG(LogTowerDefense, Log, TEXT("Defender '%s' attacked '%s' for %.0f damage."),
		*GetName(), *GetNameSafe(Target), AttackDamage);
}

void ADefenderBase::Die(AActor* DeadActor)
{
	UE_LOG(LogTowerDefense, Log, TEXT("Defender '%s' was destroyed."), *GetName());
	StopCombat();

	if (ADefenderPlacementPoint* PlacementPoint = OwningPlacementPoint.Get())
	{
		OwningPlacementPoint = nullptr;
		PlacementPoint->NotifyDefenderDestroyed();
	}

	Destroy();
}

void ADefenderBase::HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void ADefenderBase::HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
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

void ADefenderBase::StopCombat()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimerHandle);
	}

	CurrentTarget = nullptr;
	TargetsInRange.Reset();
}

bool ADefenderBase::IsValidTarget(AActor* Actor) const
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

bool ADefenderBase::IsCombatAllowed() const
{
	if (HealthComponent && HealthComponent->IsDead())
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const ATowerDefenseGameState* GameState = World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
	return GameState && GameState->IsMatchInProgress();
}

void ADefenderBase::StartAttackTimer()
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

void ADefenderBase::HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy)
{
	PlayDamageFlash();
}

void ADefenderBase::PlayDamageFlash()
{
	if (!BodyMaterialInstance)
	{
		return;
	}

	BodyMaterialInstance->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.95f, 0.18f, 0.12f, 1.0f));
	BodyMaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), FLinearColor(0.95f, 0.18f, 0.12f, 1.0f));

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageFlashTimerHandle);
		World->GetTimerManager().SetTimer(DamageFlashTimerHandle, this, &ThisClass::RestoreBodyColor, 0.12f, false);
	}
}

void ADefenderBase::RestoreBodyColor()
{
	JungleVisuals::UpdateDynamicMaterial(
		BodyMaterialInstance,
		BodyColor,
		AppliedUpgradeTier,
		VisualSwayAmount,
		GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f);
}

void ADefenderBase::ApplyBodyColor()
{
	if (!MeshComponent)
	{
		return;
	}

	BodyMaterialInstance = JungleVisuals::ApplyBodyMaterial(
		MeshComponent,
		this,
		BodyColor,
		AppliedUpgradeTier,
		VisualSwayAmount);
}

void ADefenderBase::ApplyUpgradeStep(int32 TierIndex)
{
	if (TierIndex != AppliedUpgradeTier + 1)
	{
		return;
	}

	const FUpgradeStep* Step = TowerDefenseUpgrade::GetStep(TowerDefenseUpgrade::TrackFromDefenderKind(DefenderKind), TierIndex);
	if (!Step)
	{
		return;
	}

	if (HealthComponent)
	{
		HealthComponent->IncreaseMaxHealth(Step->BonusMaxHealth, true);
		MaxHealth = HealthComponent->GetMaxHealth();
	}

	AttackDamage += Step->BonusDamage;
	AttackCooldown = FMath::Max(AttackCooldown * Step->CooldownMultiplier, 0.1f);
	AttackRange += Step->BonusRange;
	if (AttackRangeSphere)
	{
		AttackRangeSphere->SetSphereRadius(AttackRange);
	}

	BodyColor = Step->Color;
	BaseMeshScale *= Step->ScaleMultiplier;
	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(BaseMeshScale);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 50.0f * BaseMeshScale.Z));
	}

	AppliedUpgradeTier = TierIndex;
	ApplyKindSpecificUpgrade(TierIndex);
	ApplyBodyColor();
	StartAttackTimer();
	RefreshUpgradeVisuals();
	JungleVisuals::SpawnUpgradeBloom(GetWorld(), GetActorLocation() + FVector(0.0f, 0.0f, 80.0f));
}

void ADefenderBase::SyncUpgradesToTier(int32 GlobalTier)
{
	const int32 TargetTier = FMath::Clamp(GlobalTier, 0, TowerDefenseUpgrade::GetMaxTier());
	while (AppliedUpgradeTier < TargetTier)
	{
		ApplyUpgradeStep(AppliedUpgradeTier + 1);
	}
}

void ADefenderBase::ApplyKindSpecificUpgrade(int32 TierIndex)
{
	if (DefenderKind == EDefenderKind::JungleScout && TierIndex >= 2)
	{
		EnsureCollarMesh();
	}
}

void ADefenderBase::EnsureCollarMesh()
{
	if (!CollarMesh)
	{
		return;
	}

	CollarMesh->SetHiddenInGame(false);
	CollarMesh->SetVisibility(true);
	JungleVisuals::ApplyBodyMaterial(CollarMesh, this, FLinearColor(0.82f, 0.66f, 0.14f, 1.0f), AppliedUpgradeTier, 0.0f);
}

void ADefenderBase::RefreshUpgradeVisuals()
{
	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(BaseMeshScale);
	}
}

void ADefenderBase::SetupHealthBar()
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
