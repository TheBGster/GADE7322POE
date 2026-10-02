// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "TowerDefenseGameState.h"
#include "TowerDefenseHUD.generated.h"

class UTowerDefenseHUDWidget;
class UGameOverWidget;
class UPauseMenuWidget;


UCLASS()
class GADE7322POE_API ATowerDefenseHUD : public AHUD
{
	GENERATED_BODY()

public:
	ATowerDefenseHUD();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|UI")
	UTowerDefenseHUDWidget* GetHUDWidget() const { return HUDWidget; }

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void HidePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|UI")
	bool IsPauseMenuVisible() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void HandleMatchStateChanged(ETowerDefenseMatchState NewState);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TSubclassOf<UTowerDefenseHUDWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TSubclassOf<UPauseMenuWidget> PauseMenuWidgetClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TObjectPtr<UTowerDefenseHUDWidget> HUDWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TObjectPtr<UGameOverWidget> GameOverWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Defense|UI")
	TObjectPtr<UPauseMenuWidget> PauseMenuWidget;

	void CreateWidgets();
	void RemoveWidgets();
	void BindToGameState();
	void UnbindFromGameState();
	void ApplyMatchState(ETowerDefenseMatchState NewState);
	void SetGameplayInputMode();
	void SetGameOverInputMode();
	void SetPauseInputMode();

	TWeakObjectPtr<ATowerDefenseGameState> BoundGameState;
};
