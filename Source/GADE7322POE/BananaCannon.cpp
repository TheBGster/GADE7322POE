// Copyright Epic Games, Inc. All Rights Reserved.

#include "BananaCannon.h"
#include "BananaProjectile.h"
#include "GADE7322POE.h"
#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Math/NumericLimits.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ABananaCannon::ABananaCannon()
{
	DefenderKind = EDefenderKind::BananaCannon;
	DisplayName = FText::FromString(TEXT("Banana Cannon"));
	BodyColor = FLinearColor(0.95f, 0.80f, 0.10f, 1.0f);
	MaxHealth = 70.0f;
	AttackRange = 1100.0f;
	AttackDamage = 55.0f;
	AttackCooldown = 2.2f;
	PlacementCost = 50;
	ProjectileSpeed = 1500.0f;
	ProjectileClass = ABananaProjectile::StaticClass();
	Tags.AddUnique(TowerDefenseTags::BananaCannon);

	RestingMeshScale = FVector(1.15f, 1.15f, 1.8f);
	if (MeshComponent)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> ConeMesh(TEXT("/Engine/BasicShapes/Cone"));
		if (ConeMesh.Succeeded())
		{
			MeshComponent->SetStaticMesh(ConeMesh.Object);
		}

		MeshComponent->SetRelativeScale3D(RestingMeshScale);
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 90.0f));
	}
}

void ABananaCannon::FindTarget()
{
	if (!IsCombatAllowed())
	{
		CurrentTarget = nullptr;
		return;
	}

	CurrentTarget = nullptr;
	float BestHealth = -1.0f;

	for (int32 Index = TargetsInRange.Num() - 1; Index >= 0; --Index)
	{
		AActor* Candidate = TargetsInRange[Index].Get();
		if (!IsValidTarget(Candidate))
		{
			TargetsInRange.RemoveAtSwap(Index);
			continue;
		}

		float CandidateHealth = 0.0f;
		if (const AEnemyBase* Enemy = Cast<AEnemyBase>(Candidate))
		{
			if (const UHealthComponent* Health = Enemy->GetHealthComponent())
			{
				CandidateHealth = Health->GetCurrentHealth();
			}
		}

		if (CandidateHealth > BestHealth)
		{
			BestHealth = CandidateHealth;
			CurrentTarget = Candidate;
		}
	}
}

void ABananaCannon::AttackTarget()
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
	if (!IsValidTarget(Target) || !GetWorld())
	{
		return;
	}

	UClass* ClassToSpawn = ProjectileClass ? ProjectileClass.Get() : ABananaProjectile::StaticClass();
	const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, 80.0f);
	ABananaProjectile* Projectile = GetWorld()->SpawnActor<ABananaProjectile>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator);
	if (IsValid(Projectile))
	{
		Projectile->InitialiseProjectile(Target, this, AttackDamage, ProjectileSpeed);
	}

	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(RestingMeshScale * 1.18f);
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(FireVisualTimerHandle);
			World->GetTimerManager().SetTimer(FireVisualTimerHandle, this, &ThisClass::ResetFireVisual, 0.12f, false);
		}
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Banana Cannon fired at '%s'."), *GetNameSafe(Target));
}

void ABananaCannon::ResetFireVisual()
{
	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(RestingMeshScale);
	}
}
