// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefenderSelectCardWidget.h"
#include "TowerDefenseGameState.h"
#include "TowerDefensePlayerController.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/ButtonSlot.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

TSharedRef<SWidget> UDefenderSelectCardWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UDefenderSelectCardWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!Entry.DisplayName.IsEmpty())
	{
		InitialiseCard(Entry);
	}
}

void UDefenderSelectCardWidget::NativeDestruct()
{
	if (SelectButton)
	{
		SelectButton->OnClicked.RemoveDynamic(this, &ThisClass::HandleClicked);
	}

	Super::NativeDestruct();
}

void UDefenderSelectCardWidget::BuildDefaultLayout()
{
	bLayoutBuilt = true;

	USizeBox* Size = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("CardSize"));
	Size->SetWidthOverride(168.0f);
	Size->SetHeightOverride(124.0f);
	Size->SetVisibility(ESlateVisibility::Visible);
	WidgetTree->RootWidget = Size;

	OuterBorder = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("OuterBorder"));
	OuterBorder->SetPadding(FMargin(4.0f));
	OuterBorder->SetBrushColor(FLinearColor(0.28f, 0.18f, 0.08f, 1.0f));
	Size->AddChild(OuterBorder);

	SelectButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("SelectButton"));
	SelectButton->OnClicked.AddDynamic(this, &ThisClass::HandleClicked);
	OuterBorder->AddChild(SelectButton);

	FSlateBrush Normal;
	Normal.DrawAs = ESlateBrushDrawType::Box;
	Normal.TintColor = FSlateColor(FLinearColor(0.42f, 0.28f, 0.12f, 0.96f));
	FSlateBrush Hovered = Normal;
	Hovered.TintColor = FSlateColor(FLinearColor(0.52f, 0.36f, 0.16f, 1.0f));
	FSlateBrush Pressed = Normal;
	Pressed.TintColor = FSlateColor(FLinearColor(0.34f, 0.22f, 0.08f, 1.0f));
	FSlateBrush Disabled = Normal;
	Disabled.TintColor = FSlateColor(FLinearColor(0.18f, 0.14f, 0.10f, 0.72f));

	FButtonStyle Style;
	Style.SetNormal(Normal);
	Style.SetHovered(Hovered);
	Style.SetPressed(Pressed);
	Style.SetDisabled(Disabled);
	SelectButton->SetStyle(Style);

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("CardContent"));
	if (UButtonSlot* ButtonSlot = Cast<UButtonSlot>(SelectButton->AddChild(Content)))
	{
		ButtonSlot->SetPadding(FMargin(10.0f, 8.0f));
		ButtonSlot->SetHorizontalAlignment(HAlign_Fill);
		ButtonSlot->SetVerticalAlignment(VAlign_Center);
	}

	AccentSwatch = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("AccentSwatch"));
	AccentSwatch->SetColorAndOpacity(FLinearColor(0.32f, 0.42f, 0.22f, 1.0f));
	AccentSwatch->SetDesiredSizeOverride(FVector2D(148.0f, 8.0f));
	if (UVerticalBoxSlot* SwatchSlot = Content->AddChildToVerticalBox(AccentSwatch))
	{
		SwatchSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
		SwatchSlot->SetHorizontalAlignment(HAlign_Fill);
	}

	IconImage = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), TEXT("IconImage"));
	IconImage->SetVisibility(ESlateVisibility::Collapsed);

	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
	NameText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 13));
	NameText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.93f, 0.78f)));
	NameText->SetJustification(ETextJustify::Center);
	NameText->SetAutoWrapText(true);
	Content->AddChildToVerticalBox(NameText);

	CostText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("CostText"));
	CostText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 14));
	CostText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.86f, 0.28f)));
	CostText->SetJustification(ETextJustify::Center);
	if (UVerticalBoxSlot* CostSlot = Content->AddChildToVerticalBox(CostText))
	{
		CostSlot->SetPadding(FMargin(0.0f, 4.0f, 0.0f, 0.0f));
	}

	SelectedLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SelectedLabel"));
	SelectedLabel->SetText(NSLOCTEXT("TowerDefense", "CardSelected", "SELECTED"));
	SelectedLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));
	SelectedLabel->SetColorAndOpacity(FSlateColor(FLinearColor(0.20f, 0.85f, 0.32f)));
	SelectedLabel->SetJustification(ETextJustify::Center);
	SelectedLabel->SetVisibility(ESlateVisibility::Collapsed);
	if (UVerticalBoxSlot* SelectedSlot = Content->AddChildToVerticalBox(SelectedLabel))
	{
		SelectedSlot->SetPadding(FMargin(0.0f, 6.0f, 0.0f, 0.0f));
	}
}

void UDefenderSelectCardWidget::InitialiseCard(const FDefenderCatalogEntry& InEntry)
{
	Entry = InEntry;

	if (NameText)
	{
		NameText->SetText(Entry.DisplayName);
	}

	if (AccentSwatch)
	{
		AccentSwatch->SetColorAndOpacity(Entry.AccentColor);
	}

	if (IconImage)
	{
		if (Entry.Icon)
		{
			IconImage->SetBrushFromTexture(Entry.Icon);
			IconImage->SetVisibility(ESlateVisibility::HitTestInvisible);
		}
		else
		{
			IconImage->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		RefreshCard(GameState->GetCurrentResources(), EDefenderKind::JungleScout, false);
	}
}

void UDefenderSelectCardWidget::RefreshCard(int32 CurrentResources, EDefenderKind SelectedKind, bool bHasSelection)
{
	const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
	const int32 Cost = GameState ? GameState->GetDefenderCostForKind(Entry.Kind) : 0;
	const bool bCanAfford = GameState && GameState->CanAfford(Cost);
	const bool bIsSelected = bHasSelection && SelectedKind == Entry.Kind;

	if (CostText)
	{
		CostText->SetText(FText::Format(NSLOCTEXT("TowerDefense", "CardCost", "${0}"), FText::AsNumber(Cost)));
		CostText->SetColorAndOpacity(FSlateColor(bCanAfford
			? FLinearColor(0.98f, 0.86f, 0.28f)
			: FLinearColor(0.72f, 0.32f, 0.28f)));
	}

	ApplyVisualState(bCanAfford, bIsSelected);
}

void UDefenderSelectCardWidget::ApplyVisualState(bool bCanAfford, bool bIsSelected)
{
	if (SelectButton)
	{
		SelectButton->SetIsEnabled(bCanAfford || bIsSelected);
	}

	if (OuterBorder)
	{
		OuterBorder->SetBrushColor(bIsSelected
			? FLinearColor(0.95f, 0.78f, 0.16f, 1.0f)
			: (bCanAfford ? FLinearColor(0.28f, 0.18f, 0.08f, 1.0f) : FLinearColor(0.12f, 0.09f, 0.06f, 0.85f)));
	}

	if (NameText)
	{
		NameText->SetRenderOpacity(bCanAfford || bIsSelected ? 1.0f : 0.45f);
	}

	if (SelectedLabel)
	{
		SelectedLabel->SetVisibility(bIsSelected ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UDefenderSelectCardWidget::HandleClicked()
{
	if (ATowerDefensePlayerController* PlayerController = GetTowerDefensePlayerController())
	{
		PlayerController->HandleDefenderCardClicked(Entry.Kind);
	}
}

ATowerDefensePlayerController* UDefenderSelectCardWidget::GetTowerDefensePlayerController() const
{
	return Cast<ATowerDefensePlayerController>(GetOwningPlayer());
}
