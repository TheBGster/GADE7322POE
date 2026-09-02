// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TowerDefenseGameMode.generated.h"

class ATowerDefenseGameState;
class AProceduralTerrainGenerator;
class ACentralTower;
class AEnemySpawner;
class ADefenderPlacementPoint;

/**
 * Owns the tower defence match flow: start, game over, and restart.
 * Terrain generation, spawning, and combat will be driven from here in later stages.
 */
UCLASS()
class GADE7322POE_API ATowerDefenseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ATowerDefenseGameMode();

	virtual void StartPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void StartNewGame();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void HandleTowerDestroyed();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void HandleGameOver();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense")
	void RestartCurrentGame();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Terrain")
	AProceduralTerrainGenerator* GetTerrainGenerator() const { return TerrainGenerator; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Tower")
	ACentralTower* GetCentralTower() const { return CentralTower; }

	/** Console command: type TDRestart in the Output Log / console. */
	UFUNCTION(Exec)
	void TDRestart();

	/** Console command: regenerate terrain without reloading the level. */
	UFUNCTION(Exec)
	void TDRegen();

protected:
	void GenerateWorldTerrain();
	void SpawnCentralTower();
	void SpawnEnemySpawner();
	void SpawnPlacementPoints();
	void SpawnAttackTestTargets();
	void DestroyGameplayActors();
	void StopActiveEnemies();
	void StopActiveDefenders();
	void MovePlayerToTerrainOverview();
	ATowerDefenseGameState* GetTowerDefenseGameState() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Terrain")
	TSubclassOf<AProceduralTerrainGenerator> TerrainGeneratorClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Terrain")
	FVector TerrainOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Terrain")
	bool bMovePlayerAboveTerrain;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	TSubclassOf<ACentralTower> CentralTowerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Enemies")
	TSubclassOf<AEnemySpawner> EnemySpawnerClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	bool bSpawnAttackTestTargets;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Terrain")
	TObjectPtr<AProceduralTerrainGenerator> TerrainGenerator;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Tower")
	TObjectPtr<ACentralTower> CentralTower;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Enemies")
	TObjectPtr<AEnemySpawner> EnemySpawner;
};
