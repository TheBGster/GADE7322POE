// Copyright Epic Games, Inc. All Rights Reserved.

#include "HealthComponent.h"
#include "GADE7322POE.h"
#include "GameFramework/Actor.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}

	InitializeHealth();
}

void UHealthComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.RemoveDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}

	Super::EndPlay(EndPlayReason);
}

void UHealthComponent::InitializeHealth(float InMaxHealth)
{
	if (InMaxHealth > 0.0f)
	{
		MaxHealth = InMaxHealth;
	}

	MaxHealth = FMath::Max(MaxHealth, 1.0f);
	CurrentHealth = MaxHealth;
	bIsDead = false;

	BroadcastHealthChanged();

	UE_LOG(LogTowerDefense, Log, TEXT("%s health initialized to %.0f / %.0f"),
		*GetNameSafe(GetOwner()), CurrentHealth, MaxHealth);
}

float UHealthComponent::TakeDamage(float DamageAmount, AActor* DamageCauser, AController* InstigatedBy)
{
	if (bIsDead || DamageAmount <= 0.0f)
	{
		return 0.0f;
	}

	AActor* Owner = GetOwner();
	if (Owner && !Owner->CanBeDamaged())
	{
		return 0.0f;
	}

	const float AppliedDamage = FMath::Min(DamageAmount, CurrentHealth);
	CurrentHealth = FMath::Max(CurrentHealth - DamageAmount, 0.0f);

	OnDamaged.Broadcast(AppliedDamage, DamageCauser, InstigatedBy);
	BroadcastHealthChanged();

	UE_LOG(LogTowerDefense, Log, TEXT("%s took %.0f damage. Health: %.0f / %.0f"),
		*GetNameSafe(Owner), AppliedDamage, CurrentHealth, MaxHealth);

	if (CurrentHealth <= 0.0f)
	{
		HandleDeath();
	}

	return AppliedDamage;
}

float UHealthComponent::Heal(float HealAmount)
{
	if (bIsDead || HealAmount <= 0.0f)
	{
		return 0.0f;
	}

	const float PreviousHealth = CurrentHealth;
	CurrentHealth = FMath::Min(CurrentHealth + HealAmount, MaxHealth);
	const float ActualHeal = CurrentHealth - PreviousHealth;

	if (ActualHeal > 0.0f)
	{
		BroadcastHealthChanged();
		UE_LOG(LogTowerDefense, Log, TEXT("%s healed %.0f. Health: %.0f / %.0f"),
			*GetNameSafe(GetOwner()), ActualHeal, CurrentHealth, MaxHealth);
	}

	return ActualHeal;
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	TakeDamage(Damage, DamageCauser, InstigatedBy);
}

void UHealthComponent::BroadcastHealthChanged()
{
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::HandleDeath()
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;
	CurrentHealth = 0.0f;

	AActor* Owner = GetOwner();
	UE_LOG(LogTowerDefense, Log, TEXT("%s died."), *GetNameSafe(Owner));

	OnDeath.Broadcast(Owner);
}
