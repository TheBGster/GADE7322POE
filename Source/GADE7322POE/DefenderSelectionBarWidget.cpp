// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefenderSelectionBarWidget.h"
#include "DefenderSelectCardWidget.h"
#include "TowerDefenseGameState.h"
#include "TowerDefensePlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UDefenderSelectionBarWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UDefenderSelectionBarWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	BindDelegates();
	BuildCards();
	RefreshBar();
}

void UDefenderSelectionBarWidget::NativeDestruct()
{
	UnbindDelegates();
	Super::NativeDestruct();
}

void UDefenderSelectionBarWidget::BuildDefaultLayout()
{
	BarBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("BarBorder"));
	BarBorder->SetBrushColor(FLinearColor(0.10f, 0.07f, 0.03f, 0.92f));
	BarBorder->SetPadding(FMargin(18.0f, 10.0f, 18.0f, 12.0f));
	BarBorder->SetVisibility(ESlateVisibility::Visible);
	WidgetTree->RootWidget = BarBorder;

	UHorizontalBox* RootRow = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("RootRow"));
	BarBorder->AddChild(RootRow);

	UVerticalBox* StatusBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatusBox"));
	if (UHorizontalBoxSlot* StatusSlot = RootRow->AddChildToHorizontalBox(StatusBox))
	{
		StatusSlot->SetPadding(FMargin(0.0f, 4.0f, 18.0f, 4.0f));
		StatusSlot->SetVerticalAlignment(VAlign_Center);
	}

	WaveLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("WaveLabel"));
	WaveLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 16));
	WaveLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.92f, 0.62f)));
	WaveLabel->SetText(NSLOCTEXT("TowerDefense", "BarWaveWaiting", "WAVE --"));
	StatusBox->AddChildToVerticalBox(WaveLabel);

	CoinsLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CoinsLabel"));
	CoinsLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 18));
	CoinsLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.28f)));
	CoinsLabel->SetText(NSLOCTEXT("TowerDefense", "BarCoins", "COINS: --"));
	if (UVerticalBoxSlot* CoinSlot = StatusBox->AddChildToVerticalBox(CoinsLabel))
	{
		CoinSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}

	USizeBox* CardHost = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CardHost"));
	if (UHorizontalBoxSlot* HostSlot = RootRow->AddChildToHorizontalBox(CardHost))
	{
		HostSlot->SetSize(ESlateSizeRule::Fill);
		HostSlot->SetHorizontalAlignment(HAlign_Fill);
		HostSlot->SetVerticalAlignment(VAlign_Center);
	}

	CardBox = WidgetTree->ConstructWidget<UHorizontalBox>(UHorizontalBox::StaticClass(), TEXT("CardBox"));
	CardHost->AddChild(CardBox);
}

void UDefenderSelectionBarWidget::BuildCards()
{
	if (!CardBox || !WidgetTree)
	{
		return;
	}

	CardBox->ClearChildren();
	Cards.Reset();

	ATowerDefensePlayerController* PlayerController = GetTowerDefensePlayerController();
	if (!PlayerController)
	{
		return;
	}

	int32 Index = 0;
	for (const FDefenderCatalogEntry& Entry : PlayerController->GetDefenderCatalog())
	{
		const FName CardName(*FString::Printf(TEXT("DefenderCard_%d"), Index++));
		UDefenderSelectCardWidget* Card = WidgetTree->ConstructWidget<UDefenderSelectCardWidget>(UDefenderSelectCardWidget::StaticClass(), CardName);
		if (!Card)
		{
			continue;
		}

		if (UHorizontalBoxSlot* CardSlot = CardBox->AddChildToHorizontalBox(Card))
		{
			CardSlot->SetPadding(FMargin(6.0f, 0.0f));
			CardSlot->SetVerticalAlignment(VAlign_Center);
		}

		Card->InitialiseCard(Entry);
		Cards.Add(Card);
	}
}

void UDefenderSelectionBarWidget::RefreshBar()
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 Resources = GameState ? GameState->GetCurrentResources() : 0;
	const int32 Wave = GameState ? GameState->GetCurrentWave() : 0;

	if (CoinsLabel)
	{
		CoinsLabel->SetText(FText::Format(NSLOCTEXT("TowerDefense", "BarCoinsValue", "COINS: {0}"), FText::AsNumber(Resources)));
		CoinsLabel->SetColorAndOpacity(FSlateColor(Resources > 0
			? FLinearColor(0.98f, 0.86f, 0.28f)
			: FLinearColor(0.90f, 0.35f, 0.30f)));
	}

	if (WaveLabel)
	{
		if (Wave <= 0)
		{
			WaveLabel->SetText(NSLOCTEXT("TowerDefense", "BarWaveWaiting", "WAVE --"));
		}
		else
		{
			WaveLabel->SetText(FText::Format(NSLOCTEXT("TowerDefense", "BarWaveValue", "WAVE {0}"), FText::AsNumber(Wave)));
		}
	}

	RefreshCards();
}

void UDefenderSelectionBarWidget::RefreshCards()
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const ATowerDefensePlayerController* PlayerController = GetTowerDefensePlayerController();
	const int32 Resources = GameState ? GameState->GetCurrentResources() : 0;
	const EDefenderKind SelectedKind = PlayerController ? PlayerController->GetSelectedDefenderKind() : EDefenderKind::JungleScout;
	const bool bHasSelection = PlayerController && PlayerController->HasActiveDefenderSelection();

	if (Cards.Num() == 0)
	{
		BuildCards();
	}

	for (UDefenderSelectCardWidget* Card : Cards)
	{
		if (Card)
		{
			Card->RefreshCard(Resources, SelectedKind, bHasSelection);
		}
	}
}

void UDefenderSelectionBarWidget::BindDelegates()
{
	ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	if (BoundGameState.Get() != GameState)
	{
		if (ATowerDefenseGameState* Previous = BoundGameState.Get())
		{
			Previous->OnResourcesChanged.RemoveDynamic(this, &ThisClass::HandleResourcesChanged);
			Previous->OnWaveChanged.RemoveDynamic(this, &ThisClass::HandleWaveChanged);
		}

		BoundGameState = GameState;
		if (GameState)
		{
			GameState->OnResourcesChanged.AddDynamic(this, &ThisClass::HandleResourcesChanged);
			GameState->OnWaveChanged.AddDynamic(this, &ThisClass::HandleWaveChanged);
		}
	}

	ATowerDefensePlayerController* PlayerController = GetTowerDefensePlayerController();
	if (BoundPlayerController.Get() != PlayerController)
	{
		if (ATowerDefensePlayerController* Previous = BoundPlayerController.Get())
		{
			Previous->OnSelectedDefenderChanged.RemoveDynamic(this, &ThisClass::HandleSelectedDefenderChanged);
		}

		BoundPlayerController = PlayerController;
		if (PlayerController)
		{
			PlayerController->OnSelectedDefenderChanged.AddDynamic(this, &ThisClass::HandleSelectedDefenderChanged);
		}
	}
}

void UDefenderSelectionBarWidget::UnbindDelegates()
{
	if (ATowerDefenseGameState* GameState = BoundGameState.Get())
	{
		GameState->OnResourcesChanged.RemoveDynamic(this, &ThisClass::HandleResourcesChanged);
		GameState->OnWaveChanged.RemoveDynamic(this, &ThisClass::HandleWaveChanged);
	}

	if (ATowerDefensePlayerController* PlayerController = BoundPlayerController.Get())
	{
		PlayerController->OnSelectedDefenderChanged.RemoveDynamic(this, &ThisClass::HandleSelectedDefenderChanged);
	}

	BoundGameState.Reset();
	BoundPlayerController.Reset();
}

void UDefenderSelectionBarWidget::HandleResourcesChanged(int32 NewResourceAmount)
{
	RefreshBar();
	(void)NewResourceAmount;
}

void UDefenderSelectionBarWidget::HandleWaveChanged(int32 NewWave, int32 EnemiesInWave)
{
	RefreshBar();
	(void)NewWave;
	(void)EnemiesInWave;
}

void UDefenderSelectionBarWidget::HandleSelectedDefenderChanged(EDefenderKind Kind, int32 Cost)
{
	RefreshCards();
	(void)Kind;
	(void)Cost;
}

ATowerDefenseGameState* UDefenderSelectionBarWidget::GetTowerDefenseGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
}

ATowerDefensePlayerController* UDefenderSelectionBarWidget::GetTowerDefensePlayerController() const
{
	return Cast<ATowerDefensePlayerController>(GetOwningPlayer());
}
