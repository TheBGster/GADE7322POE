// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseTypes.h"
#include "TowerDefenseHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UDefenderSelectionBarWidget;
class ATowerDefenseGameState;
class ATowerDefensePlayerController;

/**
 * In-game HUD: tower health, resources, defender cost, and placement help.
 * Works as a C++ widget immediately. Optional Blueprint child (WBP_HUD) can restyle it
 * by using these widget names: TowerHealthText, TowerHealthBar, ResourcesText,
 * DefenderCostText, InstructionsText.
 */
UCLASS()
class GADE7322POE_API UTowerDefenseHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void RefreshFromGameState();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Tower Defense|UI")
	void OnHUDUpdated(int32 Resources, int32 DefenderCost, float TowerCurrentHealth, float TowerMaxHealth);

	UFUNCTION()
	void HandleResourcesChanged(int32 NewResourceAmount);

	UFUNCTION()
	void HandleTowerHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleWaveChanged(int32 NewWave, int32 EnemiesInWave);

	UFUNCTION()
	void HandleSelectedDefenderChanged(EDefenderKind Kind, int32 Cost);

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> TowerHealthText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UProgressBar> TowerHealthBar;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> ResourcesText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> DefenderCostText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> InstructionsText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> SelectedDefenderText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UDefenderSelectionBarWidget> SelectionBar;

	void BuildDefaultLayout();
	void BindToGameState();
	void UnbindFromGameState();
	void BindToPlayerController();
	void UnbindFromPlayerController();
	void UpdateResourcesDisplay(int32 Resources, int32 DefenderCost);
	void UpdateTowerHealthDisplay(float CurrentHealth, float MaxHealth);
	void UpdateWaveDisplay(int32 Wave, int32 EnemiesInWave, int32 EnemiesDefeated);
	void UpdateSelectedDefenderDisplay();
	ATowerDefenseGameState* GetTowerDefenseGameState() const;

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
	TWeakObjectPtr<ATowerDefensePlayerController> BoundPlayerController;
	int32 LastEnemiesInWave = 0;
};
