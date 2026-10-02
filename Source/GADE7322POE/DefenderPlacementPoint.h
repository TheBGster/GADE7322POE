// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "DefenderPlacementPoint.generated.h"

class UMaterialInstanceDynamic;

class ADefenderBase;
class UStaticMeshComponent;


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
	bool PlaceDefenderOfClass(TSubclassOf<ADefenderBase> ClassToPlace);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetOccupied(bool bOccupied);

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void NotifyDefenderDestroyed();

	UFUNCTION(BlueprintPure, Category = "Placement")
	bool IsOccupied() const { return bIsOccupied; }

	UFUNCTION(BlueprintPure, Category = "Placement")
	FVector GetPlacementLocation() const { return PlacementLocation; }

	UFUNCTION(BlueprintCallable, Category = "Placement")
	void SetPlacementHighlight(EPlacementPadHighlight Highlight);

protected:
	virtual void BeginPlay() override;

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

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PadMaterialInstance;

	EPlacementPadHighlight CurrentHighlight;
};
