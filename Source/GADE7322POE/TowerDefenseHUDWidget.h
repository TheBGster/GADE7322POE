// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseTypes.h"
#include "TowerDefenseHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UDefenderSelectionBarWidget;
class UUpgradeStripWidget;
class ATowerDefenseGameState;
class ATowerDefensePlayerController;


UCLASS()
class GADE7322POE_API UTowerDefenseHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void RefreshFromGameState();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void ShowPlacementFeedback(const FText& Message, FLinearColor Color);

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
	void HandleWaveCleared(int32 WaveNumber, int32 BonusResources);

	UFUNCTION()
	void HandleSelectedDefenderChanged(EDefenderKind Kind, int32 Cost);

	UFUNCTION()
	void HandleUpgradeChanged(EUpgradeTrack Track, int32 NewTier);

	UFUNCTION()
	void HandleChronicleChanged();

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

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> PlacementFeedbackText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> GroveTitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> BriefingText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> WaveFlavourText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UUpgradeStripWidget> UpgradeStrip;

	void BuildDefaultLayout();
	void BindToGameState();
	void UnbindFromGameState();
	void BindToPlayerController();
	void UnbindFromPlayerController();
	void UpdateResourcesDisplay(int32 Resources, int32 DefenderCost);
	void UpdateTowerHealthDisplay(float CurrentHealth, float MaxHealth);
	void UpdateWaveDisplay(int32 Wave, int32 EnemiesInWave, int32 EnemiesDefeated);
	void UpdateSelectedDefenderDisplay();
	void UpdateChronicleDisplay();
	void HideBriefing();
	ATowerDefenseGameState* GetTowerDefenseGameState() const;

	void ClearPlacementFeedback();

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
	TWeakObjectPtr<ATowerDefensePlayerController> BoundPlayerController;
	int32 LastEnemiesInWave = 0;
	FTimerHandle PlacementFeedbackTimerHandle;
	FTimerHandle BriefingTimerHandle;
};
