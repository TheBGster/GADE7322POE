// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HealthTestActor.generated.h"

class UHealthComponent;
class UStaticMeshComponent;

/**
 * Temporary Stage 2 test actor. Place it in the level, then left-click it to apply damage.
 * This class is only for verifying the health component. Later stages replace it with the tower, enemies, and defenders.
 */
UCLASS()
class GADE7322POE_API AHealthTestActor : public AActor
{
	GENERATED_BODY()

public:
	AHealthTestActor();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleDeath(AActor* DeadActor);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> MeshComponent;
};
