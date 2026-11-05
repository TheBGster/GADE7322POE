// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BananaProjectile.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class UStaticMesh;

UCLASS()
class GADE7322POE_API ABananaProjectile : public AActor
{
	GENERATED_BODY()

public:
	ABananaProjectile();

	void InitialiseProjectile(AActor* InTarget, AActor* InInstigator, float InDamage, float InSpeed, float InScale = 1.0f);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	void ApplyColor();
	void ApplyBananaMesh();
	void NormalizeBananaScale(float ExtraScale);
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMesh> BananaMesh;

	TWeakObjectPtr<AActor> TargetActor;
	TWeakObjectPtr<AActor> DamageInstigator;
	float DamageAmount;
	float Lifetime;
};
