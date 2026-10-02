// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TowerDefenseGameState.h"
#include "TowerDefenseTypes.h"
#include "TowerDefensePlayerController.generated.h"

class ADefenderBase;
class ADefenderPlacementPoint;
class ADefenderPlacementPreview;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSelectedDefenderChanged, EDefenderKind, Kind, int32, Cost);


UCLASS()
class GADE7322POE_API ATowerDefensePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ATowerDefensePlayerController();

	UPROPERTY(BlueprintAssignable, Category = "Tower Defense|Events")
	FOnSelectedDefenderChanged OnSelectedDefenderChanged;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	EDefenderKind GetSelectedDefenderKind() const { return SelectedDefenderKind; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	TSubclassOf<ADefenderBase> GetSelectedDefenderClass() const { return SelectedDefenderClass; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	int32 GetSelectedDefenderCost() const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	FText GetSelectedDefenderDisplayName() const;

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	bool HasActiveDefenderSelection() const { return bHasActiveSelection; }

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Placement")
	const TArray<FDefenderCatalogEntry>& GetDefenderCatalog() const { return DefenderCatalog; }

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Placement")
	void SelectDefenderKind(EDefenderKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Placement")
	void HandleDefenderCardClicked(EDefenderKind Kind);

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Placement")
	void ClearDefenderSelection();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Pause")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Pause")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|Pause")
	void ResumeGame();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|Pause")
	bool IsGamePausedMenu() const { return bIsPauseMenuOpen; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	UFUNCTION()
	void HandleSelectPressed();

	UFUNCTION()
	void HandleCancelPressed();

	UFUNCTION()
	void HandlePausePressed();

	UFUNCTION()
	void SelectJungleScout();

	UFUNCTION()
	void SelectBananaCannon();

	UFUNCTION()
	void SelectVineTrap();

	UFUNCTION()
	void HandleResourcesChanged(int32 NewResourceAmount);

	UFUNCTION()
	void HandleMatchStateChanged(ETowerDefenseMatchState NewState);

	bool GetSelectionHit(FHitResult& OutHit) const;
	bool IsCursorOverInteractiveWidget() const;
	ADefenderPlacementPoint* FindPlacementPointUnderCursor() const;
	ADefenderPlacementPoint* FindNearestPlacementPoint(const FVector& WorldLocation) const;
	void TryPlaceDefender(ADefenderPlacementPoint* PlacementPoint);
	void ShowPlacementFeedback(const FText& Message, const FLinearColor& Color);
	void ResolveSelectedClass();
	void BuildDefaultCatalog();
	void EnsurePlacementPreview();
	void DestroyPlacementPreview();
	void UpdatePlacementPreview();
	void RefreshPadHighlights(ADefenderPlacementPoint* HoveredPad);
	void ClearPadHighlights();
	bool CanAffordSelected() const;
	const FDefenderCatalogEntry* FindCatalogEntry(EDefenderKind Kind) const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	TArray<FDefenderCatalogEntry> DefenderCatalog;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	EDefenderKind SelectedDefenderKind;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	TSubclassOf<ADefenderBase> SelectedDefenderClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement")
	bool bHasActiveSelection;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|Pause")
	bool bIsPauseMenuOpen;

	UPROPERTY()
	TObjectPtr<ADefenderPlacementPreview> PlacementPreview;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Placement", meta = (ClampMin = "50.0"))
	float PlacementSnapRadius;

	/** Debug: left-clicking an actor with a Health Component applies this damage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug", meta = (ClampMin = "0.0"))
	float DebugDamageAmount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|Debug")
	bool bApplyDebugDamageOnSelect;
};
