// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefenderBase.h"
#include "BananaCannon.generated.h"

class ABananaProjectile;


UCLASS()
class GADE7322POE_API ABananaCannon : public ADefenderBase
{
	GENERATED_BODY()

public:
	ABananaCannon();

protected:
	virtual void FindTarget() override;
	virtual void AttackTarget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Combat")
	TSubclassOf<ABananaProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Combat", meta = (ClampMin = "100.0"))
	float ProjectileSpeed;

	void ResetFireVisual();

	FTimerHandle FireVisualTimerHandle;
	FVector RestingMeshScale;
};
