// Copyright Epic Games, Inc. All Rights Reserved.

#include "JungleVisuals.h"
#include "GADE7322POE.h"
#include "JungleBurstEffect.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"

namespace
{
UMaterialInterface* LoadMaterial(const TCHAR* Path)
{
	return LoadObject<UMaterialInterface>(nullptr, Path);
}

UNiagaraSystem* LoadNiagara(const TCHAR* Path)
{
	return LoadObject<UNiagaraSystem>(nullptr, Path);
}

void SpawnVisibleBurst(UWorld* World, const FVector& Location, const FLinearColor& Color, float Size, int32 ShardCount)
{
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	if (AJungleBurstEffect* Burst = World->SpawnActor<AJungleBurstEffect>(Location, FRotator::ZeroRotator, SpawnParams))
	{
		Burst->InitialiseBurst(Color, Size, ShardCount);
	}
}
}

UMaterialInterface* JungleVisuals::GetFallbackMaterial()
{
	return LoadMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
}

UMaterialInterface* JungleVisuals::GetSwayMaterial()
{
	if (UMaterialInterface* Material = LoadMaterial(TEXT("/Game/Materials/M_JungleSway.M_JungleSway")))
	{
		return Material;
	}
	return GetFallbackMaterial();
}

UMaterialInterface* JungleVisuals::GetLivingGoldMaterial()
{
	if (UMaterialInterface* Material = LoadMaterial(TEXT("/Game/Materials/M_LivingGold.M_LivingGold")))
	{
		return Material;
	}
	return GetFallbackMaterial();
}

UMaterialInterface* JungleVisuals::GetGrovePressureMaterial()
{
	return LoadMaterial(TEXT("/Game/Materials/M_PP_GrovePressure.M_PP_GrovePressure"));
}

UNiagaraSystem* JungleVisuals::GetBananaBurstSystem()
{
	return LoadNiagara(TEXT("/Game/VFX/NS_BananaBurst.NS_BananaBurst"));
}

UNiagaraSystem* JungleVisuals::GetUpgradeBloomSystem()
{
	return LoadNiagara(TEXT("/Game/VFX/NS_UpgradeBloom.NS_UpgradeBloom"));
}

UMaterialInstanceDynamic* JungleVisuals::ApplyBodyMaterial(
	UStaticMeshComponent* Mesh,
	UObject* Outer,
	const FLinearColor& Color,
	int32 UpgradeTier,
	float SwayAmount)
{
	if (!Mesh)
	{
		return nullptr;
	}

	UMaterialInterface* Source = (SwayAmount > 0.0f)
		? GetSwayMaterial()
		: ((UpgradeTier > 0) ? GetLivingGoldMaterial() : GetFallbackMaterial());
	if (!Source)
	{
		return nullptr;
	}

	UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Source, Outer);
	if (!Mid)
	{
		return nullptr;
	}

	UpdateDynamicMaterial(Mid, Color, UpgradeTier, SwayAmount, 0.0f);
	Mesh->SetMaterial(0, Mid);
	return Mid;
}

void JungleVisuals::UpdateDynamicMaterial(
	UMaterialInstanceDynamic* Material,
	const FLinearColor& Color,
	int32 UpgradeTier,
	float SwayAmount,
	float WorldTime)
{
	if (!Material)
	{
		return;
	}

	Material->SetVectorParameterValue(TEXT("Color"), Color);
	Material->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Material->SetScalarParameterValue(TEXT("UpgradeTier"), static_cast<float>(UpgradeTier));
	Material->SetScalarParameterValue(TEXT("SwayAmount"), SwayAmount);
	Material->SetScalarParameterValue(TEXT("SwaySpeed"), 1.6f + UpgradeTier * 0.35f);
	Material->SetScalarParameterValue(TEXT("Time"), WorldTime);
}

void JungleVisuals::SpawnBananaBurst(UWorld* World, const FVector& Location)
{
	const FVector BurstLocation = Location + FVector(0.0f, 0.0f, 60.0f);
	if (UNiagaraSystem* System = GetBananaBurstSystem())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			System,
			BurstLocation,
			FRotator::ZeroRotator,
			FVector(6.0f),
			true,
			true,
			ENCPoolMethod::None,
			false);
	}

	SpawnVisibleBurst(World, BurstLocation, FLinearColor(1.0f, 0.86f, 0.08f, 1.0f), 2.2f, 8);
}

void JungleVisuals::SpawnUpgradeBloom(UWorld* World, const FVector& Location)
{
	const FVector BurstLocation = Location + FVector(0.0f, 0.0f, 40.0f);
	if (UNiagaraSystem* System = GetUpgradeBloomSystem())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			World,
			System,
			BurstLocation,
			FRotator::ZeroRotator,
			FVector(26.0f),
			true,
			true,
			ENCPoolMethod::None,
			false);
	}

	SpawnVisibleBurst(World, BurstLocation, FLinearColor(0.98f, 0.78f, 0.12f, 1.0f), 8.5f, 16);
}
