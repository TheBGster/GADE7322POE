// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawner.h"
#include "GADE7322POE.h"
#include "ChimpRaider.h"
#include "DefenderBase.h"
#include "EnemyBase.h"
#include "HealthComponent.h"
#include "EngineUtils.h"
#include "GorillaBrute.h"
#include "TimerManager.h"
#include "TowerDefenseGameState.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnInterval = 2.4f;
	FirstWaveDelay = 4.0f;
	BaseInterWaveDelay = 6.5f;
	MaxActiveEnemies = 12;
	bIsSpawning = false;
	NextPathIndex = 0;
	NextSpawnIndex = 0;
	CurrentWaveNumber = 0;
	bWaveSpawnComplete = false;
	EnemyClass = AEnemyBase::StaticClass();
	GorillaBruteClass = AGorillaBrute::StaticClass();
	ChimpRaiderClass = AChimpRaider::StaticClass();
	CachedScoutCount = 0;
	CachedCannonCount = 0;
	CachedTrapCount = 0;
	CachedTowerHealthPercent = 1.0f;
	CachedResources = 0;
	CachedEnemiesDefeated = 0;
	CachedDefenseDPS = 0.0f;
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void AEnemySpawner::SetGeneratedPaths(const TArray<FGeneratedPath>& InPaths)
{
	GeneratedPaths = InPaths;
	NextPathIndex = 0;

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy spawner received %d generated paths."), GeneratedPaths.Num());
}

void AEnemySpawner::StartSpawning()
{
	if (GeneratedPaths.Num() == 0)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Enemy spawner cannot start: no generated paths."));
		return;
	}

	UWorld* World = GetWorld();
	const ATowerDefenseGameState* GameState = World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
	if (!World || !GameState || !GameState->IsMatchInProgress())
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Enemy spawner cannot start: match is not in progress."));
		return;
	}

	bIsSpawning = true;
	CurrentWaveNumber = 0;
	NextSpawnIndex = 0;
	bWaveSpawnComplete = false;
	CurrentWaveQueue.Reset();
	WaveRandom.Initialize(FMath::Rand());

	World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	World->GetTimerManager().ClearTimer(WaveTimerHandle);
	ScheduleNextWave(FirstWaveDelay);

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy wave system started. First wave in %.1fs."), FirstWaveDelay);
}

void AEnemySpawner::StopSpawning()
{
	bIsSpawning = false;
	bWaveSpawnComplete = true;
	CurrentWaveQueue.Reset();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
		World->GetTimerManager().ClearTimer(WaveTimerHandle);
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy spawner stopped."));
}

void AEnemySpawner::StartNextWave()
{
	if (!CanSpawnNow())
	{
		return;
	}

	++CurrentWaveNumber;
	BuildCurrentWave();
	NextSpawnIndex = 0;
	bWaveSpawnComplete = false;

	if (ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		GameState->SetCurrentWave(CurrentWaveNumber, CurrentWaveQueue.Num());
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Wave %d: %d enemies (Scouts=%d Cannons=%d Traps=%d TowerHP=%.0f%% Resources=%d)."),
		CurrentWaveNumber,
		CurrentWaveQueue.Num(),
		CachedScoutCount,
		CachedCannonCount,
		CachedTrapCount,
		CachedTowerHealthPercent * 100.0f,
		CachedResources);

	ScheduleNextSpawn(0.15f);
}

void AEnemySpawner::SpawnNextEnemy()
{
	if (!CanSpawnNow())
	{
		return;
	}

	if (NextSpawnIndex >= CurrentWaveQueue.Num())
	{
		bWaveSpawnComplete = true;
		CheckWaveComplete();
		return;
	}

	if (CountActiveEnemies() >= MaxActiveEnemies)
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("Wave spawn delayed: max active enemies reached (%d)."), MaxActiveEnemies);
		ScheduleNextSpawn(0.75f);
		return;
	}

	const FWaveSpawnEntry Entry = CurrentWaveQueue[NextSpawnIndex++];
	SpawnEnemyFromEntry(Entry);

	const float NextDelay = Entry.DelayAfterSpawn > 0.0f
		? Entry.DelayAfterSpawn
		: GetSpawnDelayForKind(Entry.EnemyKind);
	ScheduleNextSpawn(NextDelay);
}

void AEnemySpawner::SpawnEnemyFromEntry(const FWaveSpawnEntry& Entry)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UClass* ClassToSpawn = GetClassForKind(Entry.EnemyKind).Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = AEnemyBase::StaticClass();
	}

	const FGeneratedPath* Path = SelectPathForEntry(Entry);
	if (!Path || Path->Waypoints.Num() == 0)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Enemy spawn skipped: selected path is invalid."));
		return;
	}

	const FVector SpawnLocation = Path->SpawnLocation.IsNearlyZero()
		? Path->Waypoints[0]
		: Path->SpawnLocation;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!IsValid(Enemy))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to spawn enemy on path %d."), Path->PathID);
		return;
	}

	Enemy->SetPath(*Path);
	UE_LOG(LogTowerDefense, Log, TEXT("Spawned %s on path %d at %s."),
		*Enemy->GetDisplayName().ToString(), Path->PathID, *SpawnLocation.ToCompactString());
}

void AEnemySpawner::BuildCurrentWave()
{
	CurrentWaveQueue.Reset();

	CachedScoutCount = 0;
	CachedCannonCount = 0;
	CachedTrapCount = 0;
	CachedDefenseDPS = 0.0f;
	CachedTowerHealthPercent = 1.0f;
	CachedResources = 0;
	CachedEnemiesDefeated = 0;

	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<ADefenderBase> It(World); It; ++It)
		{
			const ADefenderBase* Defender = *It;
			if (!IsValid(Defender) || !Defender->GetHealthComponent() || Defender->GetHealthComponent()->IsDead())
			{
				continue;
			}

			const float Cooldown = FMath::Max(Defender->GetAttackCooldown(), 0.1f);
			CachedDefenseDPS += Defender->GetAttackDamage() / Cooldown;

			switch (Defender->GetDefenderKind())
			{
			case EDefenderKind::BananaCannon:
				++CachedCannonCount;
				break;
			case EDefenderKind::VineTrap:
				++CachedTrapCount;
				break;
			default:
				++CachedScoutCount;
				break;
			}
		}
	}

	if (const ATowerDefenseGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ATowerDefenseGameState>() : nullptr)
	{
		const float MaxHealth = GameState->GetTowerMaxHealth();
		CachedTowerHealthPercent = MaxHealth > 0.0f ? GameState->GetTowerCurrentHealth() / MaxHealth : 1.0f;
		CachedResources = GameState->GetCurrentResources();
		CachedEnemiesDefeated = GameState->GetEnemiesDefeated();
	}

	int32 WaveSize = 4 + CurrentWaveNumber;
	if (CachedResources >= 80)
	{
		++WaveSize;
	}
	if (CachedEnemiesDefeated >= 15)
	{
		++WaveSize;
	}
	if (CachedTowerHealthPercent < 0.35f)
	{
		WaveSize = FMath::Max(WaveSize - 1, 4);
	}

	WaveSize = FMath::Clamp(WaveSize, 4, 14);

	int32 GorillasSpawned = 0;
	int32 ChimpsSpawned = 0;

	for (int32 Index = 0; Index < WaveSize; ++Index)
	{
		FWaveSpawnEntry Entry;
		if (CurrentWaveNumber <= 1)
		{
			Entry.EnemyKind = EEnemyKind::JungleScout;
		}
		else
		{
			Entry.EnemyKind = PickWeightedEnemyKind(GorillasSpawned, ChimpsSpawned);
		}

		if (Entry.EnemyKind == EEnemyKind::GorillaBrute)
		{
			++GorillasSpawned;
		}
		else if (Entry.EnemyKind == EEnemyKind::ChimpRaider)
		{
			++ChimpsSpawned;
		}

		if (GeneratedPaths.Num() > 0)
		{
			Entry.PreferredPathIndex = WaveRandom.RandRange(0, GeneratedPaths.Num() - 1);
		}

		Entry.DelayAfterSpawn = GetSpawnDelayForKind(Entry.EnemyKind);
		CurrentWaveQueue.Add(Entry);
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Wave %d composition built. Size=%d Gorillas=%d Chimps=%d DefenseDPS=%.1f"),
		CurrentWaveNumber, WaveSize, GorillasSpawned, ChimpsSpawned, CachedDefenseDPS);
}

EEnemyKind AEnemySpawner::PickWeightedEnemyKind(int32 GorillasSpawned, int32 ChimpsSpawned) const
{
	const int32 MaxGorillas = FMath::Clamp(1 + CurrentWaveNumber / 3, 1, 3);
	const int32 MaxChimps = FMath::Clamp(2 + CurrentWaveNumber / 2, 2, 6);

	float ScoutWeight = 1.0f;
	float GorillaWeight = 0.16f + CurrentWaveNumber * 0.035f;
	float ChimpWeight = 0.20f + CurrentWaveNumber * 0.045f;

	if (CachedCannonCount >= 2 || (CachedCannonCount > 0 && CachedCannonCount >= CachedScoutCount))
	{
		ChimpWeight += 0.55f;
	}

	if (CachedScoutCount >= 2 || CachedTrapCount >= 2)
	{
		GorillaWeight += 0.50f;
	}

	if (CachedTrapCount > CachedCannonCount && CachedTrapCount > 0)
	{
		GorillaWeight += 0.22f;
	}

	if (CachedDefenseDPS < 20.0f && CurrentWaveNumber >= 3)
	{
		ChimpWeight += 0.18f;
	}

	if (CachedTowerHealthPercent < 0.45f)
	{
		GorillaWeight *= 0.55f;
		ChimpWeight += 0.18f;
	}

	if (CachedResources >= 75)
	{
		GorillaWeight += 0.08f;
		ChimpWeight += 0.08f;
	}

	if (CachedEnemiesDefeated >= 12)
	{
		GorillaWeight += 0.06f;
		ChimpWeight += 0.06f;
	}

	if (GorillasSpawned >= MaxGorillas)
	{
		GorillaWeight = 0.0f;
	}

	if (ChimpsSpawned >= MaxChimps)
	{
		ChimpWeight = 0.0f;
	}

	const float Total = ScoutWeight + GorillaWeight + ChimpWeight;
	if (Total <= KINDA_SMALL_NUMBER)
	{
		return EEnemyKind::JungleScout;
	}

	const float Roll = WaveRandom.FRandRange(0.0f, Total);
	if (Roll < GorillaWeight)
	{
		return EEnemyKind::GorillaBrute;
	}

	if (Roll < GorillaWeight + ChimpWeight)
	{
		return EEnemyKind::ChimpRaider;
	}

	return EEnemyKind::JungleScout;
}

TSubclassOf<AEnemyBase> AEnemySpawner::GetClassForKind(EEnemyKind Kind) const
{
	switch (Kind)
	{
	case EEnemyKind::GorillaBrute:
		return GorillaBruteClass ? GorillaBruteClass : TSubclassOf<AEnemyBase>(AGorillaBrute::StaticClass());
	case EEnemyKind::ChimpRaider:
		return ChimpRaiderClass ? ChimpRaiderClass : TSubclassOf<AEnemyBase>(AChimpRaider::StaticClass());
	default:
		return EnemyClass ? EnemyClass : TSubclassOf<AEnemyBase>(AEnemyBase::StaticClass());
	}
}

void AEnemySpawner::ScheduleNextSpawn(float Delay)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::SpawnNextEnemy, FMath::Max(Delay, 0.1f), false);
}

void AEnemySpawner::ScheduleNextWave(float Delay)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	World->GetTimerManager().ClearTimer(WaveTimerHandle);
	World->GetTimerManager().SetTimer(WaveTimerHandle, this, &ThisClass::StartNextWave, FMath::Max(Delay, 0.5f), false);
}

void AEnemySpawner::CheckWaveComplete()
{
	if (!CanSpawnNow())
	{
		return;
	}

	if (CountActiveEnemies() > 0)
	{
		ScheduleNextSpawn(1.0f);
		return;
	}

	ScheduleNextWave(GetInterWaveDelay());
}

float AEnemySpawner::GetInterWaveDelay() const
{
	return FMath::Max(3.5f, BaseInterWaveDelay - CurrentWaveNumber * 0.35f);
}

float AEnemySpawner::GetSpawnDelayForKind(EEnemyKind Kind) const
{
	const float ScaledInterval = FMath::Max(0.7f, SpawnInterval - CurrentWaveNumber * 0.08f);
	switch (Kind)
	{
	case EEnemyKind::ChimpRaider:
		return FMath::Max(0.45f, ScaledInterval * 0.55f);
	case EEnemyKind::GorillaBrute:
		return ScaledInterval * 1.35f;
	default:
		return ScaledInterval;
	}
}

int32 AEnemySpawner::CountActiveEnemies() const
{
	int32 AliveCount = 0;
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AEnemyBase> It(World); It; ++It)
		{
			if (IsValid(*It) && !It->IsDead())
			{
				++AliveCount;
			}
		}
	}

	return AliveCount;
}

const FGeneratedPath* AEnemySpawner::SelectNextPath()
{
	if (GeneratedPaths.Num() == 0)
	{
		return nullptr;
	}

	for (int32 Attempt = 0; Attempt < GeneratedPaths.Num(); ++Attempt)
	{
		const int32 PathIndex = NextPathIndex % GeneratedPaths.Num();
		++NextPathIndex;

		if (GeneratedPaths[PathIndex].Waypoints.Num() > 0)
		{
			return &GeneratedPaths[PathIndex];
		}
	}

	return nullptr;
}

const FGeneratedPath* AEnemySpawner::SelectPathForEntry(const FWaveSpawnEntry& Entry)
{
	if (GeneratedPaths.IsValidIndex(Entry.PreferredPathIndex) && GeneratedPaths[Entry.PreferredPathIndex].Waypoints.Num() > 0)
	{
		return &GeneratedPaths[Entry.PreferredPathIndex];
	}

	return SelectNextPath();
}

bool AEnemySpawner::CanSpawnNow() const
{
	if (!bIsSpawning || GeneratedPaths.Num() == 0)
	{
		return false;
	}

	const UWorld* World = GetWorld();
	const ATowerDefenseGameState* GameState = World ? World->GetGameState<ATowerDefenseGameState>() : nullptr;
	return GameState && GameState->IsMatchInProgress();
}
