// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TowerDefenseGameState.generated.h"

UENUM(BlueprintType)
enum class ETowerDefenseMatchState : uint8
{
	WaitingToStart UMETA(DisplayName = "Waiting To Start"),
	InProgress UMETA(DisplayName = "In Progress"),
	GameOver UMETA(DisplayName = "Game Over")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, ETowerDefenseMatchState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResourcesChanged, int32, NewResourceAmount);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTowerHealthChanged, float, CurrentHealth, float, MaxHealth);

/**
 * Shared match data for the tower defence game.
 * Holds player resources, mirrored tower health, and the current match state.
 */
UCLASS()
class GADE7322POE_API ATowerDefenseGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ATowerDefenseGameState();

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnMatchStateChanged OnMatchStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnResourcesChanged OnResourcesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnTowerHealthChanged OnTowerHealthChanged;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void ResetForNewGame();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void SetMatchState(ETowerDefenseMatchState NewState);

	UFUNCTION(BlueprintPure, Category = "Tower Defense")
	ETowerDefenseMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetCurrentResources() const { return CurrentResources; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetStartingResources() const { return StartingResources; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetDefenderCost() const { return DefenderCost; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetEnemyKillReward() const { return EnemyKillReward; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	bool CanAfford(int32 Cost) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	bool CanAffordDefender() const;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Resources")
	void AddResources(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Resources")
	bool SpendResources(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Resources")
	bool TrySpendDefenderCost();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Resources")
	void HandleEnemyKilled(int32 RewardOverride = -1);

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Tower")
	void SetTowerHealth(float NewCurrentHealth, float NewMaxHealth);

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Tower")
	float GetTowerCurrentHealth() const { return TowerCurrentHealth; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Tower")
	float GetTowerMaxHealth() const { return TowerMaxHealth; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 StartingResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 DefenderCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 EnemyKillReward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources")
	int32 CurrentResources;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	float TowerMaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	float TowerCurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense")
	ETowerDefenseMatchState MatchState;
};
