// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseHUD.h"
#include "GameOverWidget.h"
#include "PauseMenuWidget.h"
#include "TowerDefenseHUDWidget.h"
#include "GADE7322POE.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ATowerDefenseHUD::ATowerDefenseHUD()
{
	PrimaryActorTick.bCanEverTick = false;
	HUDWidgetClass = UTowerDefenseHUDWidget::StaticClass();
	GameOverWidgetClass = UGameOverWidget::StaticClass();
	PauseMenuWidgetClass = UPauseMenuWidget::StaticClass();
}

void ATowerDefenseHUD::BeginPlay()
{
	Super::BeginPlay();

	CreateWidgets();
	BindToGameState();

	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		ApplyMatchState(GameState->GetMatchState());
	}
	else
	{
		ApplyMatchState(ETowerDefenseMatchState::WaitingToStart);
	}
}

void ATowerDefenseHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindFromGameState();
	RemoveWidgets();
	Super::EndPlay(EndPlayReason);
}

void ATowerDefenseHUD::CreateWidgets()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("TowerDefenseHUD could not create widgets: no owning PlayerController."));
		return;
	}

	UClass* HUDClassToUse = HUDWidgetClass ? HUDWidgetClass.Get() : UTowerDefenseHUDWidget::StaticClass();
	HUDWidget = CreateWidget<UTowerDefenseHUDWidget>(PlayerController, HUDClassToUse);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport(0);
		UE_LOG(LogTowerDefense, Log, TEXT("HUD widget added to viewport."));
	}

	UClass* GameOverClassToUse = GameOverWidgetClass ? GameOverWidgetClass.Get() : UGameOverWidget::StaticClass();
	GameOverWidget = CreateWidget<UGameOverWidget>(PlayerController, GameOverClassToUse);
	if (GameOverWidget)
	{
		GameOverWidget->AddToViewport(10);
		GameOverWidget->HideGameOver();
		UE_LOG(LogTowerDefense, Log, TEXT("Game Over widget added to viewport."));
	}

	UClass* PauseClassToUse = PauseMenuWidgetClass ? PauseMenuWidgetClass.Get() : UPauseMenuWidget::StaticClass();
	PauseMenuWidget = CreateWidget<UPauseMenuWidget>(PlayerController, PauseClassToUse);
	if (PauseMenuWidget)
	{
		PauseMenuWidget->AddToViewport(8);
		PauseMenuWidget->HidePauseMenu();
		UE_LOG(LogTowerDefense, Log, TEXT("Pause menu widget added to viewport."));
	}
}

void ATowerDefenseHUD::RemoveWidgets()
{
	if (HUDWidget)
	{
		HUDWidget->RemoveFromParent();
		HUDWidget = nullptr;
	}

	if (GameOverWidget)
	{
		GameOverWidget->RemoveFromParent();
		GameOverWidget = nullptr;
	}

	if (PauseMenuWidget)
	{
		PauseMenuWidget->RemoveFromParent();
		PauseMenuWidget = nullptr;
	}
}

void ATowerDefenseHUD::BindToGameState()
{
	ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
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
	GameState->OnMatchStateChanged.AddDynamic(this, &ATowerDefenseHUD::HandleMatchStateChanged);
}

void ATowerDefenseHUD::UnbindFromGameState()
{
	if (ATowerDefenseGameState* GameState = BoundGameState.Get())
	{
		GameState->OnMatchStateChanged.RemoveDynamic(this, &ATowerDefenseHUD::HandleMatchStateChanged);
	}

	BoundGameState.Reset();
}

void ATowerDefenseHUD::HandleMatchStateChanged(ETowerDefenseMatchState NewState)
{
	ApplyMatchState(NewState);
}

void ATowerDefenseHUD::ApplyMatchState(ETowerDefenseMatchState NewState)
{
	if (NewState == ETowerDefenseMatchState::GameOver)
	{
		HidePauseMenu();

		if (GameOverWidget)
		{
			GameOverWidget->ShowGameOver();
		}

		SetGameOverInputMode();
		UE_LOG(LogTowerDefense, Log, TEXT("Game Over UI shown."));
		return;
	}

	if (GameOverWidget)
	{
		GameOverWidget->HideGameOver();
	}

	SetGameplayInputMode();
}

void ATowerDefenseHUD::SetGameplayInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void ATowerDefenseHUD::SetGameOverInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	if (GameOverWidget)
	{
		InputMode.SetWidgetToFocus(GameOverWidget->TakeWidget());
	}

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}

void ATowerDefenseHUD::ShowPauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->ShowPauseMenu();
	}

	SetPauseInputMode();
}

void ATowerDefenseHUD::HidePauseMenu()
{
	if (PauseMenuWidget)
	{
		PauseMenuWidget->HidePauseMenu();
	}

	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		if (GameState->GetMatchState() == ETowerDefenseMatchState::GameOver)
		{
			SetGameOverInputMode();
			return;
		}
	}

	SetGameplayInputMode();
}

bool ATowerDefenseHUD::IsPauseMenuVisible() const
{
	return PauseMenuWidget && PauseMenuWidget->IsPauseMenuVisible();
}

void ATowerDefenseHUD::SetPauseInputMode()
{
	APlayerController* PlayerController = GetOwningPlayerController();
	if (!PlayerController)
	{
		return;
	}

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	if (PauseMenuWidget)
	{
		InputMode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
	}

	PlayerController->SetInputMode(InputMode);
	PlayerController->bShowMouseCursor = true;
}
