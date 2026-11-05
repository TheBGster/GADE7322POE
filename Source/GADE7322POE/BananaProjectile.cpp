// Copyright Epic Games, Inc. All Rights Reserved.

#include "BananaProjectile.h"
#include "GADE7322POE.h"
#include "GameFramework/DamageType.h"
#include "JungleVisuals.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABananaProjectile::ABananaProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	ProjectileSpeed = 1400.0f;
	ImpactRadius = 40.0f;
	BananaColor = FLinearColor(0.95f, 0.82f, 0.12f, 1.0f);
	DamageAmount = 0.0f;
	Lifetime = 3.0f;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(SceneRoot);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComponent->SetGenerateOverlapEvents(false);
	MeshComponent->SetCastShadow(false);
	MeshComponent->SetRelativeScale3D(FVector(1.0f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> ImportedBanana(TEXT("/Game/Meshes/SM_Banana.SM_Banana"));
	if (ImportedBanana.Succeeded())
	{
		BananaMesh = ImportedBanana.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere"));
	if (BananaMesh)
	{
		MeshComponent->SetStaticMesh(BananaMesh);
	}
	else if (SphereMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(SphereMesh.Object);
		MeshComponent->SetRelativeScale3D(FVector(0.28f, 0.18f, 0.18f));
	}
}

void ABananaProjectile::BeginPlay()
{
	Super::BeginPlay();
	ApplyBananaMesh();
	NormalizeBananaScale(1.0f);
	ApplyColor();
}

void ABananaProjectile::InitialiseProjectile(AActor* InTarget, AActor* InInstigator, float InDamage, float InSpeed, float InScale)
{
	TargetActor = InTarget;
	DamageInstigator = InInstigator;
	DamageAmount = InDamage;
	if (InSpeed > 0.0f)
	{
		ProjectileSpeed = InSpeed;
	}

	NormalizeBananaScale(InScale > 0.0f ? InScale : 1.0f);
}

void ABananaProjectile::ApplyBananaMesh()
{
	if (!MeshComponent)
	{
		return;
	}

	if (!BananaMesh)
	{
		BananaMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Meshes/SM_Banana.SM_Banana"));
	}

	if (!BananaMesh)
	{
		BananaMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Imports/Banana.Banana"));
	}

	if (BananaMesh && MeshComponent->GetStaticMesh() != BananaMesh)
	{
		MeshComponent->SetStaticMesh(BananaMesh);
	}
}

void ABananaProjectile::NormalizeBananaScale(float ExtraScale)
{
	if (!MeshComponent)
	{
		return;
	}

	const UStaticMesh* Mesh = MeshComponent->GetStaticMesh();
	if (!Mesh)
	{
		return;
	}

	const float Radius = FMath::Max(Mesh->GetBounds().SphereRadius, 1.0f);
	const float TargetRadius = 32.0f * FMath::Max(ExtraScale, 0.1f);
	const float Uniform = TargetRadius / Radius;
	MeshComponent->SetRelativeScale3D(FVector(Uniform));
}

void ABananaProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Lifetime -= DeltaTime;
	if (Lifetime <= 0.0f)
	{
		Destroy();
		return;
	}

	AActor* Target = TargetActor.Get();
	if (!IsValid(Target))
	{
		Destroy();
		return;
	}

	const FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	if (ToTarget.Size() <= ImpactRadius)
	{
		Detonate();
		return;
	}

	AddActorWorldOffset(ToTarget.GetSafeNormal() * ProjectileSpeed * DeltaTime, false);
	SetActorRotation(ToTarget.Rotation());
}

void ABananaProjectile::ApplyColor()
{
	if (!MeshComponent)
	{
		return;
	}

	if (BananaMesh)
	{
		return;
	}

	UMaterialInterface* SourceMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (!SourceMaterial)
	{
		return;
	}

	if (UMaterialInstanceDynamic* Material = UMaterialInstanceDynamic::Create(SourceMaterial, this))
	{
		Material->SetVectorParameterValue(TEXT("Color"), BananaColor);
		Material->SetVectorParameterValue(TEXT("BaseColor"), BananaColor);
		MeshComponent->SetMaterial(0, Material);
	}
}

void ABananaProjectile::Detonate()
{
	JungleVisuals::SpawnBananaBurst(GetWorld(), GetActorLocation());

	if (AActor* Target = TargetActor.Get())
	{
		UGameplayStatics::ApplyDamage(Target, DamageAmount, nullptr, DamageInstigator.Get(), UDamageType::StaticClass());
		UE_LOG(LogTowerDefense, Log, TEXT("Banana projectile hit '%s' for %.0f."), *GetNameSafe(Target), DamageAmount);
	}

	Destroy();
}
