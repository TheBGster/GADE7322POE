// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TowerDefensePlayerController.generated.h"

class ADefenderPlacementPoint;

/**
 * Handles player input for the tower defence game.
 * Left-click a placement pad to spend resources and spawn a defender.
 */
UCLASS()
class GADE7322POE_API ATowerDefensePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATowerDefensePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleSelectPressed();

	bool GetSelectionHit(FHitResult& OutHit) const;
	void TryPlaceDefender(ADefenderPlacementPoint* PlacementPoint);
	int32 GetDefenderCost() const;

	/** Debug: left-clicking an actor with a Health Component applies this damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug", meta = (ClampMin = "0.0"))
	float DebugDamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug")
	bool bApplyDebugDamageOnSelect;
};
