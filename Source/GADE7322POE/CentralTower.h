// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CentralTower.generated.h"

class UHealthComponent;
class UMaterialInstanceDynamic;
class UMaterialInterface;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * Central structure the player must defend.
 * Uses UHealthComponent for damage/death and a timer (not Tick) for automatic attacks.
 */
UCLASS()
class GADE7322POE_API ACentralTower : public AActor
{
	GENERATED_BODY()

public:
	ACentralTower();

	UFUNCTION(BlueprintPure, Category = "Tower")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "Tower")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

	UFUNCTION(BlueprintCallable, Category = "Tower")
	void StopCombat();

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
	void HandleHealthChanged(float CurrentHealth, float MaxHealth);

	UFUNCTION()
	void HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	bool IsValidTarget(AActor* Actor) const;
	bool IsTargetInRange(AActor* Actor) const;
	bool IsCombatAllowed() const;
	void StartAttackTimer();
	void SyncHealthToGameState() const;
	void ApplyTowerMaterial();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Health", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Combat", meta = (ClampMin = "50.0"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Combat")
	bool bDrawAttackRange;

	/** Optional parent material. If empty, Engine BasicShapeMaterial is used. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Materials")
	TObjectPtr<UMaterialInterface> BaseTowerMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tower|Materials")
	FLinearColor TowerColor;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Tower|Materials")
	TObjectPtr<UMaterialInstanceDynamic> TowerMaterialInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower|Combat")
	TWeakObjectPtr<AActor> CurrentTarget;

	FTimerHandle AttackTimerHandle;
	TArray<TWeakObjectPtr<AActor>> TargetsInRange;
};
