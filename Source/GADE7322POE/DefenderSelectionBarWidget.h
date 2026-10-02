// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseTypes.h"
#include "DefenderSelectionBarWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UTextBlock;
class UDefenderSelectCardWidget;
class ATowerDefenseGameState;
class ATowerDefensePlayerController;

/** Bottom-of-screen defender picker, wave, and coin display. */
UCLASS()
class GADE7322POE_API UDefenderSelectionBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void RefreshBar();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleResourcesChanged(int32 NewResourceAmount);

	UFUNCTION()
	void HandleWaveChanged(int32 NewWave, int32 EnemiesInWave);

	UFUNCTION()
	void HandleSelectedDefenderChanged(EDefenderKind Kind, int32 Cost);

	void BuildDefaultLayout();
	void BuildCards();
	void RefreshCards();
	void BindDelegates();
	void UnbindDelegates();
	ATowerDefenseGameState* GetTowerDefenseGameState() const;
	ATowerDefensePlayerController* GetTowerDefensePlayerController() const;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> BarBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> WaveLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CoinsLabel;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> CardBox;

	UPROPERTY()
	TArray<TObjectPtr<UDefenderSelectCardWidget>> Cards;

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
	TWeakObjectPtr<ATowerDefensePlayerController> BoundPlayerController;
};
