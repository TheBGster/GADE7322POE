// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseTypes.h"
#include "UpgradeStripWidget.generated.h"

class UButton;
class UTextBlock;
class UVerticalBox;
class ATowerDefenseGameState;
class ATowerDefensePlayerController;


UCLASS()
class GADE7322POE_API UUpgradeStripWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void RefreshStrip();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleResourcesChanged(int32 NewResourceAmount);

	UFUNCTION()
	void HandleUpgradeChanged(EUpgradeTrack Track, int32 NewTier);

	UFUNCTION()
	void HandleTowerClicked();

	UFUNCTION()
	void HandleScoutClicked();

	UFUNCTION()
	void HandleCannonClicked();

	UFUNCTION()
	void HandleVineClicked();

	void BuildDefaultLayout();
	void BindDelegates();
	void UnbindDelegates();
	void RefreshButton(UButton* Button, UTextBlock* Label, EUpgradeTrack Track);
	UButton* MakeUpgradeButton(FName Name, TObjectPtr<UTextBlock>& OutLabel);
	void TryBuy(EUpgradeTrack Track);
	ATowerDefenseGameState* GetTowerDefenseGameState() const;

	UPROPERTY()
	TObjectPtr<UVerticalBox> ButtonBox;

	UPROPERTY()
	TObjectPtr<UButton> TowerButton;

	UPROPERTY()
	TObjectPtr<UButton> ScoutButton;

	UPROPERTY()
	TObjectPtr<UButton> CannonButton;

	UPROPERTY()
	TObjectPtr<UButton> VineButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> TowerLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> ScoutLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> CannonLabel;

	UPROPERTY()
	TObjectPtr<UTextBlock> VineLabel;

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
};
