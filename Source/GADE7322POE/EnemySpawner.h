// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;


UCLASS()
class GADE7322POE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()

public:
	AEnemySpawner();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void SetGeneratedPaths(const TArray<FGeneratedPath>& InPaths);

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StartSpawning();

	UFUNCTION(BlueprintCallable, Category = "Spawner")
	void StopSpawning();

	UFUNCTION(BlueprintPure, Category = "Spawner")
	bool IsSpawning() const { return bIsSpawning; }

	UFUNCTION(BlueprintPure, Category = "Spawner")
	int32 GetCurrentWaveNumber() const { return CurrentWaveNumber; }

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void StartNextWave();

	UFUNCTION()
	void SpawnNextEnemy();

	void BuildCurrentWave();
	void SpawnEnemyFromEntry(const FWaveSpawnEntry& Entry);
	void ScheduleNextSpawn(float Delay);
	void ScheduleNextWave(float Delay);
	void CheckWaveComplete();
	TSubclassOf<AEnemyBase> GetClassForKind(EEnemyKind Kind) const;
	EEnemyKind PickWeightedEnemyKind(int32 GorillasSpawned, int32 ChimpsSpawned) const;
	int32 CountActiveEnemies() const;
	const FGeneratedPath* SelectNextPath();
	const FGeneratedPath* SelectPathForEntry(const FWaveSpawnEntry& Entry);
	bool CanSpawnNow() const;
	float GetInterWaveDelay() const;
	float GetSpawnDelayForKind(EEnemyKind Kind) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AEnemyBase> GorillaBruteClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AEnemyBase> ChimpRaiderClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.2"))
	float SpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.5"))
	float FirstWaveDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1.0"))
	float BaseInterWaveDelay;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxActiveEnemies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FGeneratedPath> GeneratedPaths;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FWaveSpawnEntry> CurrentWaveQueue;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bIsSpawning;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 CurrentWaveNumber;

	FTimerHandle SpawnTimerHandle;
	FTimerHandle WaveTimerHandle;
	mutable FRandomStream WaveRandom;
	mutable int32 NextPathIndex;
	int32 NextSpawnIndex;
	bool bWaveSpawnComplete;

	mutable int32 CachedScoutCount;
	mutable int32 CachedCannonCount;
	mutable int32 CachedTrapCount;
	mutable float CachedTowerHealthPercent;
	mutable int32 CachedResources;
	mutable int32 CachedEnemiesDefeated;
	mutable float CachedDefenseDPS;
};
