// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefensePlayerController.h"
#include "GADE7322POE.h"
#include "BananaCannon.h"
#include "DefenderBase.h"
#include "DefenderPlacementPoint.h"
#include "GameFramework/DamageType.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TowerDefenseGameState.h"
#include "VineTrap.h"

ATowerDefensePlayerController::ATowerDefensePlayerController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;
	DefaultMouseCursor = EMouseCursor::Default;
	DebugDamageAmount = 25.0f;
	bApplyDebugDamageOnSelect = false;
	SelectedDefenderKind = EDefenderKind::JungleScout;
	SelectedDefenderClass = ADefenderBase::StaticClass();
}

void ATowerDefensePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	ResolveSelectedClass();
	OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());

	UE_LOG(LogTowerDefense, Log, TEXT("Player controller ready. Press 1/2/3 to choose a defender, then click a pad."));
}

void ATowerDefensePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::HandleSelectPressed);
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::SelectJungleScout);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::SelectBananaCannon);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::SelectVineTrap);
	}
}

void ATowerDefensePlayerController::SelectJungleScout()
{
	SelectDefenderKind(EDefenderKind::JungleScout);
}

void ATowerDefensePlayerController::SelectBananaCannon()
{
	SelectDefenderKind(EDefenderKind::BananaCannon);
}

void ATowerDefensePlayerController::SelectVineTrap()
{
	SelectDefenderKind(EDefenderKind::VineTrap);
}

void ATowerDefensePlayerController::SelectDefenderKind(EDefenderKind Kind)
{
	SelectedDefenderKind = Kind;
	ResolveSelectedClass();
	OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());
	UE_LOG(LogTowerDefense, Log, TEXT("Selected defender: %s (cost %d)."),
		*GetSelectedDefenderDisplayName().ToString(), GetSelectedDefenderCost());
}

void ATowerDefensePlayerController::ResolveSelectedClass()
{
	switch (SelectedDefenderKind)
	{
	case EDefenderKind::BananaCannon:
		SelectedDefenderClass = ABananaCannon::StaticClass();
		break;
	case EDefenderKind::VineTrap:
		SelectedDefenderClass = AVineTrap::StaticClass();
		break;
	default:
		SelectedDefenderClass = ADefenderBase::StaticClass();
		break;
	}
}

int32 ATowerDefensePlayerController::GetSelectedDefenderCost() const
{
	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		return GameState->GetDefenderCostForKind(SelectedDefenderKind);
	}

	switch (SelectedDefenderKind)
	{
	case EDefenderKind::BananaCannon:
		return 50;
	case EDefenderKind::VineTrap:
		return 35;
	default:
		return 25;
	}
}

FText ATowerDefensePlayerController::GetSelectedDefenderDisplayName() const
{
	switch (SelectedDefenderKind)
	{
	case EDefenderKind::BananaCannon:
		return FText::FromString(TEXT("Banana Cannon"));
	case EDefenderKind::VineTrap:
		return FText::FromString(TEXT("Vine Trap"));
	default:
		return FText::FromString(TEXT("Jungle Scout"));
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
	if (!GameState || !GameState->IsMatchInProgress())
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot place a defender: the match is not in progress."));
		return;
	}

	if (!PlacementPoint->CanPlaceDefender())
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("That placement point is already occupied."));
		return;
	}

	ResolveSelectedClass();
	const int32 Cost = GameState->GetDefenderCostForKind(SelectedDefenderKind);
	if (!GameState->SpendResources(Cost))
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot afford %s. Cost: %d  Current resources: %d"),
			*GetSelectedDefenderDisplayName().ToString(), Cost, GameState->GetCurrentResources());
		return;
	}

	if (!PlacementPoint->PlaceDefenderOfClass(SelectedDefenderClass))
	{
		GameState->AddResources(Cost);
	}
}

bool ATowerDefensePlayerController::GetSelectionHit(FHitResult& OutHit) const
{
	return GetHitResultUnderCursor(ECC_Visibility, false, OutHit);
}
