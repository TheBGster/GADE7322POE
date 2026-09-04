// Copyright Epic Games, Inc. All Rights Reserved.

#include "DefenderPlacementPoint.h"
#include "GADE7322POE.h"
#include "DefenderBase.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ADefenderPlacementPoint::ADefenderPlacementPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	bIsOccupied = false;
	PlacementLocation = FVector::ZeroVector;
	DefenderClass = ADefenderBase::StaticClass();

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
	MeshComponent->SetGenerateOverlapEvents(true);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(TEXT("/Engine/BasicShapes/Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CylinderMesh.Object);
	}

	MeshComponent->SetRelativeScale3D(FVector(1.4f, 1.4f, 0.12f));
	MeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 8.0f));
}

void ADefenderPlacementPoint::InitializePlacement(const FVector& InLocation)
{
	PlacementLocation = InLocation;
	SetActorLocation(InLocation);
	SetOccupied(false);
}

bool ADefenderPlacementPoint::CanPlaceDefender() const
{
	return DefenderClass && !OccupyingDefender.IsValid();
}

bool ADefenderPlacementPoint::PlaceDefender()
{
	if (!CanPlaceDefender())
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Cannot place a defender on '%s'."), *GetName());
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	UClass* ClassToSpawn = DefenderClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = ADefenderBase::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ADefenderBase* Defender = World->SpawnActor<ADefenderBase>(ClassToSpawn, PlacementLocation, FRotator::ZeroRotator, SpawnParams);
	if (!IsValid(Defender))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to spawn a defender at %s."), *PlacementLocation.ToCompactString());
		return false;
	}

	Defender->SetOwningPlacementPoint(this);
	OccupyingDefender = Defender;
	SetOccupied(true);

	UE_LOG(LogTowerDefense, Log, TEXT("Placed defender '%s' at %s."), *Defender->GetName(), *PlacementLocation.ToCompactString());
	return true;
}

void ADefenderPlacementPoint::SetOccupied(bool bOccupied)
{
	bIsOccupied = bOccupied;
	if (!bOccupied)
	{
		OccupyingDefender = nullptr;
	}

	UpdateVisualState();
}

void ADefenderPlacementPoint::NotifyDefenderDestroyed()
{
	UE_LOG(LogTowerDefense, Log, TEXT("Placement point '%s' is available again."), *GetName());
	SetOccupied(false);
}

void ADefenderPlacementPoint::UpdateVisualState()
{
	if (MeshComponent)
	{
		MeshComponent->SetVisibility(!bIsOccupied);
		MeshComponent->SetCollisionEnabled(bIsOccupied ? ECollisionEnabled::NoCollision : ECollisionEnabled::QueryAndPhysics);
	}
}
