// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TowerDefenseTypes.h"

class AActor;
class UWorld;

struct FUpgradeStep
{
	FText Name;
	int32 Cost = 0;
	float BonusMaxHealth = 0.0f;
	float BonusDamage = 0.0f;
	float CooldownMultiplier = 1.0f;
	float BonusRange = 0.0f;
	float BonusSlow = 0.0f;
	float BonusPulseDamage = 0.0f;
	float BonusProjectileSpeed = 0.0f;
	float ScaleMultiplier = 1.0f;
	FLinearColor Color = FLinearColor::White;
};

namespace TowerDefenseUpgrade
{
	EUpgradeTrack TrackFromDefenderKind(EDefenderKind Kind);
	int32 GetMaxTier();
	const FUpgradeStep* GetStep(EUpgradeTrack Track, int32 TierIndex);
	FText GetTrackDisplayName(EUpgradeTrack Track);
	void ApplyStepToActor(AActor* Actor, EUpgradeTrack Track, int32 TierIndex);
	void SyncActorToGlobalTier(AActor* Actor, int32 GlobalTier);
	void ApplyTrackToWorld(UWorld* World, EUpgradeTrack Track, int32 NewTier);
}
