// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerDefenseTypes.h"
#include "DefenderSelectCardWidget.generated.h"

class UBorder;
class UButton;
class UImage;
class UTextBlock;
class ATowerDefensePlayerController;

/** One selectable defender card in the bottom selection bar. */
UCLASS()
class GADE7322POE_API UDefenderSelectCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void InitialiseCard(const FDefenderCatalogEntry& InEntry);
	void RefreshCard(int32 CurrentResources, EDefenderKind SelectedKind, bool bHasSelection);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleClicked();

	void BuildDefaultLayout();
	void ApplyVisualState(bool bCanAfford, bool bIsSelected);
	ATowerDefensePlayerController* GetTowerDefensePlayerController() const;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UBorder> OuterBorder;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UButton> SelectButton;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> AccentSwatch;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> CostText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> SelectedLabel;

	FDefenderCatalogEntry Entry;
	bool bLayoutBuilt = false;
};
