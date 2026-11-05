// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseGameState.h"
#include "GADE7322POE.h"
#include "GroveChronicleGenerator.h"
#include "TowerDefenseUpgrade.h"

ATowerDefenseGameState::ATowerDefenseGameState()
{
	StartingResources = 100;
	DefenderCost = 25;
	BananaCannonCost = 50;
	VineTrapCost = 35;
	EnemyKillReward = 20;
	WaveClearBaseBonus = 25;
	WaveClearBonusPerWave = 15;
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;
	CurrentWave = 0;
	EnemiesDefeated = 0;
	LastClearedWave = 0;
	TowerUpgradeTier = 0;
	ScoutUpgradeTier = 0;
	CannonUpgradeTier = 0;
	TrapUpgradeTier = 0;
}

void ATowerDefenseGameState::ResetForNewGame()
{
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;
	CurrentWave = 0;
	EnemiesDefeated = 0;
	LastClearedWave = 0;
	TowerUpgradeTier = 0;
	ScoutUpgradeTier = 0;
	CannonUpgradeTier = 0;
	TrapUpgradeTier = 0;
	GroveChronicle = FGroveChronicle();
	CurrentWaveFlavour = FText::GetEmpty();

	OnResourcesChanged.Broadcast(CurrentResources);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
	OnWaveChanged.Broadcast(CurrentWave, 0);
	OnUpgradeChanged.Broadcast(EUpgradeTrack::Tower, 0);
	OnChronicleChanged.Broadcast();

	UE_LOG(LogTowerDefense, Log, TEXT("Game state reset. Starting resources: %d"), CurrentResources);
}

void ATowerDefenseGameState::SetMatchState(ETowerDefenseMatchState NewState)
{
	if (MatchState == NewState)
	{
		return;
	}

	MatchState = NewState;
	OnMatchStateChanged.Broadcast(MatchState);

	UE_LOG(LogTowerDefense, Log, TEXT("Match state changed to %s"),
		*UEnum::GetValueAsString(MatchState));
}

bool ATowerDefenseGameState::CanAfford(int32 Cost) const
{
	return Cost >= 0 && CurrentResources >= Cost;
}

bool ATowerDefenseGameState::CanAffordDefender() const
{
	return CanAfford(DefenderCost);
}

bool ATowerDefenseGameState::CanAffordDefenderKind(EDefenderKind Kind) const
{
	return CanAfford(GetDefenderCostForKind(Kind));
}

void ATowerDefenseGameState::AddResources(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentResources += Amount;
	OnResourcesChanged.Broadcast(CurrentResources);

	UE_LOG(LogTowerDefense, Log, TEXT("Added %d resources. Total: %d"), Amount, CurrentResources);
}

bool ATowerDefenseGameState::SpendResources(int32 Amount)
{
	if (!CanAfford(Amount))
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot spend %d resources. Current: %d"), Amount, CurrentResources);
		return false;
	}

	CurrentResources -= Amount;
	OnResourcesChanged.Broadcast(CurrentResources);

	UE_LOG(LogTowerDefense, Log, TEXT("Spent %d resources. Remaining: %d"), Amount, CurrentResources);
	return true;
}

bool ATowerDefenseGameState::TrySpendDefenderCost()
{
	return SpendResources(DefenderCost);
}

void ATowerDefenseGameState::HandleEnemyKilled(int32 RewardOverride)
{
	if (MatchState != ETowerDefenseMatchState::InProgress)
	{
		return;
	}

	const int32 Reward = RewardOverride >= 0 ? RewardOverride : EnemyKillReward;
	++EnemiesDefeated;
	if (Reward > 0)
	{
		AddResources(Reward);
	}
	UE_LOG(LogTowerDefense, Log, TEXT("Enemy kill reward applied. +%d  Defeated: %d"), Reward, EnemiesDefeated);
}

int32 ATowerDefenseGameState::GetWaveClearBonus(int32 WaveNumber) const
{
	const int32 SafeWave = FMath::Max(WaveNumber, 1);
	return WaveClearBaseBonus + WaveClearBonusPerWave * SafeWave;
}

void ATowerDefenseGameState::HandleWaveCleared(int32 WaveNumber)
{
	if (MatchState != ETowerDefenseMatchState::InProgress || WaveNumber <= 0 || WaveNumber == LastClearedWave)
	{
		return;
	}

	LastClearedWave = WaveNumber;
	const int32 Bonus = GetWaveClearBonus(WaveNumber);
	if (Bonus > 0)
	{
		AddResources(Bonus);
	}

	OnWaveCleared.Broadcast(WaveNumber, Bonus);
	UE_LOG(LogTowerDefense, Log, TEXT("Wave %d cleared. Bonus +%d resources."), WaveNumber, Bonus);
}

int32 ATowerDefenseGameState::GetDefenderCostForKind(EDefenderKind Kind) const
{
	switch (Kind)
	{
	case EDefenderKind::BananaCannon:
		return BananaCannonCost;
	case EDefenderKind::VineTrap:
		return VineTrapCost;
	default:
		return DefenderCost;
	}
}

void ATowerDefenseGameState::SetCurrentWave(int32 NewWave, int32 EnemiesInWave)
{
	CurrentWave = FMath::Max(NewWave, 0);
	CurrentWaveFlavour = GroveChronicleGenerator::GetWaveFlavour(GroveChronicle, CurrentWave, GetTotalUpgradePower());
	OnWaveChanged.Broadcast(CurrentWave, EnemiesInWave);
	OnChronicleChanged.Broadcast();
	UE_LOG(LogTowerDefense, Log, TEXT("Wave %d started (%d enemies)."), CurrentWave, EnemiesInWave);
}

int32& ATowerDefenseGameState::GetUpgradeTierRef(EUpgradeTrack Track)
{
	switch (Track)
	{
	case EUpgradeTrack::JungleScout:
		return ScoutUpgradeTier;
	case EUpgradeTrack::BananaCannon:
		return CannonUpgradeTier;
	case EUpgradeTrack::VineTrap:
		return TrapUpgradeTier;
	default:
		return TowerUpgradeTier;
	}
}

int32 ATowerDefenseGameState::GetUpgradeTier(EUpgradeTrack Track) const
{
	switch (Track)
	{
	case EUpgradeTrack::JungleScout:
		return ScoutUpgradeTier;
	case EUpgradeTrack::BananaCannon:
		return CannonUpgradeTier;
	case EUpgradeTrack::VineTrap:
		return TrapUpgradeTier;
	default:
		return TowerUpgradeTier;
	}
}

int32 ATowerDefenseGameState::GetNextUpgradeCost(EUpgradeTrack Track) const
{
	const FUpgradeStep* Step = TowerDefenseUpgrade::GetStep(Track, GetUpgradeTier(Track) + 1);
	return Step ? Step->Cost : 0;
}

FText ATowerDefenseGameState::GetNextUpgradeName(EUpgradeTrack Track) const
{
	const int32 CurrentTier = GetUpgradeTier(Track);
	if (CurrentTier >= TowerDefenseUpgrade::GetMaxTier())
	{
		return NSLOCTEXT("TowerDefense", "UpgradeMaxed", "Maxed");
	}

	const FUpgradeStep* Step = TowerDefenseUpgrade::GetStep(Track, CurrentTier + 1);
	return Step ? Step->Name : FText::GetEmpty();
}

bool ATowerDefenseGameState::CanBuyUpgrade(EUpgradeTrack Track) const
{
	if (MatchState != ETowerDefenseMatchState::InProgress)
	{
		return false;
	}

	if (GetUpgradeTier(Track) >= TowerDefenseUpgrade::GetMaxTier())
	{
		return false;
	}

	return CanAfford(GetNextUpgradeCost(Track));
}

bool ATowerDefenseGameState::TryBuyUpgrade(EUpgradeTrack Track)
{
	if (GetUpgradeTier(Track) >= TowerDefenseUpgrade::GetMaxTier())
	{
		return false;
	}

	const int32 Cost = GetNextUpgradeCost(Track);
	if (!SpendResources(Cost))
	{
		return false;
	}

	int32& Tier = GetUpgradeTierRef(Track);
	++Tier;
	TowerDefenseUpgrade::ApplyTrackToWorld(GetWorld(), Track, Tier);
	OnUpgradeChanged.Broadcast(Track, Tier);

	UE_LOG(LogTowerDefense, Log, TEXT("Bought %s upgrade tier %d for %d."),
		*TowerDefenseUpgrade::GetTrackDisplayName(Track).ToString(), Tier, Cost);
	return true;
}

int32 ATowerDefenseGameState::GetTotalUpgradePower() const
{
	return TowerUpgradeTier + ScoutUpgradeTier + CannonUpgradeTier + TrapUpgradeTier;
}

void ATowerDefenseGameState::SetGroveChronicle(const FGroveChronicle& InChronicle)
{
	GroveChronicle = InChronicle;
	CurrentWaveFlavour = GroveChronicle.OpeningBrief;
	OnChronicleChanged.Broadcast();
}

void ATowerDefenseGameState::SetTowerHealth(float NewCurrentHealth, float NewMaxHealth)
{
	TowerMaxHealth = FMath::Max(NewMaxHealth, 0.0f);
	TowerCurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, TowerMaxHealth);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
}
