// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseUpgrade.h"
#include "BananaCannon.h"
#include "CentralTower.h"
#include "DefenderBase.h"
#include "EngineUtils.h"
#include "VineTrap.h"

namespace
{
const FUpgradeStep TowerSteps[2] = {
	{
		FText::FromString(TEXT("Heartwood")),
		60,
		40.0f, 8.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.10f,
		FLinearColor(0.22f, 0.48f, 0.18f, 1.0f)
	},
	{
		FText::FromString(TEXT("Canopy Bastion")),
		100,
		60.0f, 6.0f, 0.75f, 0.0f, 0.0f, 0.0f, 0.0f, 1.12f,
		FLinearColor(0.72f, 0.58f, 0.12f, 1.0f)
	}
};

const FUpgradeStep ScoutSteps[2] = {
	{
		FText::FromString(TEXT("Swift Collar")),
		40,
		20.0f, 0.0f, 0.78f, 0.0f, 0.0f, 0.0f, 0.0f, 1.08f,
		FLinearColor(0.48f, 0.58f, 0.18f, 1.0f)
	},
	{
		FText::FromString(TEXT("Canopy Watch")),
		70,
		30.0f, 4.0f, 1.0f, 140.0f, 0.0f, 0.0f, 0.0f, 1.08f,
		FLinearColor(0.78f, 0.62f, 0.16f, 1.0f)
	}
};

const FUpgradeStep CannonSteps[2] = {
	{
		FText::FromString(TEXT("Heavy Barrel")),
		55,
		20.0f, 18.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.08f,
		FLinearColor(0.92f, 0.70f, 0.08f, 1.0f)
	},
	{
		FText::FromString(TEXT("Overripe Volley")),
		90,
		30.0f, 8.0f, 0.75f, 0.0f, 0.0f, 0.0f, 400.0f, 1.14f,
		FLinearColor(0.95f, 0.78f, 0.16f, 1.0f)
	}
};

const FUpgradeStep VineSteps[2] = {
	{
		FText::FromString(TEXT("Binding Roots")),
		45,
		25.0f, 0.0f, 1.0f, 0.0f, 0.12f, 0.0f, 0.0f, 1.06f,
		FLinearColor(0.12f, 0.62f, 0.18f, 1.0f)
	},
	{
		FText::FromString(TEXT("Heartvine")),
		80,
		35.0f, 0.0f, 1.0f, 40.0f, 0.08f, 5.0f, 0.0f, 1.10f,
		FLinearColor(0.18f, 0.72f, 0.22f, 1.0f)
	}
};
}

EUpgradeTrack TowerDefenseUpgrade::TrackFromDefenderKind(EDefenderKind Kind)
{
	switch (Kind)
	{
	case EDefenderKind::BananaCannon:
		return EUpgradeTrack::BananaCannon;
	case EDefenderKind::VineTrap:
		return EUpgradeTrack::VineTrap;
	default:
		return EUpgradeTrack::JungleScout;
	}
}

int32 TowerDefenseUpgrade::GetMaxTier()
{
	return 2;
}

const FUpgradeStep* TowerDefenseUpgrade::GetStep(EUpgradeTrack Track, int32 TierIndex)
{
	if (TierIndex < 1 || TierIndex > GetMaxTier())
	{
		return nullptr;
	}

	const int32 StepIndex = TierIndex - 1;
	switch (Track)
	{
	case EUpgradeTrack::Tower:
		return &TowerSteps[StepIndex];
	case EUpgradeTrack::JungleScout:
		return &ScoutSteps[StepIndex];
	case EUpgradeTrack::BananaCannon:
		return &CannonSteps[StepIndex];
	case EUpgradeTrack::VineTrap:
		return &VineSteps[StepIndex];
	default:
		return nullptr;
	}
}

FText TowerDefenseUpgrade::GetTrackDisplayName(EUpgradeTrack Track)
{
	switch (Track)
	{
	case EUpgradeTrack::Tower:
		return FText::FromString(TEXT("Tower"));
	case EUpgradeTrack::JungleScout:
		return FText::FromString(TEXT("Scout"));
	case EUpgradeTrack::BananaCannon:
		return FText::FromString(TEXT("Cannon"));
	case EUpgradeTrack::VineTrap:
		return FText::FromString(TEXT("Vine"));
	default:
		return FText::FromString(TEXT("Upgrade"));
	}
}

void TowerDefenseUpgrade::ApplyStepToActor(AActor* Actor, EUpgradeTrack Track, int32 TierIndex)
{
	if (!IsValid(Actor))
	{
		return;
	}

	if (ACentralTower* Tower = Cast<ACentralTower>(Actor))
	{
		if (Track == EUpgradeTrack::Tower)
		{
			Tower->ApplyUpgradeStep(TierIndex);
		}
		return;
	}

	if (ADefenderBase* Defender = Cast<ADefenderBase>(Actor))
	{
		if (TrackFromDefenderKind(Defender->GetDefenderKind()) == Track)
		{
			Defender->ApplyUpgradeStep(TierIndex);
		}
	}
}

void TowerDefenseUpgrade::SyncActorToGlobalTier(AActor* Actor, int32 GlobalTier)
{
	if (!IsValid(Actor) || GlobalTier <= 0)
	{
		return;
	}

	if (ACentralTower* Tower = Cast<ACentralTower>(Actor))
	{
		Tower->SyncUpgradesToTier(GlobalTier);
		return;
	}

	if (ADefenderBase* Defender = Cast<ADefenderBase>(Actor))
	{
		Defender->SyncUpgradesToTier(GlobalTier);
	}
}

void TowerDefenseUpgrade::ApplyTrackToWorld(UWorld* World, EUpgradeTrack Track, int32 NewTier)
{
	if (!World || NewTier <= 0)
	{
		return;
	}

	if (Track == EUpgradeTrack::Tower)
	{
		for (TActorIterator<ACentralTower> It(World); It; ++It)
		{
			SyncActorToGlobalTier(*It, NewTier);
		}
		return;
	}

	for (TActorIterator<ADefenderBase> It(World); It; ++It)
	{
		ADefenderBase* Defender = *It;
		if (IsValid(Defender) && TrackFromDefenderKind(Defender->GetDefenderKind()) == Track)
		{
			SyncActorToGlobalTier(Defender, NewTier);
		}
	}
}
