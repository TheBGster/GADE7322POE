// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefenderPlacementPoint.generated.h"

class ADefenderBase;
class UStaticMeshComponent;

/**
 * Predetermined, clickable slot generated from the terrain.
 * Only one defender can occupy a point, and points are never created on enemy paths.
 */
UCLASS()
class GADE7322POE_API ADefenderPlacementPoint : public AActor
{
	GENERATED_BODY()

public:
	ADefenderPlacementPoint();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void InitializePlacement(const FVector& InLocation);

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool CanPlaceDefender() const;

	UFUNCTION(BlueprintCallable, Category = "Placement")
	bool PlaceDefender();

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetOccupied(bool bOccupied);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void NotifyDefenderDestroyed();

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsOccupied() const { return bIsOccupied; }

protected:
	void UpdateVisualState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Placement")
	TSubclassOf<ADefenderBase> DefenderClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	bool bIsOccupied;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Placement")
	FVector PlacementLocation;

	UPROPERTY()
	TWeakObjectPtr<ADefenderBase> OccupyingDefender;
};
