// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseMeshFit.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/StaticMesh.h"

UStaticMesh* TowerDefenseMeshFit::LoadMesh(const TCHAR* Path)
{
	if (!Path || !*Path)
	{
		return nullptr;
	}

	return LoadObject<UStaticMesh>(nullptr, Path);
}

float TowerDefenseMeshFit::Apply(
	UStaticMeshComponent* MeshComp,
	UStaticMesh* Mesh,
	float TargetHeight,
	float MaxXY,
	float YawOffset,
	UWidgetComponent* HealthBar)
{
	if (!MeshComp || !Mesh)
	{
		return 1.0f;
	}

	MeshComp->SetStaticMesh(Mesh);

	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const float MeshHeight = FMath::Max(Bounds.BoxExtent.Z * 2.0f, 1.0f);
	float Scale = FMath::Max(TargetHeight, 10.0f) / MeshHeight;

	if (MaxXY > 0.0f)
	{
		const float MeshXY = FMath::Max(Bounds.BoxExtent.X, Bounds.BoxExtent.Y) * 2.0f;
		if (MeshXY > 1.0f)
		{
			Scale = FMath::Min(Scale, MaxXY / MeshXY);
		}
	}

	MeshComp->SetRelativeScale3D(FVector(Scale));
	MeshComp->SetRelativeRotation(FRotator(0.0f, YawOffset, 0.0f));

	const float BottomZ = (Bounds.Origin.Z - Bounds.BoxExtent.Z) * Scale;
	MeshComp->SetRelativeLocation(FVector(0.0f, 0.0f, -BottomZ));

	if (HealthBar)
	{
		const float TopZ = (Bounds.Origin.Z + Bounds.BoxExtent.Z) * Scale + 18.0f;
		HealthBar->SetRelativeLocation(FVector(0.0f, 0.0f, TopZ));
	}

	return Scale;
}
