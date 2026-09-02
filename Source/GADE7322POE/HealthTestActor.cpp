// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthTestActor.h"
#include "GADE7322POE.h"
#include "HealthComponent.h"
#include "TowerDefenseTypes.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

AHealthTestActor::AHealthTestActor()
{
	PrimaryActorTick.bCanEverTick = false;

	Tags.Add(TowerDefenseTags::Enemy);

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
		MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
		MeshComponent->SetGenerateOverlapEvents(true);
	}

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
}

void AHealthTestActor::BeginPlay()
{
	Super::BeginPlay();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &ThisClass::HandleDeath);
	}
}

void AHealthTestActor::HandleDeath(AActor* DeadActor)
{
	UE_LOG(LogTowerDefense, Log, TEXT("Health test actor '%s' reached 0 health and will be destroyed."), *GetName());
	Destroy();
}
