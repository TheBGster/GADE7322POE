// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UNiagaraSystem;
class UStaticMeshComponent;
class UWorld;

namespace JungleVisuals
{
	UMaterialInterface* GetFallbackMaterial();
	UMaterialInterface* GetSwayMaterial();
	UMaterialInterface* GetLivingGoldMaterial();
	UMaterialInterface* GetGrovePressureMaterial();
	UNiagaraSystem* GetBananaBurstSystem();
	UNiagaraSystem* GetUpgradeBloomSystem();

	UMaterialInstanceDynamic* ApplyBodyMaterial(
		UStaticMeshComponent* Mesh,
		UObject* Outer,
		const FLinearColor& Color,
		int32 UpgradeTier,
		float SwayAmount);

	void UpdateDynamicMaterial(
		UMaterialInstanceDynamic* Material,
		const FLinearColor& Color,
		int32 UpgradeTier,
		float SwayAmount,
		float WorldTime);

	void SpawnBananaBurst(UWorld* World, const FVector& Location);
	void SpawnUpgradeBloom(UWorld* World, const FVector& Location);
}
