// Copyright Epic Games, Inc. All Rights Reserved.

#include "CombatStatusWidget.h"
#include "HealthComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateBrush.h"
#include "Styling/SlateTypes.h"

TSharedRef<SWidget> UCombatStatusWidget::RebuildWidget()
{
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		BuildDefaultLayout();
	}

	return Super::RebuildWidget();
}

void UCombatStatusWidget::NativeDestruct()
{
	if (UHealthComponent* Health = BoundHealth.Get())
	{
		Health->OnHealthChanged.RemoveDynamic(this, &UCombatStatusWidget::HandleHealthChanged);
	}

	BoundHealth.Reset();
	Super::NativeDestruct();
}

void UCombatStatusWidget::BuildDefaultLayout()
{
	UVerticalBox* Root = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("Root"));
	Root->SetVisibility(ESlateVisibility::HitTestInvisible);
	WidgetTree->RootWidget = Root;

	NameText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("NameText"));
	NameText->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", 10));
	NameText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	NameText->SetJustification(ETextJustify::Center);
	NameText->SetVisibility(ESlateVisibility::HitTestInvisible);
	Root->AddChildToVerticalBox(NameText);

	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), TEXT("HealthBar"));
	HealthBar->SetPercent(1.0f);
	HealthBar->SetFillColorAndOpacity(FLinearColor(0.25f, 0.85f, 0.30f));
	HealthBar->SetVisibility(ESlateVisibility::HitTestInvisible);

	FSlateBrush Background;
	Background.DrawAs = ESlateBrushDrawType::Box;
	Background.TintColor = FSlateColor(FLinearColor(0.05f, 0.05f, 0.05f, 0.9f));
	FSlateBrush Fill;
	Fill.DrawAs = ESlateBrushDrawType::Box;
	Fill.TintColor = FSlateColor(FLinearColor::White);
	FProgressBarStyle Style;
	Style.SetBackgroundImage(Background);
	Style.SetFillImage(Fill);
	HealthBar->SetWidgetStyle(Style);

	if (UVerticalBoxSlot* BarSlot = Root->AddChildToVerticalBox(HealthBar))
	{
		BarSlot->SetPadding(FMargin(0.0f, 2.0f, 0.0f, 0.0f));
	}
}

void UCombatStatusWidget::BindToHealth(UHealthComponent* InHealth, const FText& InDisplayName)
{
	if (UHealthComponent* Previous = BoundHealth.Get())
	{
		Previous->OnHealthChanged.RemoveDynamic(this, &UCombatStatusWidget::HandleHealthChanged);
	}

	DisplayName = InDisplayName;
	BoundHealth = InHealth;

	if (InHealth)
	{
		InHealth->OnHealthChanged.AddUniqueDynamic(this, &UCombatStatusWidget::HandleHealthChanged);
		UpdateDisplay(InHealth->GetCurrentHealth(), InHealth->GetMaxHealth());
	}
}

void UCombatStatusWidget::HandleHealthChanged(float CurrentHealth, float MaxHealth)
{
	UpdateDisplay(CurrentHealth, MaxHealth);
}

void UCombatStatusWidget::UpdateDisplay(float CurrentHealth, float MaxHealth)
{
	if (NameText)
	{
		NameText->SetText(DisplayName);
	}

	const float Percent = MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
	if (HealthBar)
	{
		HealthBar->SetPercent(Percent);
		const FLinearColor Fill = Percent > 0.45f
			? FLinearColor(0.25f, 0.85f, 0.30f)
			: (Percent > 0.2f ? FLinearColor(0.92f, 0.72f, 0.18f) : FLinearColor(0.90f, 0.22f, 0.18f));
		HealthBar->SetFillColorAndOpacity(Fill);
	}
}
