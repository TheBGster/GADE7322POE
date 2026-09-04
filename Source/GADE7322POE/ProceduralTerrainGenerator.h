// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "ProceduralTerrainGenerator.generated.h"

class UHierarchicalInstancedStaticMeshComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USceneComponent;

/**
 * Builds a random grid-based terrain at runtime using instanced static meshes.
 * Stage 4 adds edge-to-centre paths, spawn locations, and defender placement data.
 */
UCLASS()
class GADE7322POE_API AProceduralTerrainGenerator : public AActor
{
	GENERATED_BODY()

public:
	AProceduralTerrainGenerator();

	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void GenerateTerrain();

	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void ClearPreviousTerrain();

	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool IsValidCoordinate(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	FVector GridToWorldLocation(int32 X, int32 Y, int32 HeightLevel = 0) const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	FVector GetTileSurfaceLocation(int32 X, int32 Y) const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	bool WorldToGrid(const FVector& WorldLocation, int32& OutX, int32& OutY) const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	FIntPoint GetCenterGridPosition() const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	FVector GetGridCenterWorldLocation() const;

	UFUNCTION(BlueprintPure, Category = "Terrain")
	int32 GetGridWidth() const { return GridWidth; }

	UFUNCTION(BlueprintPure, Category = "Terrain")
	int32 GetGridHeight() const { return GridHeight; }

	UFUNCTION(BlueprintPure, Category = "Terrain")
	float GetTileSize() const { return TileSize; }

	UFUNCTION(BlueprintPure, Category = "Terrain")
	int32 GetRandomSeed() const { return RandomSeed; }

	const TArray<FGeneratedPath>& GetGeneratedPaths() const { return GeneratedPaths; }
	const TArray<FVector>& GetSpawnLocations() const { return SpawnLocations; }
	const TArray<FVector>& GetDefenderPlacementLocations() const { return DefenderPlacementLocations; }

	const TArray<FTerrainTile>& GetTerrainTiles() const { return TerrainTiles; }
	const FTerrainTile* GetTile(int32 X, int32 Y) const;
	FTerrainTile* GetTile(int32 X, int32 Y);

protected:
	virtual void BeginPlay() override;

	void GenerateGrid();
	void MarkTowerLocation();
	void GeneratePaths();
	void GenerateEnemySpawnLocations();
	void GenerateDefenderPlacementLocations();
	void BuildTileInstances();
	void ApplyTileMaterials();
	void DrawPathDebug() const;

	void ConfigureInstancer(UHierarchicalInstancedStaticMeshComponent* Instancer, UStaticMesh* Mesh) const;
	UMaterialInterface* GetSourceTileMaterial() const;
	void SetInstancerColor(UHierarchicalInstancedStaticMeshComponent* Instancer, TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance, const FLinearColor& Color);
	int32 ChooseHeightLevel(int32 X, int32 Y) const;
	ETerrainTileType ChooseTileType(int32 HeightLevel) const;
	bool IsInsideCenterRadius(int32 X, int32 Y) const;
	int32 GetTileIndex(int32 X, int32 Y) const;
	float GetTileWorldHeight(int32 HeightLevel) const;
	float GetRenderedTileHeight(const FTerrainTile& Tile) const;
	void InitializeRandomStream();

	TArray<FIntPoint> SelectPathStartCells();
	FIntPoint PickCellOnEdge(int32 EdgeIndex);
	bool IsTooCloseToAny(const FIntPoint& Candidate, const TArray<FIntPoint>& Existing, int32 MinDistance) const;
	bool FindPathAStar(const FIntPoint& Start, const FIntPoint& Goal, TArray<FIntPoint>& OutPath) const;
	void FindPathGreedy(const FIntPoint& Start, const FIntPoint& Goal, TArray<FIntPoint>& OutPath);
	int32 GetMoveCost(const FIntPoint& To, const FIntPoint& Goal) const;
	void ApplyPathTiles(const TArray<FIntPoint>& Points);
	FGeneratedPath BuildGeneratedPath(int32 PathID, const TArray<FIntPoint>& GridPoints) const;
	bool IsValidDefenderPlacementTile(const FTerrainTile& Tile) const;
	UHierarchicalInstancedStaticMeshComponent* SelectInstancerForTile(const FTerrainTile& Tile) const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> GroundTileInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> ElevatedTileInstancer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	TObjectPtr<UHierarchicalInstancedStaticMeshComponent> PathTileInstancer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "5", ClampMax = "64"))
	int32 GridWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "5", ClampMax = "64"))
	int32 GridHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "50.0"))
	float TileSize;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "10.0"))
	float BaseTileHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "0.0"))
	float HeightStep;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "5.0"))
	float PathTileHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Grid", meta = (ClampMin = "0", ClampMax = "6"))
	int32 MaxHeightLevel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Random")
	bool bRandomizeSeedOnGenerate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Random")
	int32 RandomSeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Random", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float NoiseScale;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Layout")
	bool bFlattenCenter;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Layout", meta = (ClampMin = "0", ClampMax = "8"))
	int32 CenterClearRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Paths", meta = (ClampMin = "3", ClampMax = "8"))
	int32 NumberOfPaths;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Paths", meta = (ClampMin = "2", ClampMax = "16"))
	int32 MinPathStartSeparation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Paths", meta = (ClampMin = "0.0"))
	float SpawnHeightOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Paths")
	bool bDrawDebugPaths;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Placement", meta = (ClampMin = "1", ClampMax = "64"))
	int32 MaxDefenderPlacementPoints;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Placement", meta = (ClampMin = "1", ClampMax = "8"))
	int32 PlacementMinSpacing;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Mesh")
	TObjectPtr<UStaticMesh> GroundTileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Mesh")
	TObjectPtr<UStaticMesh> ElevatedTileMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Mesh")
	TObjectPtr<UStaticMesh> PathTileMesh;

	/** Optional parent material. If empty, Engine BasicShapeMaterial is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Materials")
	TObjectPtr<UMaterialInterface> BaseTileMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Materials")
	FLinearColor GroundColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Terrain|Materials")
	FLinearColor PathColor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Terrain|Materials")
	TObjectPtr<UMaterialInstanceDynamic> GroundMaterialInstance;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Terrain|Materials")
	TObjectPtr<UMaterialInstanceDynamic> PathMaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	TArray<FTerrainTile> TerrainTiles;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain|Paths")
	TArray<FGeneratedPath> GeneratedPaths;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain|Paths")
	TArray<FVector> SpawnLocations;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain|Placement")
	TArray<FVector> DefenderPlacementLocations;

	FRandomStream RandomStream;
	FVector2D NoiseOrigin;
};
