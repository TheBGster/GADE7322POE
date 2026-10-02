// Copyright Epic Games, Inc. All Rights Reserved.

#include "VineTrap.h"
#include "EnemyBase.h"
#include "GADE7322POE.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AVineTrap::AVineTrap()
{
	DefenderKind = EDefenderKind::VineTrap;
	DisplayName = FText::FromString(TEXT("Vine Trap"));
	BodyColor = FLinearColor(0.08f, 0.42f, 0.12f, 1.0f);
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
	VineRingMesh->SetRelativeScale3D(FVector(2.4f, 2.4f, 0.08f));
	VineRingMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> TorusMesh(TEXT("/Engine/BasicShapes/Torus"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (TorusMesh.Succeeded())
	{
		VineRingMesh->SetStaticMesh(TorusMesh.Object);
	}
	else if (CylinderMesh.Succeeded())
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

	if (VineRingMesh)
	{
		UMaterialInterface* SourceMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
		if (SourceMaterial)
		{
			if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(SourceMaterial, this))
			{
				const FLinearColor RingColor(0.05f, 0.28f, 0.08f, 1.0f);
				Material->SetVectorParameterValue(TEXT("Color"), RingColor);
				Material->SetVectorParameterValue(TEXT("BaseColor"), RingColor);
				VineRingMesh->SetMaterial(0, Material);
			}
		}
	}
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
		VineRingMesh->SetRelativeScale3D(FVector(3.1f, 3.1f, 0.10f));
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
		VineRingMesh->SetRelativeScale3D(FVector(2.4f, 2.4f, 0.08f));
	}
}
