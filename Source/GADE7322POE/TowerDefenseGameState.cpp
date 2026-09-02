// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseGameState.h"
#include "GADE7322POE.h"

ATowerDefenseGameState::ATowerDefenseGameState()
{
	StartingResources = 100;
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;
}

void ATowerDefenseGameState::ResetForNewGame()
{
	CurrentResources = StartingResources;
	TowerMaxHealth = 0.0f;
	TowerCurrentHealth = 0.0f;
	MatchState = ETowerDefenseMatchState::WaitingToStart;

	OnResourcesChanged.Broadcast(CurrentResources);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
	OnMatchStateChanged.Broadcast(MatchState);

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

void ATowerDefenseGameState::AddResources(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	CurrentResources += Amount;
	OnResourcesChanged.Broadcast(CurrentResources);

	UE_LOG(LogTowerDefense, Verbose, TEXT("Added %d resources. Total: %d"), Amount, CurrentResources);
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

	UE_LOG(LogTowerDefense, Verbose, TEXT("Spent %d resources. Remaining: %d"), Amount, CurrentResources);
	return true;
}

void ATowerDefenseGameState::SetTowerHealth(float NewCurrentHealth, float NewMaxHealth)
{
	TowerMaxHealth = FMath::Max(NewMaxHealth, 0.0f);
	TowerCurrentHealth = FMath::Clamp(NewCurrentHealth, 0.0f, TowerMaxHealth);
	OnTowerHealthChanged.Broadcast(TowerCurrentHealth, TowerMaxHealth);
}
