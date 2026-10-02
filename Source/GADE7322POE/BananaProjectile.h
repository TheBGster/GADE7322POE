// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BananaProjectile.generated.h"

class UStaticMeshComponent;
class USceneComponent;


class GADE7322POE_API ABananaProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABananaProjectile();

	void InitialiseProjectile(AActor* InTarget, AActor* InInstigator, float InDamage, float InSpeed);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void ApplyColor();
	void Detonate();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "100.0"))
	float ProjectileSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile", meta = (ClampMin = "0.0"))
	float ImpactRadius;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	FLinearColor BananaColor;

	TWeakObjectPtr<AActor> TargetActor;
	TWeakObjectPtr<AActor> DamageInstigator;
	float DamageAmount;
	float Lifetime;
};
