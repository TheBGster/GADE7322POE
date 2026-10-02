// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChimpRaider.h"
#include "Components/WidgetComponent.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

AChimpRaider::AChimpRaider()
{
	EnemyKind = EEnemyKind::ChimpRaider;
	DisplayName = FText::FromString(TEXT("Chimp Raider"));
	BodyColor = FLinearColor(0.72f, 0.38f, 0.12f, 1.0f);
	MaxHealth = 22.0f;
	MoveSpeed = 520.0f;
	AttackDamage = 5.0f;
	AttackRange = 150.0f;
	AttackCooldown = 0.7f;
	ResourceReward = 8;
	DamagedSpeedBurstMultiplier = 1.45f;
	DamagedSpeedBurstDuration = 1.1f;
	Tags.AddUnique(TowerDefenseTags::ChimpRaider);

	if (MeshComponent)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone"));
		if (ConeMesh.Succeeded())
		{
			MeshComponent->SetStaticMesh(ConeMesh.Object);
		}

		MeshComponent->SetRelativeScale3D(FVector(0.40f, 0.40f, 0.70f));
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 35.0f));
	}

	if (HealthBarComponent)
	{
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));
		HealthBarComponent->SetDrawSize(FVector2D(100.0f, 24.0f));
	}
}

void AChimpRaider::HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy)
{
	Super::HandleDamaged(DamageAmount, DamageCauser, InstigatedBy);

	SpeedBurstMultiplier = DamagedSpeedBurstMultiplier;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpeedBurstTimerHandle);
		World->GetTimerManager().SetTimer(SpeedBurstTimerHandle, this, &ThisClass::ClearSpeedBurst, DamagedSpeedBurstDuration, false);
	}
}

void AChimpRaider::ClearSpeedBurst()
{
	SpeedBurstMultiplier = 1.0f;
}
