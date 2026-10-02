// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
class UTextBlock;


UCLASS()
class GADE7322POE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void ShowPauseMenu();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void HidePauseMenu();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|UI")
	bool IsPauseMenuVisible() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;
	virtual FReply NativeOnPreviewKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void HandleResumeClicked();

	UFUNCTION()
	void HandleRestartClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UButton> ResumeButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UButton> RestartButton;

	void BuildDefaultLayout();
	UButton* MakeMenuButton(FName Name, const FText& Label, const FLinearColor& Color);
};
