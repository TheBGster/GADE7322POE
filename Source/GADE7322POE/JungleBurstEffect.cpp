// Copyright Epic Games, Inc. All Rights Reserved.

#include "JungleBurstEffect.h"
#include "JungleVisuals.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

AJungleBurstEffect::AJungleBurstEffect()
{
	PrimaryActorTick.bCanEverTick = true;
	BurstSize = 4.0f;
	Elapsed = 0.0f;
	Lifetime = 0.85f;
	BurstColor = FLinearColor(1.0f, 0.85f, 0.12f, 1.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	CoreMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CoreMesh"));
	CoreMesh->SetupAttachment(SceneRoot);
	CoreMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CoreMesh->SetGenerateOverlapEvents(false);
	CoreMesh->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMesh.Succeeded())
	{
		CoreMesh->SetStaticMesh(SphereMesh.Object);
	}
}

void AJungleBurstEffect::InitialiseBurst(const FLinearColor& Color, float Size, int32 ShardCount)
{
	BurstColor = Color;
	BurstSize = FMath::Max(Size, 1.0f);
	Lifetime = 0.75f + BurstSize * 0.08f;
	Elapsed = 0.0f;
	SetLifeSpan(Lifetime + 0.05f);

	UStaticMesh* Sphere = CoreMesh ? CoreMesh->GetStaticMesh() : nullptr;
	UMaterialInterface* Source = JungleVisuals::GetLivingGoldMaterial();
	if (!Source)
	{
		Source = JungleVisuals::GetFallbackMaterial();
	}

	auto TintMesh = [this, Source](UStaticMeshComponent* Mesh, const FLinearColor& Tint)
	{
		if (!Mesh || !Source)
		{
			return;
		}

		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Source, this))
		{
			Mid->SetVectorParameterValue(TEXT("Color"), Tint);
			Mid->SetVectorParameterValue(TEXT("BaseColor"), Tint);
			Mid->SetScalarParameterValue(TEXT("UpgradeTier"), 2.0f);
			Mesh->SetMaterial(0, Mid);
		}
	};

	if (CoreMesh)
	{
		CoreMesh->SetRelativeScale3D(FVector(BurstSize * 0.45f));
		TintMesh(CoreMesh, BurstColor);
	}

	ShardMeshes.Reset();
	ShardVelocities.Reset();

	const int32 Count = FMath::Clamp(ShardCount, 6, 18);
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FName ShardName(*FString::Printf(TEXT("BurstShard_%d"), Index));
		UStaticMeshComponent* Shard = NewObject<UStaticMeshComponent>(this, ShardName);
		if (!Shard)
		{
			continue;
		}

		Shard->SetupAttachment(SceneRoot);
		Shard->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Shard->SetGenerateOverlapEvents(false);
		Shard->SetCastShadow(false);
		if (Sphere)
		{
			Shard->SetStaticMesh(Sphere);
		}
		Shard->SetRelativeScale3D(FVector(BurstSize * 0.18f));
		Shard->RegisterComponent();
		TintMesh(Shard, BurstColor * FLinearColor(1.15f, 1.05f, 0.55f));

		const float Angle = (2.0f * PI * Index) / Count + FMath::FRandRange(-0.18f, 0.18f);
		const float Speed = 700.0f + BurstSize * 90.0f;
		const FVector Velocity(FMath::Cos(Angle) * Speed, FMath::Sin(Angle) * Speed, FMath::FRandRange(180.0f, 520.0f));
		ShardMeshes.Add(Shard);
		ShardVelocities.Add(Velocity);
	}
}

void AJungleBurstEffect::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Elapsed += DeltaTime;
	const float Alpha = FMath::Clamp(Elapsed / Lifetime, 0.0f, 1.0f);
	const float Grow = FMath::InterpEaseOut(BurstSize * 0.45f, BurstSize * 1.85f, Alpha, 1.6f);
	if (CoreMesh)
	{
		CoreMesh->SetRelativeScale3D(FVector(Grow));
	}

	for (int32 Index = 0; Index < ShardMeshes.Num(); ++Index)
	{
		UStaticMeshComponent* Shard = ShardMeshes[Index];
		if (!Shard)
		{
			continue;
		}

		Shard->AddWorldOffset(ShardVelocities[Index] * DeltaTime, false);
		const float ShardScale = FMath::Lerp(BurstSize * 0.22f, BurstSize * 0.05f, Alpha);
		Shard->SetRelativeScale3D(FVector(ShardScale));
	}
}
