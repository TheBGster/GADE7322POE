// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatStatusWidget.generated.h"

class UHealthComponent;
class UProgressBar;
class UTextBlock;


UCLASS()
class GADE7322POE_API UCombatStatusWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Tower Defense|UI")
	void BindToHealth(UHealthComponent* InHealth, const FText& InDisplayName);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> NameText;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> HealthBar;

	void BuildDefaultLayout();
	void UpdateDisplay(float CurrentHealth, float MaxHealth);

	TWeakObjectPtr<UHealthComponent> BoundHealth;
	FText DisplayName;
};
