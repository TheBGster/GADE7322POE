// Copyright Epic Games, Inc. All Rights Reserved.

#include "UpgradeStripWidget.h"
#include "TowerDefenseGameState.h"
#include "TowerDefensePlayerController.h"
#include "TowerDefenseUpgrade.h"
#include "UIAudio.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UUpgradeStripWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UUpgradeStripWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);

	if (TowerButton)
	{
		TowerButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleTowerClicked);
	}
	if (ScoutButton)
	{
		ScoutButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleScoutClicked);
	}
	if (CannonButton)
	{
		CannonButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleCannonClicked);
	}
	if (VineButton)
	{
		VineButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleVineClicked);
	}

	BindDelegates();
	RefreshStrip();
}

void UUpgradeStripWidget::NativeDestruct()
{
	UnbindDelegates();
	Super::NativeDestruct();
}

void UUpgradeStripWidget::BuildDefaultLayout()
{
	UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("UpgradeBorder"));
	Border->SetBrushColor(FLinearColor(0.05f, 0.08f, 0.04f, 0.86f));
	Border->SetPadding(FMargin(12.0f, 10.0f));
	WidgetTree->RootWidget = Border;

	ButtonBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("UpgradeBox"));
	Border->AddChild(ButtonBox);

	UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("UpgradeTitle"));
	Title->SetText(NSLOCTEXT("TowerDefense", "UpgradeTitle", "Grove Upgrades"));
	Title->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 14));
	Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.92f, 0.82f, 0.28f)));
	Title->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (UVerticalBoxSlot* TitleSlot = ButtonBox->AddChildToVerticalBox(Title))
	{
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	auto AddButton = [this](UButton* Button)
	{
		USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		Size->SetWidthOverride(210.0f);
		Size->SetHeightOverride(46.0f);
		Size->AddChild(Button);
		if (UVerticalBoxSlot* Slot = ButtonBox->AddChildToVerticalBox(Size))
		{
			Slot->SetPadding(FMargin(0.0f, 3.0f));
		}
	};

	TowerButton = MakeUpgradeButton(TEXT("TowerUpgradeButton"), TowerLabel);
	ScoutButton = MakeUpgradeButton(TEXT("ScoutUpgradeButton"), ScoutLabel);
	CannonButton = MakeUpgradeButton(TEXT("CannonUpgradeButton"), CannonLabel);
	VineButton = MakeUpgradeButton(TEXT("VineUpgradeButton"), VineLabel);
	AddButton(TowerButton);
	AddButton(ScoutButton);
	AddButton(CannonButton);
	AddButton(VineButton);
}

UButton* UUpgradeStripWidget::MakeUpgradeButton(FName Name, TObjectPtr<UTextBlock>& OutLabel)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(FLinearColor(0.18f, 0.28f, 0.12f));
	OutLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	OutLabel->SetJustification(ETextJustify::Center);
	OutLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	OutLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 12));
	OutLabel->SetAutoWrapText(true);
	Button->AddChild(OutLabel);
	UIAudio::BindButtonPressSound(Button);
	return Button;
}

void UUpgradeStripWidget::BindDelegates()
{
	ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	if (BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindDelegates();
	if (!GameState)
	{
		return;
	}

	BoundGameState = GameState;
	GameState->OnResourcesChanged.AddDynamic(this, &ThisClass::HandleResourcesChanged);
	GameState->OnUpgradeChanged.AddDynamic(this, &ThisClass::HandleUpgradeChanged);
}

void UUpgradeStripWidget::UnbindDelegates()
{
	if (ATowerDefenseGameState* GameState = BoundGameState.Get())
	{
		GameState->OnResourcesChanged.RemoveDynamic(this, &ThisClass::HandleResourcesChanged);
		GameState->OnUpgradeChanged.RemoveDynamic(this, &ThisClass::HandleUpgradeChanged);
	}

	BoundGameState.Reset();
}

void UUpgradeStripWidget::RefreshStrip()
{
	RefreshButton(TowerButton, TowerLabel, EUpgradeTrack::Tower);
	RefreshButton(ScoutButton, ScoutLabel, EUpgradeTrack::JungleScout);
	RefreshButton(CannonButton, CannonLabel, EUpgradeTrack::BananaCannon);
	RefreshButton(VineButton, VineLabel, EUpgradeTrack::VineTrap);
}

void UUpgradeStripWidget::RefreshButton(UButton* Button, UTextBlock* Label, EUpgradeTrack Track)
{
	if (!Button || !Label)
	{
		return;
	}

	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 Tier = GameState ? GameState->GetUpgradeTier(Track) : 0;
	const FText TrackName = TowerDefenseUpgrade::GetTrackDisplayName(Track);

	if (Tier >= TowerDefenseUpgrade::GetMaxTier())
	{
		Label->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "UpgradeMaxButton", "{0}  T{1}  MAX"),
			TrackName,
			FText::AsNumber(Tier)));
		Button->SetBackgroundColor(FLinearColor(0.22f, 0.22f, 0.18f));
		Button->SetIsEnabled(false);
		return;
	}

	const int32 Cost = GameState ? GameState->GetNextUpgradeCost(Track) : 0;
	const FText NextName = GameState ? GameState->GetNextUpgradeName(Track) : FText::GetEmpty();
	const bool bCanBuy = GameState && GameState->CanBuyUpgrade(Track);
	Label->SetText(FText::Format(
		NSLOCTEXT("TowerDefense", "UpgradeBuyButton", "{0} T{1}  {2}  ({3})"),
		TrackName,
		FText::AsNumber(Tier),
		NextName,
		FText::AsNumber(Cost)));
	Button->SetBackgroundColor(bCanBuy ? FLinearColor(0.18f, 0.42f, 0.16f) : FLinearColor(0.28f, 0.16f, 0.12f));
	Button->SetIsEnabled(bCanBuy);
}

void UUpgradeStripWidget::HandleResourcesChanged(int32 NewResourceAmount)
{
	(void)NewResourceAmount;
	RefreshStrip();
}

void UUpgradeStripWidget::HandleUpgradeChanged(EUpgradeTrack Track, int32 NewTier)
{
	(void)Track;
	(void)NewTier;
	RefreshStrip();
}

void UUpgradeStripWidget::HandleTowerClicked()
{
	TryBuy(EUpgradeTrack::Tower);
}

void UUpgradeStripWidget::HandleScoutClicked()
{
	TryBuy(EUpgradeTrack::JungleScout);
}

void UUpgradeStripWidget::HandleCannonClicked()
{
	TryBuy(EUpgradeTrack::BananaCannon);
}

void UUpgradeStripWidget::HandleVineClicked()
{
	TryBuy(EUpgradeTrack::VineTrap);
}

void UUpgradeStripWidget::TryBuy(EUpgradeTrack Track)
{
	if (ATowerDefensePlayerController* PlayerController = Cast<ATowerDefensePlayerController>(GetOwningPlayer()))
	{
		PlayerController->TryBuyUpgrade(Track);
		return;
	}

	if (ATowerDefenseGameState* GameState = GetTowerDefenseGameState())
	{
		GameState->TryBuyUpgrade(Track);
	}
}

ATowerDefenseGameState* UUpgradeStripWidget::GetTowerDefenseGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
}
