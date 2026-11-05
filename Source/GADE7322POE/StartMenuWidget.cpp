// Copyright Epic Games, Inc. All Rights Reserved.

#include "StartMenuWidget.h"
#include "GADE7322POE.h"
#include "TowerDefenseGameMode.h"
#include "UIAudio.h"
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
#include "Kismet/KismetSystemLibrary.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UStartMenuWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UStartMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PlayButton)
	{
		PlayButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandlePlayClicked);
	}

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleExitClicked);
	}
}

void UStartMenuWidget::BuildDefaultLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Overlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Overlay"));
	Overlay->SetBrushColor(FLinearColor(0.02f, 0.05f, 0.02f, 0.88f));
	Overlay->SetPadding(FMargin(48.0f));

	if (UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(Overlay))
	{
		OverlaySlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		OverlaySlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(FLinearColor(0.12f, 0.09f, 0.04f, 0.96f));
	Panel->SetPadding(FMargin(44.0f, 36.0f));
	if (UBorderSlot* OverlayContentSlot = Cast<UBorderSlot>(Overlay->AddChild(Panel)))
	{
		OverlayContentSlot->SetHorizontalAlignment(HAlign_Center);
		OverlayContentSlot->SetVerticalAlignment(VAlign_Center);
	}

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->AddChild(Content);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(NSLOCTEXT("TowerDefense", "StartTitle", "Grove Defense"));
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.84f, 0.28f)));
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 48));
	TitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
	TitleText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));

	if (UVerticalBoxSlot* TitleSlot = Content->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 8.0f));
	}

	SubtitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("SubtitleText"));
	SubtitleText->SetText(NSLOCTEXT("TowerDefense", "StartSubtitle", "Hold the heartwood"));
	SubtitleText->SetJustification(ETextJustify::Center);
	SubtitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.78f, 0.90f, 0.55f)));
	SubtitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 18));

	if (UVerticalBoxSlot* SubtitleSlot = Content->AddChildToVerticalBox(SubtitleText))
	{
		SubtitleSlot->SetHorizontalAlignment(HAlign_Center);
		SubtitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 28.0f));
	}

	PlayButton = MakeMenuButton(TEXT("PlayButton"), NSLOCTEXT("TowerDefense", "PlayButton", "Play"), FLinearColor(0.18f, 0.48f, 0.20f));
	ExitButton = MakeMenuButton(TEXT("ExitButton"), NSLOCTEXT("TowerDefense", "ExitButton", "Exit"), FLinearColor(0.42f, 0.16f, 0.12f));

	auto AddButton = [this, Content](UButton* Button)
	{
		USizeBox* ButtonSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ButtonSize->SetWidthOverride(260.0f);
		ButtonSize->SetHeightOverride(58.0f);
		ButtonSize->AddChild(Button);

		if (UVerticalBoxSlot* ButtonSlot = Content->AddChildToVerticalBox(ButtonSize))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Center);
			ButtonSlot->SetPadding(FMargin(0.0f, 8.0f));
		}
	};

	AddButton(PlayButton);
	AddButton(ExitButton);
}

UButton* UStartMenuWidget::MakeMenuButton(FName Name, const FText& Label, const FLinearColor& Color)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(Color);

	UTextBlock* ButtonLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonLabel->SetText(Label);
	ButtonLabel->SetJustification(ETextJustify::Center);
	ButtonLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	ButtonLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 24));
	Button->AddChild(ButtonLabel);
	UIAudio::BindButtonPressSound(Button);
	return Button;
}

void UStartMenuWidget::ShowStartMenu()
{
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	SetKeyboardFocus();
}

void UStartMenuWidget::HideStartMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UStartMenuWidget::IsStartMenuVisible() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}

void UStartMenuWidget::HandlePlayClicked()
{
	ATowerDefenseGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATowerDefenseGameMode>() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Play failed: TowerDefenseGameMode was not found."));
		return;
	}

	HideStartMenu();
	GameMode->StartNewGame();
}

void UStartMenuWidget::HandleExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
