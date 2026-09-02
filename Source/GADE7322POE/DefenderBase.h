// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DefenderBase.generated.h"

class UHealthComponent;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class ADefenderPlacementPoint;

/**
 * Placeable defender that automatically attacks enemies in range.
 * Combat uses a timer, not Tick.
 */
UCLASS()
class GADE7322POE_API ADefenderBase : public AActor
{
	GENERATED_BODY()

public:
	ADefenderBase();

	UFUNCTION(BlueprintCallable, Category = "Defender")
	void SetOwningPlacementPoint(ADefenderPlacementPoint* PlacementPoint);

	UFUNCTION(BlueprintCallable, Category = "Defender")
	void StopCombat();

	UFUNCTION(BlueprintPure, Category = "Defender")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	void FindTarget();

	UFUNCTION()
	void AttackTarget();

	UFUNCTION()
	void Die(AActor* DeadActor);

	UFUNCTION()
	void HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsValidTarget(AActor* Actor) const;
	void StartAttackTimer();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Health", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Combat", meta = (ClampMin = "50.0"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender|Combat")
	TWeakObjectPtr<AActor> CurrentTarget;

	UPROPERTY()
	TWeakObjectPtr<ADefenderPlacementPoint> OwningPlacementPoint;

	FTimerHandle AttackTimerHandle;
	TArray<TWeakObjectPtr<AActor>> TargetsInRange;
};
