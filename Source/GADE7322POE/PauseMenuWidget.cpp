// Copyright Epic Games, Inc. All Rights Reserved.

#include "PauseMenuWidget.h"
#include "GADE7322POE.h"
#include "TowerDefenseGameMode.h"
#include "TowerDefensePlayerController.h"
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
#include "InputCoreTypes.h"
#include "Styling/CoreStyle.h"

TSharedRef<SWidget> UPauseMenuWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UPauseMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleResumeClicked);
	}

	if (RestartButton)
	{
		RestartButton->OnClicked.AddUniqueDynamic(this, &ThisClass::HandleRestartClicked);
	}

	HidePauseMenu();
}

void UPauseMenuWidget::BuildDefaultLayout()
{
	UCanvasPanel* RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = RootCanvas;

	UBorder* Overlay = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Overlay"));
	Overlay->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.01f, 0.78f));
	Overlay->SetPadding(FMargin(48.0f));

	if (UCanvasPanelSlot* OverlaySlot = RootCanvas->AddChildToCanvas(Overlay))
	{
		OverlaySlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
		OverlaySlot->SetOffsets(FMargin(0.0f));
	}

	UBorder* Panel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Panel"));
	Panel->SetBrushColor(FLinearColor(0.16f, 0.10f, 0.04f, 0.96f));
	Panel->SetPadding(FMargin(36.0f, 28.0f));
	if (UBorderSlot* OverlayContentSlot = Cast<UBorderSlot>(Overlay->AddChild(Panel)))
	{
		OverlayContentSlot->SetHorizontalAlignment(HAlign_Center);
		OverlayContentSlot->SetVerticalAlignment(VAlign_Center);
	}

	UVerticalBox* Content = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Content"));
	Panel->AddChild(Content);

	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("TitleText"));
	TitleText->SetText(NSLOCTEXT("TowerDefense", "PauseTitle", "Paused"));
	TitleText->SetJustification(ETextJustify::Center);
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(0.98f, 0.84f, 0.28f)));
	TitleText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 40));
	TitleText->SetShadowOffset(FVector2D(2.0f, 2.0f));
	TitleText->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.9f));

	if (UVerticalBoxSlot* TitleSlot = Content->AddChildToVerticalBox(TitleText))
	{
		TitleSlot->SetHorizontalAlignment(HAlign_Center);
		TitleSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 24.0f));
	}

	ResumeButton = MakeMenuButton(TEXT("ResumeButton"), NSLOCTEXT("TowerDefense", "ResumeButton", "Resume"), FLinearColor(0.18f, 0.48f, 0.20f));
	RestartButton = MakeMenuButton(TEXT("RestartButton"), NSLOCTEXT("TowerDefense", "PauseRestartButton", "Restart"), FLinearColor(0.42f, 0.28f, 0.10f));

	auto AddButton = [this, Content](UButton* Button)
	{
		USizeBox* ButtonSize = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		ButtonSize->SetWidthOverride(240.0f);
		ButtonSize->SetHeightOverride(56.0f);
		ButtonSize->AddChild(Button);

		if (UVerticalBoxSlot* ButtonSlot = Content->AddChildToVerticalBox(ButtonSize))
		{
			ButtonSlot->SetHorizontalAlignment(HAlign_Center);
			ButtonSlot->SetPadding(FMargin(0.0f, 8.0f));
		}
	};

	AddButton(ResumeButton);
	AddButton(RestartButton);
}

UButton* UPauseMenuWidget::MakeMenuButton(FName Name, const FText& Label, const FLinearColor& Color)
{
	UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass(), Name);
	Button->SetBackgroundColor(Color);

	UTextBlock* ButtonLabel = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	ButtonLabel->SetText(Label);
	ButtonLabel->SetJustification(ETextJustify::Center);
	ButtonLabel->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	ButtonLabel->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 22));
	Button->AddChild(ButtonLabel);
	return Button;
}

void UPauseMenuWidget::ShowPauseMenu()
{
	SetVisibility(ESlateVisibility::Visible);
	SetIsFocusable(true);
	SetKeyboardFocus();
}

FReply UPauseMenuWidget::NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	const FKey Key = InKeyEvent.GetKey();
	if (Key == EKeys::Escape || Key == EKeys::P)
	{
		HandleResumeClicked();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

void UPauseMenuWidget::HidePauseMenu()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UPauseMenuWidget::IsPauseMenuVisible() const
{
	return GetVisibility() == ESlateVisibility::Visible;
}

void UPauseMenuWidget::HandleResumeClicked()
{
	if (ATowerDefensePlayerController* PlayerController = Cast<ATowerDefensePlayerController>(GetOwningPlayer()))
	{
		PlayerController->ResumeGame();
	}
}

void UPauseMenuWidget::HandleRestartClicked()
{
	if (ATowerDefensePlayerController* PlayerController = Cast<ATowerDefensePlayerController>(GetOwningPlayer()))
	{
		PlayerController->ResumeGame();
	}

	ATowerDefenseGameMode* GameMode = GetWorld() ? GetWorld()->GetAuthGameMode<ATowerDefenseGameMode>() : nullptr;
	if (!GameMode)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Pause Restart failed: TowerDefenseGameMode was not found."));
		return;
	}

	GameMode->RestartCurrentGame();
}
