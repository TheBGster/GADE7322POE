// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "TowerDefenseTypes.h"
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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveChanged, int32, NewWave, int32, EnemiesInWave);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveCleared, int32, WaveNumber, int32, BonusResources);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnUpgradeChanged, EUpgradeTrack, Track, int32, NewTier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnChronicleChanged);


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

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnWaveChanged OnWaveChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnWaveCleared OnWaveCleared;

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnUpgradeChanged OnUpgradeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnChronicleChanged OnChronicleChanged;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void ResetForNewGame();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void SetMatchState(ETowerDefenseMatchState NewState);

	UFUNCTION(BlueprintPure, Category = "Tower Defense")
	ETowerDefenseMatchState GetMatchState() const { return MatchState; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense")
	bool IsMatchInProgress() const { return MatchState == ETowerDefenseMatchState::InProgress; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetCurrentResources() const { return CurrentResources; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetStartingResources() const { return StartingResources; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetDefenderCost() const { return DefenderCost; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetDefenderCostForKind(EDefenderKind Kind) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Waves")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Waves")
	int32 GetEnemiesDefeated() const { return EnemiesDefeated; }

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Waves")
	void SetCurrentWave(int32 NewWave, int32 EnemiesInWave = 0);

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetEnemyKillReward() const { return EnemyKillReward; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	int32 GetWaveClearBonus(int32 WaveNumber) const;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Waves")
	void HandleWaveCleared(int32 WaveNumber);

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	bool CanAfford(int32 Cost) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	bool CanAffordDefender() const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Resources")
	bool CanAffordDefenderKind(EDefenderKind Kind) const;

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

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Upgrades")
	int32 GetUpgradeTier(EUpgradeTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Upgrades")
	int32 GetNextUpgradeCost(EUpgradeTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Upgrades")
	FText GetNextUpgradeName(EUpgradeTrack Track) const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Upgrades")
	bool CanBuyUpgrade(EUpgradeTrack Track) const;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Upgrades")
	bool TryBuyUpgrade(EUpgradeTrack Track);

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Upgrades")
	int32 GetTotalUpgradePower() const;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Chronicle")
	void SetGroveChronicle(const FGroveChronicle& InChronicle);

	const FGroveChronicle& GetGroveChronicle() const { return GroveChronicle; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Chronicle")
	FText GetCurrentWaveFlavour() const { return CurrentWaveFlavour; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 StartingResources;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 DefenderCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 BananaCannonCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 VineTrapCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 EnemyKillReward;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 WaveClearBaseBonus;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources", meta = (ClampMin = "0"))
	int32 WaveClearBonusPerWave;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Resources")
	int32 CurrentResources;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	float TowerMaxHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	float TowerCurrentHealth;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense")
	ETowerDefenseMatchState MatchState;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Waves")
	int32 CurrentWave;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Waves")
	int32 EnemiesDefeated;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Waves")
	int32 LastClearedWave;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Upgrades")
	int32 TowerUpgradeTier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Upgrades")
	int32 ScoutUpgradeTier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Upgrades")
	int32 CannonUpgradeTier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Upgrades")
	int32 TrapUpgradeTier;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Chronicle")
	FGroveChronicle GroveChronicle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Chronicle")
	FText CurrentWaveFlavour;

	int32& GetUpgradeTierRef(EUpgradeTrack Track);
};
