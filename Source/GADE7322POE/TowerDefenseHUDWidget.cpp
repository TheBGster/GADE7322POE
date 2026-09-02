// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseHUDWidget.h"
#include "TowerDefenseGameState.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

namespace
{
FSlateFontInfo MakeHUDFont(int32 Size, const FName Typeface = FName(TEXT("Regular")))
{
	return FCoreStyle::GetDefaultFontStyle(Typeface, Size);
}

UTextBlock* MakeHUDText(UWidgetTree* Tree, FName Name, const FString& Text, int32 FontSize, const FLinearColor& Color)
{
	UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
	Block->SetText(FText::FromString(Text));
	Block->SetColorAndOpacity(FSlateColor(Color));
	Block->SetFont(MakeHUDFont(FontSize));
	Block->SetShadowOffset(FVector2D(1.0f, 1.0f));
	Block->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
	Block->SetVisibility(ESlateVisibility::HitTestInvisible);
	return Block;
}
}

TSharedRef<SWidget> UTowerDefenseHUDWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UTowerDefenseHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BindToGameState();
	RefreshFromGameState();
}

void UTowerDefenseHUDWidget::NativeDestruct()
{
	UnbindFromGameState();
	Super::NativeDestruct();
}

void UTowerDefenseHUDWidget::BuildDefaultLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	WidgetTree->RootWidget = RootCanvas;

	UBorder* InfoPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("InfoPanel"));
	InfoPanel->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.05f, 0.72f));
	InfoPanel->SetPadding(FMargin(18.0f, 14.0f));
	InfoPanel->SetVisibility(ESlateVisibility::HitTestInvisible);

	UVerticalBox* InfoBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("InfoBox"));
	InfoPanel->AddChild(InfoBox);

	TowerHealthText = MakeHUDText(WidgetTree, TEXT("TowerHealthText"), TEXT("Tower Health: -- / --"), 20, FLinearColor::White);
	TowerHealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("TowerHealthBar"));
	TowerHealthBar->SetPercent(0.0f);
	TowerHealthBar->SetFillColorAndOpacity(FLinearColor(0.20f, 0.78f, 0.32f));
	TowerHealthBar->SetVisibility(ESlateVisibility::HitTestInvisible);

	FSlateBrush HealthBackground;
	HealthBackground.DrawAs = ESlateBrushDrawType::Box;
	HealthBackground.TintColor = FSlateColor(FLinearColor(0.08f, 0.08f, 0.10f, 0.95f));
	HealthBackground.ImageSize = FVector2D(280.0f, 16.0f);

	FSlateBrush HealthFill;
	HealthFill.DrawAs = ESlateBrushDrawType::Box;
	HealthFill.TintColor = FSlateColor(FLinearColor::White);
	HealthFill.ImageSize = FVector2D(280.0f, 16.0f);

	FProgressBarStyle HealthBarStyle;
	HealthBarStyle.SetBackgroundImage(HealthBackground);
	HealthBarStyle.SetFillImage(HealthFill);
	TowerHealthBar->SetWidgetStyle(HealthBarStyle);

	USizeBox* HealthBarSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("HealthBarSize"));
	HealthBarSize->SetWidthOverride(280.0f);
	HealthBarSize->SetHeightOverride(16.0f);
	HealthBarSize->SetVisibility(ESlateVisibility::HitTestInvisible);
	HealthBarSize->AddChild(TowerHealthBar);

	ResourcesText = MakeHUDText(WidgetTree, TEXT("ResourcesText"), TEXT("Resources: --"), 18, FLinearColor::White);
	DefenderCostText = MakeHUDText(WidgetTree, TEXT("DefenderCostText"), TEXT("Defender Cost: --"), 16, FLinearColor(0.85f, 0.85f, 0.85f));

	auto AddInfoChild = [InfoBox](UWidget* Child, float BottomPadding)
	{
		if (UVerticalBoxSlot* Slot = InfoBox->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, BottomPadding));
		}
	};

	AddInfoChild(TowerHealthText, 6.0f);
	AddInfoChild(HealthBarSize, 12.0f);
	AddInfoChild(ResourcesText, 4.0f);
	AddInfoChild(DefenderCostText, 0.0f);

	if (UCanvasPanelSlot* InfoSlot = RootCanvas->AddChildToCanvas(InfoPanel))
	{
		InfoSlot->SetAnchors(FAnchors(0.0f, 0.0f, 0.0f, 0.0f));
		InfoSlot->SetAlignment(FVector2D(0.0f, 0.0f));
		InfoSlot->SetPosition(FVector2D(36.0f, 32.0f));
		InfoSlot->SetAutoSize(true);
	}

	InstructionsText = MakeHUDText(
		WidgetTree,
		TEXT("InstructionsText"),
		TEXT("Left-click a pad to place a defender."),
		16,
		FLinearColor(0.92f, 0.92f, 0.92f));

	if (UCanvasPanelSlot* InstructionSlot = RootCanvas->AddChildToCanvas(InstructionsText))
	{
		InstructionSlot->SetAnchors(FAnchors(0.5f, 1.0f, 0.5f, 1.0f));
		InstructionSlot->SetAlignment(FVector2D(0.5f, 1.0f));
		InstructionSlot->SetPosition(FVector2D(0.0f, -36.0f));
		InstructionSlot->SetAutoSize(true);
	}
}

void UTowerDefenseHUDWidget::BindToGameState()
{
	ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	if (BoundGameState.Get() == GameState)
	{
		return;
	}

	UnbindFromGameState();

	if (!GameState)
	{
		return;
	}

	BoundGameState = GameState;
	GameState->OnResourcesChanged.AddDynamic(this, &UTowerDefenseHUDWidget::HandleResourcesChanged);
	GameState->OnTowerHealthChanged.AddDynamic(this, &UTowerDefenseHUDWidget::HandleTowerHealthChanged);
}

void UTowerDefenseHUDWidget::UnbindFromGameState()
{
	if (ATowerDefenseGameState* GameState = BoundGameState.Get())
	{
		GameState->OnResourcesChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleResourcesChanged);
		GameState->OnTowerHealthChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleTowerHealthChanged);
	}

	BoundGameState.Reset();
}

void UTowerDefenseHUDWidget::RefreshFromGameState()
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	if (!GameState)
	{
		UpdateResourcesDisplay(0, 0);
		UpdateTowerHealthDisplay(0.0f, 0.0f);
		OnHUDUpdated(0, 0, 0.0f, 0.0f);
		return;
	}

	const int32 Resources = GameState->GetCurrentResources();
	const int32 DefenderCost = GameState->GetDefenderCost();
	const float CurrentHealth = GameState->GetTowerCurrentHealth();
	const float MaxHealth = GameState->GetTowerMaxHealth();

	UpdateResourcesDisplay(Resources, DefenderCost);
	UpdateTowerHealthDisplay(CurrentHealth, MaxHealth);
	OnHUDUpdated(Resources, DefenderCost, CurrentHealth, MaxHealth);
}

void UTowerDefenseHUDWidget::HandleResourcesChanged(int32 NewResourceAmount)
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 DefenderCost = GameState ? GameState->GetDefenderCost() : 0;
	const float CurrentHealth = GameState ? GameState->GetTowerCurrentHealth() : 0.0f;
	const float MaxHealth = GameState ? GameState->GetTowerMaxHealth() : 0.0f;

	UpdateResourcesDisplay(NewResourceAmount, DefenderCost);
	OnHUDUpdated(NewResourceAmount, DefenderCost, CurrentHealth, MaxHealth);
}

void UTowerDefenseHUDWidget::HandleTowerHealthChanged(float CurrentHealth, float MaxHealth)
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 Resources = GameState ? GameState->GetCurrentResources() : 0;
	const int32 DefenderCost = GameState ? GameState->GetDefenderCost() : 0;

	UpdateTowerHealthDisplay(CurrentHealth, MaxHealth);
	OnHUDUpdated(Resources, DefenderCost, CurrentHealth, MaxHealth);
}

void UTowerDefenseHUDWidget::UpdateResourcesDisplay(int32 Resources, int32 DefenderCost)
{
	const bool bCanAfford = DefenderCost > 0 && Resources >= DefenderCost;
	const FLinearColor ResourceColor = bCanAfford
		? FLinearColor(0.95f, 0.90f, 0.35f)
		: FLinearColor(0.95f, 0.35f, 0.32f);

	if (ResourcesText)
	{
		ResourcesText->SetText(FText::Format(NSLOCTEXT("TowerDefense", "ResourcesHUD", "Resources: {0}"), FText::AsNumber(Resources)));
		ResourcesText->SetColorAndOpacity(FSlateColor(ResourceColor));
	}

	if (DefenderCostText)
	{
		DefenderCostText->SetText(FText::Format(NSLOCTEXT("TowerDefense", "DefenderCostHUD", "Defender Cost: {0}"), FText::AsNumber(DefenderCost)));
	}

	if (InstructionsText)
	{
		InstructionsText->SetText(bCanAfford
			? NSLOCTEXT("TowerDefense", "PlaceInstruction", "Left-click a pad to place a defender.")
			: NSLOCTEXT("TowerDefense", "NeedResourcesInstruction", "Defeat enemies to earn resources, then place a defender."));
	}
}

void UTowerDefenseHUDWidget::UpdateTowerHealthDisplay(float CurrentHealth, float MaxHealth)
{
	const int32 Current = FMath::RoundToInt(CurrentHealth);
	const int32 Max = FMath::RoundToInt(MaxHealth);
	const float Percent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;

	if (TowerHealthText)
	{
		TowerHealthText->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "TowerHealthHUD", "Tower Health: {0} / {1}"),
			FText::AsNumber(Current),
			FText::AsNumber(Max)));
	}

	if (TowerHealthBar)
	{
		TowerHealthBar->SetPercent(Percent);

		const FLinearColor FillColor = Percent > 0.4f
			? FLinearColor(0.20f, 0.78f, 0.32f)
			: (Percent > 0.2f ? FLinearColor(0.92f, 0.72f, 0.18f) : FLinearColor(0.90f, 0.22f, 0.18f));
		TowerHealthBar->SetFillColorAndOpacity(FillColor);
	}
}

ATowerDefenseGameState* UTowerDefenseHUDWidget::GetTowerDefenseGameState() const
{
	const UWorld* World = GetWorld();
	return World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
}
