// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TowerDefenseGameState.h"
#include "TowerDefenseHUD.generated.h"

class UTowerDefenseHUDWidget;
class UGameOverWidget;

/**
 * Creates and owns the HUD and Game Over widgets.
 * Assign Blueprint widget classes (WBP_HUD / WBP_GameOver) to restyle;
 * the C++ widget classes are used if none are assigned.
 */
UCLASS()
class GADE7322POE_API ATowerDefenseHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATowerDefenseHUD();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleMatchStateChanged(ETowerDefenseMatchState NewState);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TSubclassOf<UTowerDefenseHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TObjectPtr<UTowerDefenseHUDWidget> HUDWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TObjectPtr<UGameOverWidget> GameOverWidget;

	void CreateWidgets();
	void RemoveWidgets();
	void BindToGameState();
	void UnbindFromGameState();
	void ApplyMatchState(ETowerDefenseMatchState NewState);
	void SetGameplayInputMode();
	void SetGameOverInputMode();

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
};
