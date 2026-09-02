// Copyright Epic Games, Inc. All Rights Reserved.

#include "GameOverWidget.h"
#include "TowerDefenseGameMode.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UGameOverWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &UGameOverWidget::HandleRestartClicked);
	}

	HideGameOver();
}

void UGameOverWidget::BuildDefaultLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Overlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Overlay"));
	Overlay->SetBrushColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f));
	Overlay->SetPadding(FMargin(48.0f));

	if (UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(Overlay))
	{
		OverlaySlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		OverlaySlot->SetOffsets(FMargin(0.0f));
	}

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	if (UBorderSlot* OverlayContentSlot = Cast<UBorderSlot>(Overlay->AddChild(Content)))
	{
		OverlayContentSlot->SetHorizontalAlignment(HAlign_Center);
		OverlayContentSlot->SetVerticalAlignment(VAlign_Center);
	}

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(NSLOCTEXT("TowerDefense", "GameOverTitle", "Game Over"));
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.82f, 0.28f)));
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 42));
	TitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
	TitleText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));

	if (UVerticalBoxSlot* TitleSlot = Content->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	RestartButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), TEXT("RestartButton"));
	RestartButton->SetBackgroundColor(FLinearColor(0.16f, 0.42f, 0.82f));

	UTextBlock* RestartLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("RestartLabel"));
	RestartLabel->SetText(NSLOCTEXT("TowerDefense", "RestartButton", "Restart"));
	RestartLabel->SetJustification(ETextJustify::Center);
	RestartLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	RestartLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 22));
	RestartButton->AddChild(RestartLabel);

	USizeBox* ButtonSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(), TEXT("RestartButtonSize"));
	ButtonSize->SetWidthOverride(240.0f);
	ButtonSize->SetHeightOverride(56.0f);
	ButtonSize->AddChild(RestartButton);

	if (UVerticalBoxSlot* ButtonSlot = Content->AddChildToVerticalBox(ButtonSize))
	{
		ButtonSlot->SetHorizontalAlignment(HAlign_Center);
		ButtonSlot->SetPadding(FMargin(12.0f));
	}
}

void UGameOverWidget::ShowGameOver()
{
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
}

void UGameOverWidget::HideGameOver()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UGameOverWidget::HandleRestartClicked()
{
	RestartGame();
}

void UGameOverWidget::RestartGame()
{
	ATowerDefenseGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATowerDefenseGameMode>() : nullptr;
	if (GameMode)
	{
		GameMode->RestartCurrentGame();
	}
}
