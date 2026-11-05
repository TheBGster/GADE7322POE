// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartMenuWidget.generated.h"

class UButton;
class UTextBlock;


UCLASS()
class GADE7322POE_API UStartMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void ShowStartMenu();

	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void HideStartMenu();

	UFUNCTION(BlueprintPure, Category = "Tower Defense|UI")
	bool IsStartMenuVisible() const;

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeConstruct() override;

	UFUNCTION()
	void HandlePlayClicked();

	UFUNCTION()
	void HandleExitClicked();

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UTextBlock> SubtitleText;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UButton> PlayButton;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional), Category = "Tower Defense|UI")
	TObjectPtr<UButton> ExitButton;

	void BuildDefaultLayout();
	UButton* MakeMenuButton(FName Name, const FText& Label, const FLinearColor& Color);
};
