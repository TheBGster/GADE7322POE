// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefensePlayerController.h"
#include "GADE7322POE.h"
#include "DefenderPlacementPoint.h"
#include "GameFramework/DamageType.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameState.h"

ATowerDefensePlayerController::ATowerDefensePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	DefenderCost = 25;
	DebugDamageAmount = 25.0f;
	bApplyDebugDamageOnSelect = false;
}

void ATowerDefensePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	UE_LOG(LogTowerDefense, Log, TEXT("Tower Defense player controller ready. Click a yellow pad to place a defender (cost %d)."), DefenderCost);
}

void ATowerDefensePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::HandleSelectPressed);
	}
}

void ATowerDefensePlayerController::HandleSelectPressed()
{
	FHitResult HitResult;
	if (!GetSelectionHit(HitResult))
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("Select pressed, no world hit."));
		return;
	}

	AActor* HitActor = HitResult.GetActor();
	UE_LOG(LogTowerDefense, Log, TEXT("Select hit '%s' at %s"),
		*GetNameSafe(HitActor),
		*HitResult.ImpactPoint.ToCompactString());

	if (ADefenderPlacementPoint* PlacementPoint = Cast<ADefenderPlacementPoint>(HitActor))
	{
		TryPlaceDefender(PlacementPoint);
		return;
	}

	if (!bApplyDebugDamageOnSelect || !HitActor)
	{
		return;
	}

	if (!HitActor->FindComponentByClass<UHealthComponent>())
	{
		return;
	}

	UGameplayStatics::ApplyDamage(HitActor, DebugDamageAmount, this, this, UDamageType::StaticClass());
}

void ATowerDefensePlayerController::TryPlaceDefender(ADefenderPlacementPoint* PlacementPoint)
{
	if (!PlacementPoint)
	{
		return;
	}

	ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
	if (!GameState || GameState->GetMatchState() != ETowerDefenseMatchState::InProgress)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot place a defender: the match is not in progress."));
		return;
	}

	if (!PlacementPoint->CanPlaceDefender())
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("That placement point is already occupied."));
		return;
	}

	if (!GameState->CanAfford(DefenderCost))
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot afford a defender. Cost: %d  Current resources: %d"),
			DefenderCost, GameState->GetCurrentResources());
		return;
	}

	if (PlacementPoint->PlaceDefender())
	{
		GameState->SpendResources(DefenderCost);
		UE_LOG(LogTowerDefense, Log, TEXT("Spent %d resources. Remaining: %d"),
			DefenderCost, GameState->GetCurrentResources());
	}
}

bool ATowerDefensePlayerController::GetSelectionHit(FHitResult& OutHit) const
{
	return GetHitResultUnderCursor(ECC_Visibility, false, OutHit);
}
