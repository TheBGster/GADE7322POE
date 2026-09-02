// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class ATowerDefenseGameState;

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

	void BuildDefaultLayout();
	void BindToGameState();
	void UnbindFromGameState();
	void UpdateResourcesDisplay(int32 Resources, int32 DefenderCost);
	void UpdateTowerHealthDisplay(float CurrentHealth, float MaxHealth);
	ATowerDefenseGameState* GetTowerDefenseGameState() const;

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
};
