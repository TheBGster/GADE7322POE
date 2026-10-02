// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TowerDefenseTypes.h"
#include "TowerDefensePlayerController.generated.h"

class ADefenderBase;
class ADefenderPlacementPoint;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectedDefenderChanged, EDefenderKind, Kind, int32, Cost);

/**
 * Handles player input for the tower defence game.
 * Keys 1/2/3 select a defender type. Left-click a pad to spend resources and place it.
 */
UCLASS()
class GADE7322POE_API ATowerDefensePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATowerDefensePlayerController();

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnSelectedDefenderChanged OnSelectedDefenderChanged;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	EDefenderKind GetSelectedDefenderKind() const { return SelectedDefenderKind; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	TSubclassOf<ADefenderBase> GetSelectedDefenderClass() const { return SelectedDefenderClass; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	int32 GetSelectedDefenderCost() const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	FText GetSelectedDefenderDisplayName() const;

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Placement")
	void SelectDefenderKind(EDefenderKind Kind);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleSelectPressed();

	UFUNCTION()
	void SelectJungleScout();

	UFUNCTION()
	void SelectBananaCannon();

	UFUNCTION()
	void SelectVineTrap();

	bool GetSelectionHit(FHitResult& OutHit) const;
	void TryPlaceDefender(ADefenderPlacementPoint* PlacementPoint);
	void ResolveSelectedClass();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	EDefenderKind SelectedDefenderKind;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	TSubclassOf<ADefenderBase> SelectedDefenderClass;

	/** Debug: left-clicking an actor with a Health Component applies this damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug", meta = (ClampMin = "0.0"))
	float DebugDamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug")
	bool bApplyDebugDamageOnSelect;
};
