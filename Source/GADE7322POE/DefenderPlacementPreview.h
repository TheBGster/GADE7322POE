// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefenderPlacementPreview.generated.h"

class ADefenderBase;
class UStaticMeshComponent;
class USceneComponent;
class UMaterialInstanceDynamic;


UCLASS()
class GADE7322POE_API ADefenderPlacementPreview : public AActor
{
	GENERATED_BODY()

public:
	ADefenderPlacementPreview();

	void ConfigureFromDefender(TSubclassOf<ADefenderBase> DefenderClass);
	void SetPreviewValid(bool bIsValid);
	void ShowAt(const FVector& WorldLocation);
	void HidePreview();

protected:
	void ApplyPreviewColor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> PreviewMaterial;

	FLinearColor BaseColor;
	bool bIsValidPlacement;
};
