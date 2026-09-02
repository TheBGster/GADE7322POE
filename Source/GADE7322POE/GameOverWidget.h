// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameOverWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * Game Over overlay with a Restart button that reloads the level.
 * Optional Blueprint child (WBP_GameOver) can restyle it using widget names:
 * TitleText, RestartButton.
 */
UCLASS()
class GADE7322POE_API UGameOverWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void RestartGame();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void ShowGameOver();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void HideGameOver();

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandleRestartClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UButton> RestartButton;

	void BuildDefaultLayout();
};
