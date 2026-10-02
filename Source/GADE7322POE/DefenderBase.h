// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "DefenderBase.generated.h"

class UHealthComponent;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class UWidgetComponent;
class UMaterialInstanceDynamic;
class ADefenderPlacementPoint;


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

	UFUNCTION(BlueprintPure, Category = "Defender")
	EDefenderKind GetDefenderKind() const { return DefenderKind; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	int32 GetPlacementCost() const { return PlacementCost; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	float GetAttackDamage() const { return AttackDamage; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	float GetAttackCooldown() const { return AttackCooldown; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	FLinearColor GetBodyColor() const { return BodyColor; }

	UFUNCTION(BlueprintPure, Category = "Defender")
	UStaticMeshComponent* GetMeshComponent() const { return MeshComponent; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION()
	virtual void FindTarget();

	UFUNCTION()
	virtual void AttackTarget();

	UFUNCTION()
	void Die(AActor* DeadActor);

	UFUNCTION()
	void HandleAttackRangeOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void HandleAttackRangeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy);

	bool IsValidTarget(AActor* Actor) const;
	bool IsCombatAllowed() const;
	void StartAttackTimer();
	void ApplyBodyColor();
	void SetupHealthBar();
	void PlayDamageFlash();
	void RestoreBodyColor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Defender|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender")
	EDefenderKind DefenderKind;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Economy", meta = (ClampMin = "0"))
	int32 PlacementCost;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Defender|Visual")
	FLinearColor BodyColor;

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
	FTimerHandle DamageFlashTimerHandle;
	TArray<TWeakObjectPtr<AActor>> TargetsInRange;
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterialInstance;
};
