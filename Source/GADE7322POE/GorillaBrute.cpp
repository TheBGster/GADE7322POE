// Copyright Epic Games, Inc. All Rights Reserved.

#include "GorillaBrute.h"
#include "GADE7322POE.h"
#include "HealthComponent.h"
#include "TowerDefenseGameState.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

AGorillaBrute::AGorillaBrute()
{
	EnemyKind = EEnemyKind::GorillaBrute;
	DisplayName = FText::FromString(TEXT("Gorilla Brute"));
	BodyColor = FLinearColor(0.18f, 0.09f, 0.04f, 1.0f);
	MaxHealth = 220.0f;
	MoveSpeed = 110.0f;
	AttackDamage = 18.0f;
	AttackRange = 200.0f;
	AttackCooldown = 1.35f;
	ResourceReward = 18;
	DefenderSmashMultiplier = 1.35f;
	Tags.AddUnique(TowerDefenseTags::GorillaBrute);

	if (MeshComponent)
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
		if (SphereMesh.Succeeded())
		{
			MeshComponent->SetStaticMesh(SphereMesh.Object);
		}

		MeshComponent->SetRelativeScale3D(FVector(1.35f, 1.35f, 1.25f));
		MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	}

	if (HealthBarComponent)
	{
		HealthBarComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 160.0f));
		HealthBarComponent->SetDrawSize(FVector2D(140.0f, 30.0f));
	}
}

void AGorillaBrute::AttackTarget()
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

	float Damage = AttackDamage;
	if (Target->ActorHasTag(TowerDefenseTags::Defender))
	{
		Damage *= DefenderSmashMultiplier;
	}

	UGameplayStatics::ApplyDamage(Target, Damage, nullptr, this, UDamageType::StaticClass());
	UE_LOG(LogTowerDefense, Log, TEXT("Gorilla Brute smashed '%s' for %.0f damage."), *GetNameSafe(Target), Damage);
}
