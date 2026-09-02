// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TowerDefenseTypes.generated.h"

UENUM(BlueprintType)
enum class ETerrainTileType : uint8
{
	Empty UMETA(DisplayName = "Empty"),
	Ground UMETA(DisplayName = "Ground"),
	Elevated UMETA(DisplayName = "Elevated"),
	Path UMETA(DisplayName = "Path"),
	Tower UMETA(DisplayName = "Tower"),
	Placeable UMETA(DisplayName = "Placeable")
};

USTRUCT(BlueprintType)
struct FTerrainTile
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	int32 X = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	int32 Y = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	ETerrainTileType TileType = ETerrainTileType::Empty;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	int32 HeightLevel = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Terrain")
	FVector WorldLocation = FVector::ZeroVector;

	FIntPoint GetGridPosition() const { return FIntPoint(X, Y); }
};

USTRUCT(BlueprintType)
struct FGeneratedPath
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	int32 PathID = INDEX_NONE;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TArray<FIntPoint> GridPoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	TArray<FVector> Waypoints;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	FVector SpawnLocation = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path")
	FIntPoint SpawnGridPosition = FIntPoint::ZeroValue;
};

namespace TowerDefenseTags
{
	inline const FName Enemy(TEXT("Enemy"));
	inline const FName Tower(TEXT("Tower"));
	inline const FName Defender(TEXT("Defender"));
}
