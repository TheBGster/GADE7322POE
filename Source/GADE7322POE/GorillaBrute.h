// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "GorillaBrute.generated.h"


UCLASS()
class GADE7322POE_API AGorillaBrute : public AEnemyBase
{
	GENERATED_BODY()

public:
	AGorillaBrute();

protected:
	virtual bool ShouldKeepMovingWhileAttacking() const override { return true; }
	virtual void AttackTarget() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "1.0"))
	float DefenderSmashMultiplier;
};
