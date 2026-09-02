// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnemySpawner.h"
#include "GADE7322POE.h"
#include "EnemyBase.h"
#include "EngineUtils.h"
#include "TimerManager.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnInterval = 3.0f;
	MaxActiveEnemies = 10;
	bIsSpawning = false;
	NextPathIndex = 0;
	EnemyClass = AEnemyBase::StaticClass();
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
	if (!World)
	{
		return;
	}

	bIsSpawning = true;
	World->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ThisClass::SpawnEnemy, SpawnInterval, true);
	SpawnEnemy();

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy spawner started. Interval: %.1fs"), SpawnInterval);
}

void AEnemySpawner::StopSpawning()
{
	bIsSpawning = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Enemy spawner stopped."));
}

void AEnemySpawner::SpawnEnemy()
{
	if (!bIsSpawning)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (CountActiveEnemies() >= MaxActiveEnemies)
	{
		UE_LOG(LogTowerDefense, Verbose, TEXT("Enemy spawn skipped: max active enemies reached (%d)."), MaxActiveEnemies);
		return;
	}

	UClass* ClassToSpawn = EnemyClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = AEnemyBase::StaticClass();
	}

	const FGeneratedPath* Path = SelectNextPath();
	if (!Path || Path->Waypoints.Num() == 0)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Enemy spawn skipped: selected path is invalid."));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(ClassToSpawn, Path->SpawnLocation, FRotator::ZeroRotator, SpawnParams);
	if (!Enemy)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to spawn enemy on path %d."), Path->PathID);
		return;
	}

	Enemy->SetPath(*Path);
	UE_LOG(LogTowerDefense, Log, TEXT("Spawned enemy '%s' on path %d at %s."),
		*Enemy->GetName(), Path->PathID, *Path->SpawnLocation.ToCompactString());
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

const FGeneratedPath* AEnemySpawner::SelectNextPath() const
{
	if (GeneratedPaths.Num() == 0)
	{
		return nullptr;
	}

	const FGeneratedPath* Path = &GeneratedPaths[NextPathIndex % GeneratedPaths.Num()];
	++NextPathIndex;
	return Path;
}
