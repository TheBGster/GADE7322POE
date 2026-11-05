// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "JungleBurstEffect.generated.h"

class UStaticMeshComponent;
class USceneComponent;


UCLASS()
class GADE7322POE_API AJungleBurstEffect : public AActor
{
	GENERATED_BODY()

public:
	AJungleBurstEffect();

	void InitialiseBurst(const FLinearColor& Color, float Size, int32 ShardCount);

protected:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> CoreMesh;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> ShardMeshes;

	TArray<FVector> ShardVelocities;
	FLinearColor BurstColor;
	float BurstSize;
	float Elapsed;
	float Lifetime;
};
