// Copyright Epic Games, Inc. All Rights Reserved.

#include "BananaCannon.h"
#include "BananaProjectile.h"
#include "GADE7322POE.h"
#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Engine/StaticMesh.h"
#include "Math/NumericLimits.h"
#include "TimerManager.h"
#include "TowerDefenseUpgrade.h"
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

	MeshTargetHeight = 180.0f;
	MeshMaxXY = 160.0f;
	RestingMeshScale = FVector(1.15f, 1.15f, 1.8f);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CannonMesh(TEXT("/Game/Meshes/SM_BananaCannon.SM_BananaCannon"));
	if (CannonMesh.Succeeded())
	{
		CharacterMesh = CannonMesh.Object;
		SetupCharacterMesh();
		if (MeshComponent)
		{
			RestingMeshScale = MeshComponent->GetRelativeScale3D();
		}
	}
	else if (MeshComponent)
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

const TCHAR* ABananaCannon::GetCharacterMeshPath() const
{
	return TEXT("/Game/Meshes/SM_BananaCannon.SM_BananaCannon");
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
	float MuzzleZ = 80.0f;
	if (MeshComponent && MeshComponent->GetStaticMesh())
	{
		const FBoxSphereBounds Bounds = MeshComponent->GetStaticMesh()->GetBounds();
		const float Scale = MeshComponent->GetRelativeScale3D().Z;
		MuzzleZ = MeshComponent->GetRelativeLocation().Z + (Bounds.Origin.Z + Bounds.BoxExtent.Z * 0.35f) * Scale;
	}
	const FVector SpawnLocation = GetActorLocation() + FVector(0.0f, 0.0f, MuzzleZ);
	ABananaProjectile* Projectile = GetWorld()->SpawnActor<ABananaProjectile>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator);
	if (IsValid(Projectile))
	{
		const float ProjectileScale = AppliedUpgradeTier >= 2 ? 1.45f : 1.0f;
		Projectile->InitialiseProjectile(Target, this, AttackDamage, ProjectileSpeed, ProjectileScale);
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
	if (CharacterMesh)
	{
		SetupCharacterMesh();
		if (MeshComponent)
		{
			RestingMeshScale = MeshComponent->GetRelativeScale3D();
		}
		return;
	}

	if (MeshComponent)
	{
		MeshComponent->SetRelativeScale3D(RestingMeshScale);
	}
}

void ABananaCannon::ApplyKindSpecificUpgrade(int32 TierIndex)
{
	const FUpgradeStep* Step = TowerDefenseUpgrade::GetStep(EUpgradeTrack::BananaCannon, TierIndex);
	if (!Step)
	{
		return;
	}

	ProjectileSpeed += Step->BonusProjectileSpeed;
	if (MeshComponent)
	{
		RestingMeshScale = MeshComponent->GetRelativeScale3D();
	}
}
