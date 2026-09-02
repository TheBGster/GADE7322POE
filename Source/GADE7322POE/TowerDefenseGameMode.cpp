// Copyright Epic Games, Inc. All Rights Reserved.

#include "TowerDefenseGameMode.h"
#include "TowerDefenseGameState.h"
#include "TowerDefensePlayerController.h"
#include "ProceduralTerrainGenerator.h"
#include "CentralTower.h"
#include "DefenderBase.h"
#include "DefenderPlacementPoint.h"
#include "EnemyBase.h"
#include "EnemySpawner.h"
#include "HealthTestActor.h"
#include "GADE7322POE.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"

ATowerDefenseGameMode::ATowerDefenseGameMode()
{
	GameStateClass = ATowerDefenseGameState::StaticClass();
	PlayerControllerClass = ATowerDefensePlayerController::StaticClass();
	DefaultPawnClass = ADefaultPawn::StaticClass();
	TerrainOrigin = FVector(0.0f, 0.0f, 50.0f);
	bMovePlayerAboveTerrain = true;
	bSpawnAttackTestTargets = false;
}

void ATowerDefenseGameMode::StartPlay()
{
	Super::StartPlay();
	StartNewGame();
}

void ATowerDefenseGameMode::StartNewGame()
{
	ATowerDefenseGameState* TDGameState = GetTowerDefenseGameState();
	if (!TDGameState)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("StartNewGame failed: GameState is not ATowerDefenseGameState."));
		return;
	}

	TDGameState->ResetForNewGame();
	TDGameState->SetMatchState(ETowerDefenseMatchState::InProgress);

	GenerateWorldTerrain();
	SpawnCentralTower();
	SpawnPlacementPoints();
	SpawnAttackTestTargets();
	SpawnEnemySpawner();
	MovePlayerToTerrainOverview();

	UE_LOG(LogTowerDefense, Log, TEXT("New Tower Defense game started."));
}

void ATowerDefenseGameMode::HandleTowerDestroyed()
{
	UE_LOG(LogTowerDefense, Log, TEXT("Central tower destroyed."));
	HandleGameOver();
}

void ATowerDefenseGameMode::HandleGameOver()
{
	ATowerDefenseGameState* TDGameState = GetTowerDefenseGameState();
	if (!TDGameState)
	{
		return;
	}

	if (TDGameState->GetMatchState() == ETowerDefenseMatchState::GameOver)
	{
		return;
	}

	TDGameState->SetMatchState(ETowerDefenseMatchState::GameOver);

	if (IsValid(EnemySpawner))
	{
		EnemySpawner->StopSpawning();
	}

	if (IsValid(CentralTower))
	{
		CentralTower->StopCombat();
	}

	StopActiveEnemies();
	StopActiveDefenders();

	UE_LOG(LogTowerDefense, Log, TEXT("Game Over."));
}

void ATowerDefenseGameMode::RestartCurrentGame()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(World, true);
	UE_LOG(LogTowerDefense, Log, TEXT("Restarting level '%s'."), *CurrentLevelName);
	UGameplayStatics::OpenLevel(World, FName(*CurrentLevelName));
}

void ATowerDefenseGameMode::TDRestart()
{
	RestartCurrentGame();
}

void ATowerDefenseGameMode::TDRegen()
{
	DestroyGameplayActors();
	GenerateWorldTerrain();
	SpawnCentralTower();
	SpawnPlacementPoints();
	SpawnAttackTestTargets();
	SpawnEnemySpawner();
	MovePlayerToTerrainOverview();
}

void ATowerDefenseGameMode::GenerateWorldTerrain()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (!IsValid(TerrainGenerator))
	{
		TerrainGenerator = Cast<AProceduralTerrainGenerator>(
			UGameplayStatics::GetActorOfClass(World, AProceduralTerrainGenerator::StaticClass()));
	}

	if (!IsValid(TerrainGenerator))
	{
		UClass* ClassToSpawn = TerrainGeneratorClass.Get();
		if (!ClassToSpawn)
		{
			ClassToSpawn = AProceduralTerrainGenerator::StaticClass();
		}

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		TerrainGenerator = World->SpawnActor<AProceduralTerrainGenerator>(ClassToSpawn, TerrainOrigin, FRotator::ZeroRotator, SpawnParams);
	}

	if (!IsValid(TerrainGenerator))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to create the procedural terrain generator."));
		return;
	}

	TerrainGenerator->GenerateTerrain();
}

void ATowerDefenseGameMode::SpawnCentralTower()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(TerrainGenerator))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Cannot spawn the central tower because terrain is missing."));
		return;
	}

	if (IsValid(CentralTower))
	{
		CentralTower->Destroy();
		CentralTower = nullptr;
	}

	const FIntPoint Center = TerrainGenerator->GetCenterGridPosition();
	const FVector SpawnLocation = TerrainGenerator->GetTileSurfaceLocation(Center.X, Center.Y);

	UClass* ClassToSpawn = CentralTowerClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = ACentralTower::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	CentralTower = World->SpawnActor<ACentralTower>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

	if (!IsValid(CentralTower))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to spawn the central tower."));
		return;
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Spawned central tower at %s."), *SpawnLocation.ToCompactString());
}

void ATowerDefenseGameMode::SpawnEnemySpawner()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(TerrainGenerator))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Cannot start the enemy spawner because terrain is missing."));
		return;
	}

	if (IsValid(EnemySpawner))
	{
		EnemySpawner->StopSpawning();
		EnemySpawner->Destroy();
		EnemySpawner = nullptr;
	}

	UClass* ClassToSpawn = EnemySpawnerClass.Get();
	if (!ClassToSpawn)
	{
		ClassToSpawn = AEnemySpawner::StaticClass();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	EnemySpawner = World->SpawnActor<AEnemySpawner>(ClassToSpawn, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

	if (!IsValid(EnemySpawner))
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Failed to spawn the enemy spawner."));
		return;
	}

	EnemySpawner->SetGeneratedPaths(TerrainGenerator->GetGeneratedPaths());
	EnemySpawner->StartSpawning();
}

void ATowerDefenseGameMode::SpawnPlacementPoints()
{
	UWorld* World = GetWorld();
	if (!World || !IsValid(TerrainGenerator))
	{
		return;
	}

	TArray<AActor*> ExistingPoints;
	UGameplayStatics::GetAllActorsOfClass(this, ADefenderPlacementPoint::StaticClass(), ExistingPoints);
	for (AActor* Point : ExistingPoints)
	{
		if (IsValid(Point))
		{
			Point->Destroy();
		}
	}

	const TArray<FVector>& PlacementLocations = TerrainGenerator->GetDefenderPlacementLocations();
	for (const FVector& Location : PlacementLocations)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (ADefenderPlacementPoint* PlacementPoint = World->SpawnActor<ADefenderPlacementPoint>(Location, FRotator::ZeroRotator, SpawnParams))
		{
			PlacementPoint->InitializePlacement(Location);
		}
	}

	UE_LOG(LogTowerDefense, Log, TEXT("Spawned %d defender placement points."), PlacementLocations.Num());
}

void ATowerDefenseGameMode::SpawnAttackTestTargets()
{
	if (!bSpawnAttackTestTargets)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || !IsValid(TerrainGenerator))
	{
		return;
	}

	const TArray<FGeneratedPath>& Paths = TerrainGenerator->GetGeneratedPaths();
	int32 SpawnedCount = 0;

	for (const FGeneratedPath& Path : Paths)
	{
		if (SpawnedCount >= 2)
		{
			break;
		}

		if (Path.Waypoints.Num() < 5)
		{
			continue;
		}

		const FVector SpawnLocation = Path.Waypoints[Path.Waypoints.Num() - 4] + FVector(0.0f, 0.0f, 50.0f);
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AHealthTestActor* TestTarget = World->SpawnActor<AHealthTestActor>(SpawnLocation, FRotator::ZeroRotator, SpawnParams))
		{
			++SpawnedCount;
			UE_LOG(LogTowerDefense, Log, TEXT("Spawned attack test target near the tower at %s."), *SpawnLocation.ToCompactString());
		}
	}
}

void ATowerDefenseGameMode::DestroyGameplayActors()
{
	StopActiveEnemies();
	StopActiveDefenders();

	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemyBase::StaticClass(), Enemies);
	for (AActor* Enemy : Enemies)
	{
		if (IsValid(Enemy))
		{
			Enemy->Destroy();
		}
	}

	TArray<AActor*> Defenders;
	UGameplayStatics::GetAllActorsOfClass(this, ADefenderBase::StaticClass(), Defenders);
	for (AActor* Defender : Defenders)
	{
		if (IsValid(Defender))
		{
			Defender->Destroy();
		}
	}

	TArray<AActor*> PlacementPoints;
	UGameplayStatics::GetAllActorsOfClass(this, ADefenderPlacementPoint::StaticClass(), PlacementPoints);
	for (AActor* Point : PlacementPoints)
	{
		if (IsValid(Point))
		{
			Point->Destroy();
		}
	}

	TArray<AActor*> TestTargets;
	UGameplayStatics::GetAllActorsOfClass(this, AHealthTestActor::StaticClass(), TestTargets);
	for (AActor* TestTarget : TestTargets)
	{
		if (IsValid(TestTarget))
		{
			TestTarget->Destroy();
		}
	}

	if (IsValid(EnemySpawner))
	{
		EnemySpawner->StopSpawning();
		EnemySpawner->Destroy();
		EnemySpawner = nullptr;
	}

	if (IsValid(CentralTower))
	{
		CentralTower->Destroy();
		CentralTower = nullptr;
	}
}

void ATowerDefenseGameMode::StopActiveEnemies()
{
	TArray<AActor*> Enemies;
	UGameplayStatics::GetAllActorsOfClass(this, AEnemyBase::StaticClass(), Enemies);
	for (AActor* Actor : Enemies)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
		{
			Enemy->StopBehavior();
		}
	}
}

void ATowerDefenseGameMode::StopActiveDefenders()
{
	TArray<AActor*> Defenders;
	UGameplayStatics::GetAllActorsOfClass(this, ADefenderBase::StaticClass(), Defenders);
	for (AActor* Actor : Defenders)
	{
		if (ADefenderBase* Defender = Cast<ADefenderBase>(Actor))
		{
			Defender->StopCombat();
		}
	}
}

void ATowerDefenseGameMode::MovePlayerToTerrainOverview()
{
	if (!bMovePlayerAboveTerrain || !IsValid(TerrainGenerator))
	{
		return;
	}

	APlayerController* PlayerController = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PlayerController)
	{
		return;
	}

	APawn* Pawn = PlayerController->GetPawn();
	if (!Pawn)
	{
		return;
	}

	const FVector Center = TerrainGenerator->GetGridCenterWorldLocation();
	const float OverviewHeight = FMath::Max(TerrainGenerator->GetGridWidth(), TerrainGenerator->GetGridHeight()) * TerrainGenerator->GetTileSize() * 0.85f;
	Pawn->SetActorLocation(Center + FVector(0.0f, 0.0f, OverviewHeight));
	PlayerController->SetControlRotation(FRotator(-80.0f, 0.0f, 0.0f));

	UE_LOG(LogTowerDefense, Log, TEXT("Moved player above terrain center %s."), *Center.ToCompactString());
}

ATowerDefenseGameState* ATowerDefenseGameMode::GetTowerDefenseGameState() const
{
	return GetGameState<ATowerDefenseGameState>();
}
