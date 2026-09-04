// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;

/**
 * Spawns enemies on generated paths using a timer. Does not Tick.
 */
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

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void SpawnEnemy();

	int32 CountActiveEnemies() const;
	const FGeneratedPath* SelectNextPath();
	bool CanSpawnNow() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSubclassOf<AEnemyBase> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "0.2"))
	float SpawnInterval;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 MaxActiveEnemies;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FGeneratedPath> GeneratedPaths;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	bool bIsSpawning;

	FTimerHandle SpawnTimerHandle;
	mutable int32 NextPathIndex;
};
