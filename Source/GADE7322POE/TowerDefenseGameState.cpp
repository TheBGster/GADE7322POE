// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseGameState.h"
#include "GADE7322POE.h"

ATowerDefenseGameState::ATowerDefenseGameState()
{
	StartingResources = 100;
	DefenderCost = 25;
	BananaCannonCost = 50;
	VineTrapCost = 35;
	EnemyKillReward = 10;
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;
	CurrentWave = 0;
	EnemiesDefeated = 0;
}

void ATowerDefenseGameState::ResetForNewGame()
{
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;
	CurrentWave = 0;
	EnemiesDefeated = 0;

	OnResourcesChanged.Broadcast(CurrentResources);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
	OnMatchStateChanged.Broadcast(MatchState);
	OnWaveChanged.Broadcast(CurrentWave, 0);

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
	OnWaveChanged.Broadcast(CurrentWave, EnemiesInWave);
	UE_LOG(LogTowerDefense, Log, TEXT("Wave %d started (%d enemies)."), CurrentWave, EnemiesInWave);
}

void ATowerDefenseGameState::SetTowerHealth(float NewCurrentHealth, float NewMaxHealth)
{
	TowerMaxHealth = FMath::Max(NewMaxHealth, 0.0f);
	TowerCurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, TowerMaxHealth);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
}
