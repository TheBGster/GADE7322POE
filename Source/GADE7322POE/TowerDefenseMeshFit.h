// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UStaticMeshComponent;
class UWidgetComponent;

namespace TowerDefenseMeshFit
{
	UStaticMesh* LoadMesh(const TCHAR* Path);

	float Apply(
		UStaticMeshComponent* MeshComp,
		UStaticMesh* Mesh,
		float TargetHeight,
		float MaxXY = 0.0f,
		float YawOffset = 0.0f,
		UWidgetComponent* HealthBar = nullptr);
}
