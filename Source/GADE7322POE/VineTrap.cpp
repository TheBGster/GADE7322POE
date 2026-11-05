// Copyright Epic Games, Inc. All Rights Reserved.

#include "VineTrap.h"
#include "EnemyBase.h"
#include "GADE7322POE.h"
#include "GameFramework/DamageType.h"
#include "JungleVisuals.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "TowerDefenseUpgrade.h"
#include "UObject/ConstructorHelpers.h"

AVineTrap::AVineTrap()
{
	DefenderKind = EDefenderKind::VineTrap;
	DisplayName = FText::FromString(TEXT("Vine Trap"));
	BodyColor = FLinearColor(0.18f, 0.72f, 0.22f, 1.0f);
	MaxHealth = 90.0f;
	AttackRange = 360.0f;
	AttackDamage = 0.0f;
	AttackCooldown = 2.6f;
	PlacementCost = 35;
	SlowPercent = 0.45f;
	SlowDuration = 2.0f;
	PulseDamage = 3.0f;
	Tags.AddUnique(TowerDefenseTags::VineTrap);

	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(FVector(1.8f, 1.8f, 0.28f));
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 18.0f));
	}

	VineRingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VineRingMesh"));
	VineRingMesh->SetupAttachment(SceneRoot);
	VineRingMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RestingRingScale = FVector(2.4f, 2.4f, 0.08f);
	VisualSwayAmount = 8.0f;
	VineRingMesh->SetRelativeScale3D(RestingRingScale);
	VineRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		VineRingMesh->SetStaticMesh(CylinderMesh.Object);
	}

	if (HealthBarComponent)
	{
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	}
}

void AVineTrap::BeginPlay()
{
	Super::BeginPlay();
	ApplyVineMaterials();
}

void AVineTrap::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RingMaterialInstance && GetWorld())
	{
		RingMaterialInstance->SetScalarParameterValue(TEXT("Time"), GetWorld()->GetTimeSeconds());
		RingMaterialInstance->SetScalarParameterValue(TEXT("SwayAmount"), VisualSwayAmount);
		RingMaterialInstance->SetScalarParameterValue(TEXT("SwaySpeed"), 1.6f + AppliedUpgradeTier * 0.35f);
		RingMaterialInstance->SetScalarParameterValue(TEXT("UpgradeTier"), static_cast<float>(AppliedUpgradeTier));
	}
}

void AVineTrap::ApplyVineMaterials()
{
	if (!VineRingMesh)
	{
		return;
	}

	RingMaterialInstance = JungleVisuals::ApplyBodyMaterial(
		VineRingMesh,
		this,
		FLinearColor(0.05f, 0.28f, 0.08f, 1.0f),
		AppliedUpgradeTier,
		VisualSwayAmount);
}

void AVineTrap::AttackTarget()
{
	if (!IsCombatAllowed())
	{
		return;
	}

	PulseVines();
}

void AVineTrap::PulseVines()
{
	int32 Affected = 0;
	for (int32 Index = TargetsInRange.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = TargetsInRange[Index].Get();
		if (!IsValidTarget(Candidate))
		{
			TargetsInRange.RemoveAtSwap(Index);
			continue;
		}

		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Candidate))
		{
			Enemy->ApplyMovementSlow(GetUniqueID(), SlowPercent, SlowDuration);
			++Affected;
		}

		if (PulseDamage > 0.0f)
		{
			UGameplayStatics::ApplyDamage(Candidate, PulseDamage, nullptr, this, UDamageType::StaticClass());
		}
	}

	if (VineRingMesh)
	{
		VineRingMesh->SetRelativeScale3D(RestingRingScale * FVector(1.29f, 1.29f, 1.25f));
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(PulseVisualTimerHandle);
			World->GetTimerManager().SetTimer(PulseVisualTimerHandle, this, &ThisClass::ResetPulseVisual, 0.18f, false);
		}
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Vine Trap pulsed. Slowed %d enemies."), Affected);
}

void AVineTrap::ResetPulseVisual()
{
	if (VineRingMesh)
	{
		VineRingMesh->SetRelativeScale3D(RestingRingScale);
	}
}

void AVineTrap::ApplyKindSpecificUpgrade(int32 TierIndex)
{
	const FUpgradeStep* Step = TowerDefenseUpgrade::GetStep(EUpgradeTrack::VineTrap, TierIndex);
	if (!Step)
	{
		return;
	}

	SlowPercent = FMath::Clamp(SlowPercent + Step->BonusSlow, 0.0f, 0.85f);
	PulseDamage += Step->BonusPulseDamage;
	VisualSwayAmount = 8.0f + TierIndex * 6.0f;
	RestingRingScale *= Step->ScaleMultiplier;
	if (VineRingMesh)
	{
		VineRingMesh->SetRelativeScale3D(RestingRingScale);
	}
	ApplyVineMaterials();
}
