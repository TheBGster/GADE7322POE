// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TowerDefenseTypes.h"
#include "EnemyBase.generated.h"

class UHealthComponent;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;
class UWidgetComponent;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EEnemyBehaviorState : uint8
{
	Moving UMETA(DisplayName = "Moving"),
	Attacking UMETA(DisplayName = "Attacking"),
	Dead UMETA(DisplayName = "Dead")
};


UCLASS()
class GADE7322POE_API AEnemyBase : public AActor
{
	GENERATED_BODY()

public:
	AEnemyBase();

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void SetPath(const FGeneratedPath& Path);

	UFUNCTION(BlueprintCallable, Category = "Enemy")
	void StopBehavior();

	UFUNCTION(BlueprintPure, Category = "Enemy")
	bool IsDead() const;

	UFUNCTION(BlueprintPure, Category = "Enemy")
	int32 GetAssignedPathID() const { return AssignedPath.PathID; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	EEnemyKind GetEnemyKind() const { return EnemyKind; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	FText GetDisplayName() const { return DisplayName; }

	UFUNCTION(BlueprintPure, Category = "Enemy")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void ApplyMovementSlow(int32 SourceId, float SlowPercent, float Duration);

	UFUNCTION(BlueprintCallable, Category = "Enemy|Movement")
	void ClearMovementSlow(int32 SourceId);

	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	bool IsSlowed() const;

	UFUNCTION(BlueprintPure, Category = "Enemy|Movement")
	float GetEffectiveMoveSpeed() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

	void MoveAlongPath(float DeltaTime);
	void AdvanceToNextWaypoint();

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
	virtual void HandleDamaged(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy);

	bool IsValidAttackTarget(AActor* Actor) const;
	bool IsCombatAllowed() const;
	virtual bool ShouldKeepMovingWhileAttacking() const { return false; }
	virtual bool ShouldAttackDefendersWhileMoving() const { return true; }
	AActor* GetCentralTowerActor() const;
	FVector GetWaypointWorldLocation(int32 WaypointIndex) const;
	void StartAttackTimer();
	void GrantKillReward();
	void ApplyBodyColor();
	void SetupHealthBar();
	void PlayDamageFlash();
	void RestoreBodyColor();
	void RecalculateSlow();
	float GetMaxSlowPercent() const;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<USphereComponent> AttackRangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TObjectPtr<UHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|UI")
	TObjectPtr<UWidgetComponent> HealthBarComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyKind EnemyKind;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Visual")
	FLinearColor BodyColor;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Health", meta = (ClampMin = "1.0"))
	float MaxHealth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "1.0"))
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Movement", meta = (ClampMin = "1.0"))
	float WaypointAcceptanceRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.0"))
	float AttackDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "50.0"))
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Combat", meta = (ClampMin = "0.1"))
	float AttackCooldown;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Rewards", meta = (ClampMin = "0"))
	int32 ResourceReward;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	int32 CurrentWaypointIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	FGeneratedPath AssignedPath;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	TWeakObjectPtr<AActor> TargetActor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy")
	EEnemyBehaviorState BehaviorState;

	FTimerHandle AttackTimerHandle;
	FTimerHandle DamageFlashTimerHandle;
	TArray<TWeakObjectPtr<AActor>> TargetsInRange;
	TMap<int32, float> SlowSources;
	TMap<int32, FTimerHandle> SlowTimers;
	TObjectPtr<UMaterialInstanceDynamic> BodyMaterialInstance;
	float SlowMultiplier;
	float SpeedBurstMultiplier;
	bool bHasReachedDestination;
	bool bHasGrantedKillReward;
};
