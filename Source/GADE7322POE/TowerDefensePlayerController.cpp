// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefensePlayerController.h"
#include "GADE7322POE.h"
#include "BananaCannon.h"
#include "DefenderBase.h"
#include "DefenderPlacementPoint.h"
#include "DefenderPlacementPreview.h"
#include "EngineUtils.h"
#include "Framework/Application/SlateApplication.h"
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
	bHasActiveSelection = false;
	SelectedDefenderKind = EDefenderKind::JungleScout;
	SelectedDefenderClass = ADefenderBase::StaticClass();
	BuildDefaultCatalog();
}

void ATowerDefensePlayerController::BuildDefaultCatalog()
{
	DefenderCatalog.Reset();

	FDefenderCatalogEntry Scout;
	Scout.Kind = EDefenderKind::JungleScout;
	Scout.DisplayName = FText::FromString(TEXT("Jungle Scout"));
	Scout.Description = FText::FromString(TEXT("Rapid-fire defender. Reliable against groups of weaker enemies."));
	Scout.DefenderClass = ADefenderBase::StaticClass();
	Scout.AccentColor = FLinearColor(0.32f, 0.42f, 0.22f, 1.0f);
	DefenderCatalog.Add(Scout);

	FDefenderCatalogEntry Cannon;
	Cannon.Kind = EDefenderKind::BananaCannon;
	Cannon.DisplayName = FText::FromString(TEXT("Banana Cannon"));
	Cannon.Description = FText::FromString(TEXT("Long-range, high damage. Prefers tanky targets like Gorillas."));
	Cannon.DefenderClass = ABananaCannon::StaticClass();
	Cannon.AccentColor = FLinearColor(0.95f, 0.80f, 0.10f, 1.0f);
	DefenderCatalog.Add(Cannon);

	FDefenderCatalogEntry Vine;
	Vine.Kind = EDefenderKind::VineTrap;
	Vine.DisplayName = FText::FromString(TEXT("Vine Trap"));
	Vine.Description = FText::FromString(TEXT("Slows enemies in a small area. Strong against Chimp Raiders."));
	Vine.DefenderClass = AVineTrap::StaticClass();
	Vine.AccentColor = FLinearColor(0.08f, 0.42f, 0.12f, 1.0f);
	DefenderCatalog.Add(Vine);
}

void ATowerDefensePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);

	if (ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		GameState->OnResourcesChanged.AddDynamic(this, &ThisClass::HandleResourcesChanged);
		GameState->OnMatchStateChanged.AddDynamic(this, &ThisClass::HandleMatchStateChanged);
	}

	OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());
	UE_LOG(LogTowerDefense, Log, TEXT("Player controller ready. Choose a defender from the bottom bar, then click a pad."));
}

void ATowerDefensePlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		GameState->OnResourcesChanged.RemoveDynamic(this, &ThisClass::HandleResourcesChanged);
		GameState->OnMatchStateChanged.RemoveDynamic(this, &ThisClass::HandleMatchStateChanged);
	}

	DestroyPlacementPreview();
	ClearPadHighlights();
	Super::EndPlay(EndPlayReason);
}

void ATowerDefensePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &ThisClass::HandleSelectPressed);
		InputComponent->BindKey(EKeys::RightMouseButton, IE_Pressed, this, &ThisClass::HandleCancelPressed);
		InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &ThisClass::HandleCancelPressed);
		InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ThisClass::SelectJungleScout);
		InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ThisClass::SelectBananaCannon);
		InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ThisClass::SelectVineTrap);
	}
}

void ATowerDefensePlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bHasActiveSelection)
	{
		UpdatePlacementPreview();
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

void ATowerDefensePlayerController::HandleDefenderCardClicked(EDefenderKind Kind)
{
	if (bHasActiveSelection && SelectedDefenderKind == Kind)
	{
		ClearDefenderSelection();
		return;
	}

	SelectDefenderKind(Kind);
}

void ATowerDefensePlayerController::SelectDefenderKind(EDefenderKind Kind)
{
	ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
	if (GameState && !GameState->CanAffordDefenderKind(Kind))
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot select %s: not enough resources."),
			*UEnum::GetValueAsString(Kind));
		return;
	}

	SelectedDefenderKind = Kind;
	ResolveSelectedClass();
	bHasActiveSelection = true;
	EnsurePlacementPreview();
	OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());
	UE_LOG(LogTowerDefense, Log, TEXT("Selected defender: %s (cost %d)."),
		*GetSelectedDefenderDisplayName().ToString(), GetSelectedDefenderCost());
}

void ATowerDefensePlayerController::ClearDefenderSelection()
{
	if (!bHasActiveSelection)
	{
		OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());
		return;
	}

	bHasActiveSelection = false;
	DestroyPlacementPreview();
	ClearPadHighlights();
	OnSelectedDefenderChanged.Broadcast(SelectedDefenderKind, GetSelectedDefenderCost());
	UE_LOG(LogTowerDefense, Log, TEXT("Defender selection cancelled."));
}

void ATowerDefensePlayerController::HandleCancelPressed()
{
	if (IsCursorOverInteractiveWidget() && !IsInputKeyDown(EKeys::Escape))
	{
		return;
	}

	ClearDefenderSelection();
}

void ATowerDefensePlayerController::HandleResourcesChanged(int32 NewResourceAmount)
{
	if (bHasActiveSelection && !CanAffordSelected())
	{
		ClearDefenderSelection();
	}

	(void)NewResourceAmount;
}

void ATowerDefensePlayerController::HandleMatchStateChanged(ETowerDefenseMatchState NewState)
{
	if (NewState != ETowerDefenseMatchState::InProgress)
	{
		ClearDefenderSelection();
	}
}

void ATowerDefensePlayerController::ResolveSelectedClass()
{
	if (const FDefenderCatalogEntry* Entry = FindCatalogEntry(SelectedDefenderKind))
	{
		if (Entry->DefenderClass)
		{
			SelectedDefenderClass = Entry->DefenderClass;
			return;
		}
	}

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
	if (!bHasActiveSelection)
	{
		return FText::FromString(TEXT("None"));
	}

	if (const FDefenderCatalogEntry* Entry = FindCatalogEntry(SelectedDefenderKind))
	{
		if (!Entry->DisplayName.IsEmpty())
		{
			return Entry->DisplayName;
		}
	}

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

const FDefenderCatalogEntry* ATowerDefensePlayerController::FindCatalogEntry(EDefenderKind Kind) const
{
	for (const FDefenderCatalogEntry& Entry : DefenderCatalog)
	{
		if (Entry.Kind == Kind)
		{
			return &Entry;
		}
	}

	return nullptr;
}

bool ATowerDefensePlayerController::CanAffordSelected() const
{
	const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr;
	return GameState && GameState->CanAffordDefenderKind(SelectedDefenderKind);
}

void ATowerDefensePlayerController::HandleSelectPressed()
{
	if (IsCursorOverInteractiveWidget())
	{
		return;
	}

	if (!bHasActiveSelection)
	{
		return;
	}

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
	if (!PlacementPoint || !bHasActiveSelection)
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

void ATowerDefensePlayerController::EnsurePlacementPreview()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!IsValid(PlacementPreview))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		PlacementPreview = World->SpawnActor<ADefenderPlacementPreview>(ADefenderPlacementPreview::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	}

	if (IsValid(PlacementPreview))
	{
		PlacementPreview->ConfigureFromDefender(SelectedDefenderClass);
		PlacementPreview->HidePreview();
	}
}

void ATowerDefensePlayerController::DestroyPlacementPreview()
{
	if (IsValid(PlacementPreview))
	{
		PlacementPreview->Destroy();
	}

	PlacementPreview = nullptr;
}

void ATowerDefensePlayerController::UpdatePlacementPreview()
{
	if (!bHasActiveSelection)
	{
		return;
	}

	EnsurePlacementPreview();
	if (!IsValid(PlacementPreview))
	{
		return;
	}

	if (IsCursorOverInteractiveWidget())
	{
		PlacementPreview->HidePreview();
		RefreshPadHighlights(nullptr);
		return;
	}

	FHitResult HitResult;
	ADefenderPlacementPoint* HoveredPad = nullptr;
	if (GetSelectionHit(HitResult))
	{
		HoveredPad = Cast<ADefenderPlacementPoint>(HitResult.GetActor());
	}

	const bool bCanPlace = HoveredPad && HoveredPad->CanPlaceDefender() && CanAffordSelected();
	if (HoveredPad)
	{
		PlacementPreview->ShowAt(HoveredPad->GetPlacementLocation());
		PlacementPreview->SetPreviewValid(bCanPlace);
	}
	else
	{
		PlacementPreview->HidePreview();
	}

	RefreshPadHighlights(HoveredPad);
}

void ATowerDefensePlayerController::RefreshPadHighlights(ADefenderPlacementPoint* HoveredPad)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const bool bCanAfford = CanAffordSelected();
	for (TActorIterator<ADefenderPlacementPoint> It(World); It; ++It)
	{
		ADefenderPlacementPoint* Pad = *It;
		if (!IsValid(Pad) || Pad->IsOccupied())
		{
			continue;
		}

		if (!bCanAfford)
		{
			Pad->SetPlacementHighlight(EPlacementPadHighlight::Invalid);
			continue;
		}

		if (Pad == HoveredPad)
		{
			Pad->SetPlacementHighlight(Pad->CanPlaceDefender()
				? EPlacementPadHighlight::ValidHover
				: EPlacementPadHighlight::Invalid);
		}
		else
		{
			Pad->SetPlacementHighlight(EPlacementPadHighlight::Available);
		}
	}
}

void ATowerDefensePlayerController::ClearPadHighlights()
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ADefenderPlacementPoint> It(World); It; ++It)
		{
			if (IsValid(*It))
			{
				It->SetPlacementHighlight(EPlacementPadHighlight::None);
			}
		}
	}
}

bool ATowerDefensePlayerController::IsCursorOverInteractiveWidget() const
{
	if (!FSlateApplication::IsInitialized())
	{
		return false;
	}

	FSlateApplication& SlateApp = FSlateApplication::Get();
	const FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(
		SlateApp.GetCursorPos(),
		SlateApp.GetInteractiveTopLevelWindows());

	if (!WidgetPath.IsValid())
	{
		return false;
	}

	for (int32 Index = WidgetPath.Widgets.Num() - 1; Index >= 0; --Index)
	{
		const SWidget& Widget = WidgetPath.Widgets[Index].Widget.Get();
		if (!Widget.GetVisibility().IsHitTestVisible())
		{
			continue;
		}

		const FName TypeName = Widget.GetType();
		if (TypeName == TEXT("SWindow")
			|| TypeName == TEXT("SOverlay")
			|| TypeName == TEXT("SCanvas")
			|| TypeName == TEXT("SConstraintCanvas")
			|| TypeName == TEXT("SInvalidationPanel")
			|| TypeName == TEXT("SGameLayerManager")
			|| TypeName == TEXT("SViewport")
			|| TypeName == TEXT("SObjectWidget"))
		{
			continue;
		}

		if (Widget.IsInteractable())
		{
			return true;
		}

		if (TypeName == TEXT("SButton") || TypeName == TEXT("SBorder") || TypeName == TEXT("SBox"))
		{
			return true;
		}
	}

	return false;
}

bool ATowerDefensePlayerController::GetSelectionHit(FHitResult& OutHit) const
{
	return GetHitResultUnderCursor(ECC_Visibility, false, OutHit);
}
