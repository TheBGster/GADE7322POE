// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "ChimpRaider.generated.h"


UCLASS()
class GADE7322POE_API AChimpRaider : public AEnemyBase
{
	GENERATED_BODY()

public:
	AChimpRaider();

protected:
	virtual bool ShouldKeepMovingWhileAttacking() const override { return true; }
	virtual bool ShouldAttackDefendersWhileMoving() const override { return false; }
	virtual void HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy) override;

	void ClearSpeedBurst();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "1.0"))
	float DamagedSpeedBurstMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "0.1"))
	float DamagedSpeedBurstDuration;

	FTimerHandle SpeedBurstTimerHandle;
};
