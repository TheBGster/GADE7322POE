// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefenderBase.h"
#include "VineTrap.generated.h"

class UStaticMeshComponent;

/**
 * Crowd-control defender. Pulses a non-stacking slow onto enemies in range.
 */
UCLASS()
class GADE7322POE_API AVineTrap : public ADefenderBase
{
	GENERATED_BODY()

public:
	AVineTrap();

protected:
	virtual void BeginPlay() override;
	virtual void AttackTarget() override;

	void PulseVines();
	void ResetPulseVisual();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<UStaticMeshComponent> VineRingMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|CrowdControl", meta = (ClampMin = "0.0", ClampMax = "0.85"))
	float SlowPercent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|CrowdControl", meta = (ClampMin = "0.1"))
	float SlowDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|CrowdControl", meta = (ClampMin = "0.0"))
	float PulseDamage;

	FTimerHandle PulseVisualTimerHandle;
};
