// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefenderPlacementPreview.h"
#include "DefenderBase.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

ADefenderPlacementPreview::ADefenderPlacementPreview()
{
	PrimaryActorTick.bCanEverTick = false;
	BaseColor = FLinearColor(0.32f, 0.42f, 0.22f, 1.0f);
	bIsValidPlacement = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCastShadow(false);
}

void ADefenderPlacementPreview::ConfigureFromDefender(TSubclassOf<ADefenderBase> DefenderClass)
{
	const ADefenderBase* CDO = DefenderClass ? DefenderClass.GetDefaultObject() : nullptr;
	if (!CDO || !CDO->GetMeshComponent() || !MeshComponent)
	{
		SetActorHiddenInGame(true);
		return;
	}

	const UStaticMeshComponent* SourceMesh = CDO->GetMeshComponent();
	MeshComponent->SetStaticMesh(SourceMesh->GetStaticMesh());
	MeshComponent->SetRelativeScale3D(SourceMesh->GetRelativeScale3D());
	MeshComponent->SetRelativeLocation(SourceMesh->GetRelativeLocation());
	BaseColor = CDO->GetBodyColor();

	UMaterialInterface* SourceMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (SourceMaterial)
	{
		PreviewMaterial = UMaterialInstanceDynamic::Create(SourceMaterial, this);
		MeshComponent->SetMaterial(0, PreviewMaterial);
	}

	ApplyPreviewColor();
}

void ADefenderPlacementPreview::SetPreviewValid(bool bIsValid)
{
	bIsValidPlacement = bIsValid;
	ApplyPreviewColor();
}

void ADefenderPlacementPreview::ShowAt(const FVector& WorldLocation)
{
	SetActorLocation(WorldLocation);
	SetActorHiddenInGame(false);
}

void ADefenderPlacementPreview::HidePreview()
{
	SetActorHiddenInGame(true);
}

void ADefenderPlacementPreview::ApplyPreviewColor()
{
	if (!PreviewMaterial)
	{
		return;
	}

	const FLinearColor Color = bIsValidPlacement
		? FLinearColor(BaseColor.R * 0.55f + 0.20f, BaseColor.G * 0.55f + 0.45f, BaseColor.B * 0.35f, 1.0f)
		: FLinearColor(0.85f, 0.18f, 0.12f, 1.0f);

	PreviewMaterial->SetVectorParameterValue(TEXT("Color"), Color);
	PreviewMaterial->SetVectorParameterValue(TEXT("BaseColor"), Color);
}
