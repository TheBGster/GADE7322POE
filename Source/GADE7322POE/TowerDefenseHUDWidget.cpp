// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseHUDWidget.h"
#include "TowerDefenseGameState.h"
#include "TowerDefensePlayerController.h"
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

	LastEnemiesInWave = 0;
	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	BindToGameState();
	BindToPlayerController();
	RefreshFromGameState();
}

void UTowerDefenseHUDWidget::NativeDestruct()
{
	UnbindFromGameState();
	UnbindFromPlayerController();
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

	WaveText = MakeHUDText(WidgetTree, TEXT("WaveText"), TEXT("Wave: --"), 18, FLinearColor(0.75f, 0.90f, 1.0f));
	ResourcesText = MakeHUDText(WidgetTree, TEXT("ResourcesText"), TEXT("Resources: --"), 18, FLinearColor::White);
	DefenderCostText = MakeHUDText(WidgetTree, TEXT("DefenderCostText"), TEXT("Scout 25  |  Cannon 50  |  Vine 35"), 15, FLinearColor(0.85f, 0.85f, 0.85f));
	SelectedDefenderText = MakeHUDText(WidgetTree, TEXT("SelectedDefenderText"), TEXT("Selected: Jungle Scout"), 16, FLinearColor(0.95f, 0.85f, 0.35f));

	auto AddInfoChild = [InfoBox](UWidget* Child, float BottomPadding)
	{
		if (UVerticalBoxSlot* Slot = InfoBox->AddChildToVerticalBox(Child))
		{
			Slot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, BottomPadding));
		}
	};

	AddInfoChild(TowerHealthText, 6.0f);
	AddInfoChild(HealthBarSize, 8.0f);
	AddInfoChild(WaveText, 8.0f);
	AddInfoChild(ResourcesText, 4.0f);
	AddInfoChild(DefenderCostText, 4.0f);
	AddInfoChild(SelectedDefenderText, 0.0f);

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
		TEXT("1 Scout   2 Cannon   3 Vine Trap   |   Click a pad to place."),
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
	GameState->OnWaveChanged.AddDynamic(this, &UTowerDefenseHUDWidget::HandleWaveChanged);
}

void UTowerDefenseHUDWidget::UnbindFromGameState()
{
	if (ATowerDefenseGameState* GameState = BoundGameState.Get())
	{
		GameState->OnResourcesChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleResourcesChanged);
		GameState->OnTowerHealthChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleTowerHealthChanged);
		GameState->OnWaveChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleWaveChanged);
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
		UpdateWaveDisplay(0, 0, 0);
		UpdateSelectedDefenderDisplay();
		OnHUDUpdated(0, 0, 0.0f, 0.0f);
		return;
	}

	const int32 Resources = GameState->GetCurrentResources();
	const int32 DefenderCost = GameState->GetDefenderCost();
	const float CurrentHealth = GameState->GetTowerCurrentHealth();
	const float MaxHealth = GameState->GetTowerMaxHealth();

	UpdateResourcesDisplay(Resources, DefenderCost);
	UpdateTowerHealthDisplay(CurrentHealth, MaxHealth);
	UpdateWaveDisplay(GameState->GetCurrentWave(), LastEnemiesInWave, GameState->GetEnemiesDefeated());
	UpdateSelectedDefenderDisplay();
	OnHUDUpdated(Resources, DefenderCost, CurrentHealth, MaxHealth);
}

void UTowerDefenseHUDWidget::HandleResourcesChanged(int32 NewResourceAmount)
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 DefenderCost = GameState ? GameState->GetDefenderCost() : 0;
	const float CurrentHealth = GameState ? GameState->GetTowerCurrentHealth() : 0.0f;
	const float MaxHealth = GameState ? GameState->GetTowerMaxHealth() : 0.0f;

	UpdateResourcesDisplay(NewResourceAmount, DefenderCost);
	if (GameState)
	{
		UpdateWaveDisplay(GameState->GetCurrentWave(), LastEnemiesInWave, GameState->GetEnemiesDefeated());
	}
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

void UTowerDefenseHUDWidget::HandleWaveChanged(int32 NewWave, int32 EnemiesInWave)
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 Defeated = GameState ? GameState->GetEnemiesDefeated() : 0;
	UpdateWaveDisplay(NewWave, EnemiesInWave, Defeated);
}

void UTowerDefenseHUDWidget::HandleSelectedDefenderChanged(EDefenderKind Kind, int32 Cost)
{
	UpdateSelectedDefenderDisplay();
	if (const ATowerDefenseGameState* GameState = GetTowerDefenseGameState())
	{
		UpdateResourcesDisplay(GameState->GetCurrentResources(), Cost);
	}

	(void)Kind;
}

void UTowerDefenseHUDWidget::BindToPlayerController()
{
	ATowerDefensePlayerController* PlayerController = Cast<ATowerDefensePlayerController>(GetOwningPlayer());
	if (BoundPlayerController.Get() == PlayerController)
	{
		return;
	}

	UnbindFromPlayerController();

	if (!PlayerController)
	{
		return;
	}

	BoundPlayerController = PlayerController;
	PlayerController->OnSelectedDefenderChanged.AddDynamic(this, &UTowerDefenseHUDWidget::HandleSelectedDefenderChanged);
}

void UTowerDefenseHUDWidget::UnbindFromPlayerController()
{
	if (ATowerDefensePlayerController* PlayerController = BoundPlayerController.Get())
	{
		PlayerController->OnSelectedDefenderChanged.RemoveDynamic(this, &UTowerDefenseHUDWidget::HandleSelectedDefenderChanged);
	}

	BoundPlayerController.Reset();
}

void UTowerDefenseHUDWidget::UpdateWaveDisplay(int32 Wave, int32 EnemiesInWave, int32 EnemiesDefeated)
{
	LastEnemiesInWave = EnemiesInWave;

	if (!WaveText)
	{
		return;
	}

	if (Wave <= 0)
	{
		WaveText->SetText(NSLOCTEXT("TowerDefense", "WaveWaitingHUD", "Wave: preparing..."));
		return;
	}

	if (EnemiesInWave > 0)
	{
		WaveText->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "WaveHUDWithCount", "Wave {0}   |   {1} incoming   |   Defeated {2}"),
			FText::AsNumber(Wave),
			FText::AsNumber(EnemiesInWave),
			FText::AsNumber(EnemiesDefeated)));
	}
	else
	{
		WaveText->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "WaveHUD", "Wave {0}   |   Defeated {1}"),
			FText::AsNumber(Wave),
			FText::AsNumber(EnemiesDefeated)));
	}
}

void UTowerDefenseHUDWidget::UpdateSelectedDefenderDisplay()
{
	const ATowerDefensePlayerController* PlayerController = BoundPlayerController.Get();
	if (!PlayerController)
	{
		PlayerController = Cast<ATowerDefensePlayerController>(GetOwningPlayer());
	}

	const FText Name = PlayerController
		? PlayerController->GetSelectedDefenderDisplayName()
		: FText::FromString(TEXT("Jungle Scout"));
	const int32 Cost = PlayerController ? PlayerController->GetSelectedDefenderCost() : 25;

	if (SelectedDefenderText)
	{
		SelectedDefenderText->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "SelectedDefenderHUD", "Selected: {0} ({1})"),
			Name,
			FText::AsNumber(Cost)));
	}
}

void UTowerDefenseHUDWidget::UpdateResourcesDisplay(int32 Resources, int32 DefenderCost)
{
	const ATowerDefenseGameState* GameState = GetTowerDefenseGameState();
	const int32 ScoutCost = GameState ? GameState->GetDefenderCostForKind(EDefenderKind::JungleScout) : 25;
	const int32 CannonCost = GameState ? GameState->GetDefenderCostForKind(EDefenderKind::BananaCannon) : 50;
	const int32 VineCost = GameState ? GameState->GetDefenderCostForKind(EDefenderKind::VineTrap) : 35;

	int32 SelectedCost = DefenderCost;
	FText SelectedName = FText::FromString(TEXT("Jungle Scout"));
	if (const ATowerDefensePlayerController* PlayerController = BoundPlayerController.Get())
	{
		SelectedCost = PlayerController->GetSelectedDefenderCost();
		SelectedName = PlayerController->GetSelectedDefenderDisplayName();
	}

	const bool bCanAfford = SelectedCost > 0 && Resources >= SelectedCost;
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
		DefenderCostText->SetText(FText::Format(
			NSLOCTEXT("TowerDefense", "DefenderCostsHUD", "Scout {0}  |  Cannon {1}  |  Vine {2}"),
			FText::AsNumber(ScoutCost),
			FText::AsNumber(CannonCost),
			FText::AsNumber(VineCost)));
	}

	UpdateSelectedDefenderDisplay();

	if (InstructionsText)
	{
		InstructionsText->SetText(bCanAfford
			? FText::Format(
				NSLOCTEXT("TowerDefense", "PlaceSelectedInstruction", "1 / 2 / 3 to choose   |   Click a pad to place {0}."),
				SelectedName)
			: FText::Format(
				NSLOCTEXT("TowerDefense", "NeedResourcesSelected", "Need {0} resources for {1}. Defeat enemies or pick a cheaper defender."),
				FText::AsNumber(SelectedCost),
				SelectedName));
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
