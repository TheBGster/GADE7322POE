// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DefenderBase.h"
#include "VineTrap.generated.h"

class UStaticMeshComponent;
class UMaterialInstanceDynamic;


UCLASS()
class GADE7322POE_API AVineTrap : public ADefenderBase
{
	GENERATED_BODY()

public:
	AVineTrap();

	float GetSlowPercent() const { return SlowPercent; }
	float GetPulseDamage() const { return PulseDamage; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void AttackTarget() override;
	virtual void ApplyKindSpecificUpgrade(int32 TierIndex) override;
	virtual const TCHAR* GetCharacterMeshPath() const override;
	void ApplyVineMaterials();

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
	FVector RestingRingScale;
	TObjectPtr<UMaterialInstanceDynamic> RingMaterialInstance;
};
