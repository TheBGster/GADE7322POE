// Copyright Epic Games, Inc. All Rights Reserved.

#include "GorillaBrute.h"
#include "GADE7322POE.h"
#include "HealthComponent.h"
#include "TowerDefenseGameState.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"
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
	ResourceReward = 35;
	DefenderSmashMultiplier = 1.35f;
	Tags.AddUnique(TowerDefenseTags::GorillaBrute);

	CharacterMeshTargetHeight = 160.0f;
	CharacterMeshYawOffset = -90.0f;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> GorillaMesh(TEXT("/Game/Fab/Realistic_Black_Gorilla_Free_Low_Poly/gfree/StaticMeshes/gfree.gfree"));
	if (GorillaMesh.Succeeded())
	{
		CharacterMesh = GorillaMesh.Object;
		SetupCharacterMesh();
	}

	if (HealthBarComponent)
	{
		HealthBarComponent->SetDrawSize(FVector2D(140.0f, 30.0f));
	}
}

const TCHAR* AGorillaBrute::GetCharacterMeshPath() const
{
	return TEXT("/Game/Fab/Realistic_Black_Gorilla_Free_Low_Poly/gfree/StaticMeshes/gfree.gfree");
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
