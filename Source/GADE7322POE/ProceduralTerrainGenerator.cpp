// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralTerrainGenerator.h"
#include "GADE7322POE.h"
#include "Algo/Reverse.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Math/NumericLimits.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	struct FPathSearchNode
	{
		int32 G = TNumericLimits<int32>::Max();
		int32 H = 0;
		FIntPoint Parent = FIntPoint(-1, -1);
		bool bClosed = false;
		bool bInOpenSet = false;
	};
}

AProceduralTerrainGenerator::AProceduralTerrainGenerator()
{
	PrimaryActorTick.bCanEverTick = false;

	GridWidth = 21;
	GridHeight = 21;
	TileSize = 200.0f;
	BaseTileHeight = 40.0f;
	HeightStep = 40.0f;
	PathTileHeight = 18.0f;
	MaxHeightLevel = 2;
	bRandomizeSeedOnGenerate = true;
	RandomSeed = 0;
	NoiseScale = 0.12f;
	bFlattenCenter = true;
	CenterClearRadius = 1;
	NumberOfPaths = 3;
	MinPathStartSeparation = 6;
	SpawnHeightOffset = 50.0f;
	bDrawDebugPaths = true;
	MaxDefenderPlacementPoints = 16;
	PlacementMinSpacing = 2;
	NoiseOrigin = FVector2D::ZeroVector;
	GroundColor = FLinearColor(0.10f, 0.50f, 0.10f, 1.0f);
	PathColor = FLinearColor(0.35f, 0.15f, 0.05f, 1.0f);

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SceneRoot->SetMobility(EComponentMobility::Movable);
	SetRootComponent(SceneRoot);

	auto ConfigureCollision = [](UHierarchicalInstancedStaticMeshComponent* Instancer)
	{
		Instancer->SetMobility(EComponentMobility::Movable);
		Instancer->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Instancer->SetCollisionObjectType(ECC_WorldStatic);
		Instancer->SetCollisionResponseToAllChannels(ECR_Block);
		Instancer->SetGenerateOverlapEvents(false);
		Instancer->SetCastShadow(true);
	};

	GroundTileInstancer = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("GroundTileInstancer"));
	GroundTileInstancer->SetupAttachment(SceneRoot);
	ConfigureCollision(GroundTileInstancer);

	ElevatedTileInstancer = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("ElevatedTileInstancer"));
	ElevatedTileInstancer->SetupAttachment(SceneRoot);
	ConfigureCollision(ElevatedTileInstancer);

	PathTileInstancer = CreateDefaultSubobject<UHierarchicalInstancedStaticMeshComponent>(TEXT("PathTileInstancer"));
	PathTileInstancer->SetupAttachment(SceneRoot);
	ConfigureCollision(PathTileInstancer);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube"));
	if (CubeMesh.Succeeded())
	{
		GroundTileMesh = CubeMesh.Object;
		ElevatedTileMesh = CubeMesh.Object;
		PathTileMesh = CubeMesh.Object;
	}

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> ShapeMaterial(TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (ShapeMaterial.Succeeded())
	{
		BaseTileMaterial = ShapeMaterial.Object;
	}
}

void AProceduralTerrainGenerator::BeginPlay()
{
	Super::BeginPlay();
}

void AProceduralTerrainGenerator::GenerateTerrain()
{
	InitializeRandomStream();
	ClearPreviousTerrain();
	GenerateGrid();
	MarkTowerLocation();
	GeneratePaths();
	GenerateEnemySpawnLocations();
	GenerateDefenderPlacementLocations();
	BuildTileInstances();
	DrawPathDebug();

	UE_LOG(LogTowerDefense, Log, TEXT("Generated terrain %dx%d using seed %d. Paths: %d  Spawns: %d  Placement points: %d"),
		GridWidth, GridHeight, RandomSeed, GeneratedPaths.Num(), SpawnLocations.Num(), DefenderPlacementLocations.Num());
}

void AProceduralTerrainGenerator::ClearPreviousTerrain()
{
	if (GroundTileInstancer)
	{
		GroundTileInstancer->ClearInstances();
	}

	if (ElevatedTileInstancer)
	{
		ElevatedTileInstancer->ClearInstances();
	}

	if (PathTileInstancer)
	{
		PathTileInstancer->ClearInstances();
	}

	if (UWorld* World = GetWorld())
	{
		FlushPersistentDebugLines(World);
		FlushDebugStrings(World);
	}

	TerrainTiles.Reset();
	GeneratedPaths.Reset();
	SpawnLocations.Reset();
	DefenderPlacementLocations.Reset();
}

void AProceduralTerrainGenerator::GenerateGrid()
{
	TerrainTiles.SetNum(GridWidth * GridHeight);

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			FTerrainTile& Tile = TerrainTiles[GetTileIndex(X, Y)];
			Tile.X = X;
			Tile.Y = Y;
			Tile.HeightLevel = ChooseHeightLevel(X, Y);
			Tile.TileType = ChooseTileType(Tile.HeightLevel);
			Tile.WorldLocation = GridToWorldLocation(X, Y, Tile.HeightLevel);
		}
	}
}

void AProceduralTerrainGenerator::MarkTowerLocation()
{
	const FIntPoint Center = GetCenterGridPosition();
	if (FTerrainTile* Tile = GetTile(Center.X, Center.Y))
	{
		Tile->TileType = ETerrainTileType::Tower;
		Tile->HeightLevel = 0;
		Tile->WorldLocation = GridToWorldLocation(Center.X, Center.Y, 0);
	}
}

void AProceduralTerrainGenerator::GeneratePaths()
{
	GeneratedPaths.Reset();

	const FIntPoint Goal = GetCenterGridPosition();
	const TArray<FIntPoint> StartCells = SelectPathStartCells();

	for (int32 PathIndex = 0; PathIndex < StartCells.Num(); ++PathIndex)
	{
		TArray<FIntPoint> GridPoints;
		if (!FindPathAStar(StartCells[PathIndex], Goal, GridPoints))
		{
			UE_LOG(LogTowerDefense, Warning, TEXT("A* failed for path %d. Using greedy fallback."), PathIndex);
			FindPathGreedy(StartCells[PathIndex], Goal, GridPoints);
		}

		if (GridPoints.Num() < 2)
		{
			for (int32 Retry = 0; Retry < 8 && GridPoints.Num() < 2; ++Retry)
			{
				FindPathGreedy(PickCellOnEdge(Retry % 4), Goal, GridPoints);
			}
		}

		if (GridPoints.Num() < 2)
		{
			UE_LOG(LogTowerDefense, Error, TEXT("Path %d is invalid and was skipped."), PathIndex);
			continue;
		}

		ApplyPathTiles(GridPoints);
		GeneratedPaths.Add(BuildGeneratedPath(PathIndex, GridPoints));

		UE_LOG(LogTowerDefense, Log, TEXT("Path %d: start (%d, %d) -> tower (%d, %d) with %d waypoints."),
			PathIndex, StartCells[PathIndex].X, StartCells[PathIndex].Y, Goal.X, Goal.Y, GridPoints.Num());
	}

	if (GeneratedPaths.Num() < NumberOfPaths)
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Generated %d paths but %d were requested."), GeneratedPaths.Num(), NumberOfPaths);
	}
}

void AProceduralTerrainGenerator::GenerateEnemySpawnLocations()
{
	SpawnLocations.Reset();
	SpawnLocations.Reserve(GeneratedPaths.Num());

	for (const FGeneratedPath& Path : GeneratedPaths)
	{
		SpawnLocations.Add(Path.SpawnLocation);
	}
}

void AProceduralTerrainGenerator::GenerateDefenderPlacementLocations()
{
	DefenderPlacementLocations.Reset();

	TArray<FIntPoint> Candidates;
	for (const FTerrainTile& Tile : TerrainTiles)
	{
		if (IsValidDefenderPlacementTile(Tile))
		{
			Candidates.Add(Tile.GetGridPosition());
		}
	}

	for (int32 Index = Candidates.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		Candidates.Swap(Index, SwapIndex);
	}

	TArray<FIntPoint> ChosenPoints;
	for (const FIntPoint& Candidate : Candidates)
	{
		if (ChosenPoints.Num() >= MaxDefenderPlacementPoints)
		{
			break;
		}

		if (IsTooCloseToAny(Candidate, ChosenPoints, PlacementMinSpacing))
		{
			continue;
		}

		if (FTerrainTile* Tile = GetTile(Candidate.X, Candidate.Y))
		{
			Tile->TileType = ETerrainTileType::Placeable;
			ChosenPoints.Add(Candidate);
			DefenderPlacementLocations.Add(GetTileSurfaceLocation(Candidate.X, Candidate.Y));
		}
	}
}

void AProceduralTerrainGenerator::BuildTileInstances()
{
	ConfigureInstancer(GroundTileInstancer, GroundTileMesh.Get());
	ConfigureInstancer(ElevatedTileInstancer, ElevatedTileMesh ? ElevatedTileMesh.Get() : GroundTileMesh.Get());
	ConfigureInstancer(PathTileInstancer, PathTileMesh ? PathTileMesh.Get() : GroundTileMesh.Get());
	ApplyTileMaterials();

	if (!GroundTileInstancer || !GroundTileInstancer->GetStaticMesh())
	{
		UE_LOG(LogTowerDefense, Error, TEXT("Terrain generation skipped mesh spawning: GroundTileMesh is not set."));
		return;
	}

	for (const FTerrainTile& Tile : TerrainTiles)
	{
		const float TileHeight = GetRenderedTileHeight(Tile);
		const float LocalX = (static_cast<float>(Tile.X) - (GridWidth - 1) * 0.5f) * TileSize;
		const float LocalY = (static_cast<float>(Tile.Y) - (GridHeight - 1) * 0.5f) * TileSize;
		const float LocalZ = TileHeight * 0.5f;
		const float ScaleXY = (Tile.TileType == ETerrainTileType::Path || Tile.TileType == ETerrainTileType::Tower)
			? (TileSize / 100.0f) * 0.92f
			: TileSize / 100.0f;

		FTransform InstanceTransform;
		InstanceTransform.SetLocation(FVector(LocalX, LocalY, LocalZ));
		InstanceTransform.SetScale3D(FVector(ScaleXY, ScaleXY, TileHeight / 100.0f));

		UHierarchicalInstancedStaticMeshComponent* Instancer = SelectInstancerForTile(Tile);
		if (!Instancer || !Instancer->GetStaticMesh())
		{
			Instancer = GroundTileInstancer.Get();
		}

		Instancer->AddInstance(InstanceTransform);
	}
}

void AProceduralTerrainGenerator::DrawPathDebug() const
{
	if (!bDrawDebugPaths)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const TArray<FColor> PathColors = { FColor::Cyan, FColor::Orange, FColor::Magenta, FColor::Yellow, FColor::Green };

	for (const FGeneratedPath& Path : GeneratedPaths)
	{
		const FColor Color = PathColors[Path.PathID % PathColors.Num()];

		for (int32 WaypointIndex = 0; WaypointIndex < Path.Waypoints.Num(); ++WaypointIndex)
		{
			const FVector& Waypoint = Path.Waypoints[WaypointIndex];
			DrawDebugSphere(World, Waypoint, 25.0f, 8, Color, true, -1.0f, 0, 2.0f);

			if (WaypointIndex + 1 < Path.Waypoints.Num())
			{
				DrawDebugLine(World, Waypoint, Path.Waypoints[WaypointIndex + 1], Color, true, -1.0f, 0, 6.0f);
			}
		}

		DrawDebugSphere(World, Path.SpawnLocation, 40.0f, 12, FColor::Red, true, -1.0f, 0, 2.0f);
		DrawDebugString(World, Path.SpawnLocation + FVector(0.0f, 0.0f, 80.0f),
			FString::Printf(TEXT("Spawn %d"), Path.PathID), nullptr, Color, 30.0f, true, 1.2f);
	}

	DrawDebugSphere(World, GetGridCenterWorldLocation() + FVector(0.0f, 0.0f, 80.0f), 50.0f, 12, FColor::Green, true, -1.0f, 0, 3.0f);
	DrawDebugString(World, GetGridCenterWorldLocation() + FVector(0.0f, 0.0f, 140.0f),
		TEXT("Tower"), nullptr, FColor::Green, 30.0f, true, 1.2f);

	for (const FVector& Placement : DefenderPlacementLocations)
	{
		DrawDebugBox(World, Placement + FVector(0.0f, 0.0f, 20.0f), FVector(35.0f, 35.0f, 20.0f), FColor::Yellow, true, -1.0f, 0, 2.0f);
	}
}

void AProceduralTerrainGenerator::ConfigureInstancer(UHierarchicalInstancedStaticMeshComponent* Instancer, UStaticMesh* Mesh) const
{
	if (!Instancer)
	{
		return;
	}

	Instancer->ClearInstances();

	if (Mesh && Instancer->GetStaticMesh() != Mesh)
	{
		Instancer->SetStaticMesh(Mesh);
	}
}

void AProceduralTerrainGenerator::ApplyTileMaterials()
{
	SetInstancerColor(GroundTileInstancer, GroundMaterialInstance, GroundColor);
	SetInstancerColor(ElevatedTileInstancer, GroundMaterialInstance, GroundColor);
	SetInstancerColor(PathTileInstancer, PathMaterialInstance, PathColor);
}

UMaterialInterface* AProceduralTerrainGenerator::GetSourceTileMaterial() const
{
	if (BaseTileMaterial)
	{
		return BaseTileMaterial.Get();
	}

	return LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
}

void AProceduralTerrainGenerator::SetInstancerColor(UHierarchicalInstancedStaticMeshComponent* Instancer, TObjectPtr<UMaterialInstanceDynamic>& MaterialInstance, const FLinearColor& Color)
{
	if (!Instancer)
	{
		return;
	}

	UMaterialInterface* SourceMaterial = GetSourceTileMaterial();
	if (!SourceMaterial)
	{
		UE_LOG(LogTowerDefense, Warning, TEXT("Could not load a base material for terrain tiles."));
		return;
	}

	if (!MaterialInstance || MaterialInstance->Parent != SourceMaterial)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(SourceMaterial, this);
	}

	if (!MaterialInstance)
	{
		return;
	}

	MaterialInstance->SetVectorParameterValue(TEXT("Color"), Color);
	MaterialInstance->SetVectorParameterValue(TEXT("BaseColor"), Color);
	Instancer->SetMaterial(0, MaterialInstance);

	UE_LOG(LogTowerDefense, Log, TEXT("Applied terrain colour %s to '%s'."),
		*Color.ToString(), *Instancer->GetName());
}

void AProceduralTerrainGenerator::InitializeRandomStream()
{
	if (bRandomizeSeedOnGenerate)
	{
		RandomSeed = FMath::RandRange(1, TNumericLimits<int32>::Max() - 1);
	}

	RandomStream.Initialize(RandomSeed);
	NoiseOrigin = FVector2D(RandomStream.FRandRange(-10000.0f, 10000.0f), RandomStream.FRandRange(-10000.0f, 10000.0f));
}

int32 AProceduralTerrainGenerator::ChooseHeightLevel(int32 X, int32 Y) const
{
	if (bFlattenCenter && IsInsideCenterRadius(X, Y))
	{
		return 0;
	}

	const float Noise = FMath::PerlinNoise2D(FVector2D(
		(static_cast<float>(X) + NoiseOrigin.X) * NoiseScale,
		(static_cast<float>(Y) + NoiseOrigin.Y) * NoiseScale));

	const float Normalized = FMath::Clamp((Noise + 1.0f) * 0.5f, 0.0f, 1.0f);
	return FMath::Clamp(FMath::RoundToInt(Normalized * MaxHeightLevel), 0, MaxHeightLevel);
}

ETerrainTileType AProceduralTerrainGenerator::ChooseTileType(int32 HeightLevel) const
{
	return HeightLevel > 0 ? ETerrainTileType::Elevated : ETerrainTileType::Ground;
}

bool AProceduralTerrainGenerator::IsInsideCenterRadius(int32 X, int32 Y) const
{
	const FIntPoint Center = GetCenterGridPosition();
	return FMath::Abs(X - Center.X) <= CenterClearRadius && FMath::Abs(Y - Center.Y) <= CenterClearRadius;
}

int32 AProceduralTerrainGenerator::GetTileIndex(int32 X, int32 Y) const
{
	return Y * GridWidth + X;
}

float AProceduralTerrainGenerator::GetTileWorldHeight(int32 HeightLevel) const
{
	return BaseTileHeight + static_cast<float>(HeightLevel) * HeightStep;
}

float AProceduralTerrainGenerator::GetRenderedTileHeight(const FTerrainTile& Tile) const
{
	if (Tile.TileType == ETerrainTileType::Path || Tile.TileType == ETerrainTileType::Tower)
	{
		return PathTileHeight;
	}

	return GetTileWorldHeight(Tile.HeightLevel);
}

TArray<FIntPoint> AProceduralTerrainGenerator::SelectPathStartCells()
{
	TArray<int32> EdgeOrder = { 0, 1, 2, 3 };
	for (int32 Index = EdgeOrder.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		EdgeOrder.Swap(Index, SwapIndex);
	}

	TArray<FIntPoint> Starts;
	Starts.Reserve(NumberOfPaths);

	for (int32 PathIndex = 0; PathIndex < NumberOfPaths; ++PathIndex)
	{
		FIntPoint Candidate = FIntPoint::ZeroValue;
		int32 Attempts = 0;
		do
		{
			Candidate = PickCellOnEdge(EdgeOrder[PathIndex % EdgeOrder.Num()]);
			++Attempts;
		}
		while (IsTooCloseToAny(Candidate, Starts, MinPathStartSeparation) && Attempts < 24);

		Starts.Add(Candidate);
	}

	return Starts;
}

FIntPoint AProceduralTerrainGenerator::PickCellOnEdge(int32 EdgeIndex)
{
	const int32 ClampedX = FMath::Max(GridWidth - 2, 1);
	const int32 ClampedY = FMath::Max(GridHeight - 2, 1);

	switch (EdgeIndex)
	{
	case 0: // North
		return FIntPoint(RandomStream.RandRange(1, ClampedX), 0);
	case 1: // South
		return FIntPoint(RandomStream.RandRange(1, ClampedX), GridHeight - 1);
	case 2: // East
		return FIntPoint(GridWidth - 1, RandomStream.RandRange(1, ClampedY));
	default: // West
		return FIntPoint(0, RandomStream.RandRange(1, ClampedY));
	}
}

bool AProceduralTerrainGenerator::IsTooCloseToAny(const FIntPoint& Candidate, const TArray<FIntPoint>& Existing, int32 MinDistance) const
{
	for (const FIntPoint& Other : Existing)
	{
		if (FMath::Abs(Candidate.X - Other.X) + FMath::Abs(Candidate.Y - Other.Y) < MinDistance)
		{
			return true;
		}
	}

	return false;
}

bool AProceduralTerrainGenerator::FindPathAStar(const FIntPoint& Start, const FIntPoint& Goal, TArray<FIntPoint>& OutPath) const
{
	OutPath.Reset();

	if (!IsValidCoordinate(Start.X, Start.Y) || !IsValidCoordinate(Goal.X, Goal.Y))
	{
		return false;
	}

	if (Start == Goal)
	{
		OutPath.Add(Start);
		return true;
	}

	const int32 NumTiles = GridWidth * GridHeight;
	TArray<FPathSearchNode> Nodes;
	Nodes.SetNum(NumTiles);

	auto Heuristic = [Goal](const FIntPoint& Point)
	{
		return (FMath::Abs(Point.X - Goal.X) + FMath::Abs(Point.Y - Goal.Y)) * 10;
	};

	const int32 StartIndex = GetTileIndex(Start.X, Start.Y);
	Nodes[StartIndex].G = 0;
	Nodes[StartIndex].H = Heuristic(Start);
	Nodes[StartIndex].bInOpenSet = true;

	TArray<int32> OpenSet;
	OpenSet.Add(StartIndex);

	const FIntPoint NeighborOffsets[4] = { FIntPoint(1, 0), FIntPoint(-1, 0), FIntPoint(0, 1), FIntPoint(0, -1) };

	while (OpenSet.Num() > 0)
	{
		int32 BestSlot = 0;
		int32 BestF = TNumericLimits<int32>::Max();
		for (int32 Slot = 0; Slot < OpenSet.Num(); ++Slot)
		{
			const FPathSearchNode& Candidate = Nodes[OpenSet[Slot]];
			const int32 FScore = Candidate.G + Candidate.H;
			if (FScore < BestF)
			{
				BestF = FScore;
				BestSlot = Slot;
			}
		}

		const int32 CurrentIndex = OpenSet[BestSlot];
		OpenSet.RemoveAtSwap(BestSlot);

		FPathSearchNode& Current = Nodes[CurrentIndex];
		Current.bInOpenSet = false;
		Current.bClosed = true;

		const FIntPoint CurrentCoord(CurrentIndex % GridWidth, CurrentIndex / GridWidth);
		if (CurrentCoord == Goal)
		{
			FIntPoint Cursor = Goal;
			while (Cursor.X >= 0 && Cursor.Y >= 0)
			{
				OutPath.Add(Cursor);
				Cursor = Nodes[GetTileIndex(Cursor.X, Cursor.Y)].Parent;
			}

			Algo::Reverse(OutPath);
			return OutPath.Num() > 1 && OutPath[0] == Start;
		}

		for (const FIntPoint& Offset : NeighborOffsets)
		{
			const FIntPoint Neighbor(CurrentCoord.X + Offset.X, CurrentCoord.Y + Offset.Y);
			if (!IsValidCoordinate(Neighbor.X, Neighbor.Y))
			{
				continue;
			}

			const int32 NeighborIndex = GetTileIndex(Neighbor.X, Neighbor.Y);
			FPathSearchNode& NeighborNode = Nodes[NeighborIndex];
			if (NeighborNode.bClosed)
			{
				continue;
			}

			const int32 TentativeG = Current.G + GetMoveCost(Neighbor, Goal);
			if (TentativeG >= NeighborNode.G)
			{
				continue;
			}

			NeighborNode.G = TentativeG;
			NeighborNode.H = Heuristic(Neighbor);
			NeighborNode.Parent = CurrentCoord;

			if (!NeighborNode.bInOpenSet)
			{
				NeighborNode.bInOpenSet = true;
				OpenSet.Add(NeighborIndex);
			}
		}
	}

	return false;
}

void AProceduralTerrainGenerator::FindPathGreedy(const FIntPoint& Start, const FIntPoint& Goal, TArray<FIntPoint>& OutPath)
{
	OutPath.Reset();
	FIntPoint Cursor = Start;
	OutPath.Add(Cursor);

	int32 Safety = GridWidth * GridHeight;
	while (Cursor != Goal && Safety-- > 0)
	{
		const bool bMoveX = Cursor.X != Goal.X && (Cursor.Y == Goal.Y || RandomStream.RandRange(0, 1) == 0);
		if (bMoveX)
		{
			Cursor.X += FMath::Sign(Goal.X - Cursor.X);
		}
		else if (Cursor.Y != Goal.Y)
		{
			Cursor.Y += FMath::Sign(Goal.Y - Cursor.Y);
		}

		OutPath.Add(Cursor);
	}
}

int32 AProceduralTerrainGenerator::GetMoveCost(const FIntPoint& To, const FIntPoint& Goal) const
{
	int32 Cost = 10;
	const FTerrainTile* Tile = GetTile(To.X, To.Y);
	if (!Tile)
	{
		return Cost;
	}

	if (Tile->HeightLevel > 0 && Tile->TileType != ETerrainTileType::Path)
	{
		Cost += 8 * Tile->HeightLevel;
	}

	const int32 DistanceToGoal = FMath::Abs(To.X - Goal.X) + FMath::Abs(To.Y - Goal.Y);
	if (Tile->TileType == ETerrainTileType::Path)
	{
		Cost = (DistanceToGoal <= 3) ? 2 : Cost + 12;
	}

	return Cost;
}

void AProceduralTerrainGenerator::ApplyPathTiles(const TArray<FIntPoint>& Points)
{
	for (const FIntPoint& Point : Points)
	{
		FTerrainTile* Tile = GetTile(Point.X, Point.Y);
		if (!Tile || Tile->TileType == ETerrainTileType::Tower)
		{
			continue;
		}

		Tile->TileType = ETerrainTileType::Path;
		Tile->HeightLevel = 0;
		Tile->WorldLocation = GridToWorldLocation(Point.X, Point.Y, 0);
	}
}

FGeneratedPath AProceduralTerrainGenerator::BuildGeneratedPath(int32 PathID, const TArray<FIntPoint>& GridPoints) const
{
	FGeneratedPath Path;
	Path.PathID = PathID;
	Path.GridPoints = GridPoints;
	Path.SpawnGridPosition = GridPoints.Num() > 0 ? GridPoints[0] : FIntPoint::ZeroValue;

	Path.Waypoints.Reserve(GridPoints.Num());
	for (const FIntPoint& Point : GridPoints)
	{
		Path.Waypoints.Add(GetTileSurfaceLocation(Point.X, Point.Y));
	}

	if (Path.Waypoints.Num() > 0)
	{
		Path.SpawnLocation = Path.Waypoints[0] + FVector(0.0f, 0.0f, SpawnHeightOffset);
	}

	return Path;
}

bool AProceduralTerrainGenerator::IsValidDefenderPlacementTile(const FTerrainTile& Tile) const
{
	if (Tile.TileType == ETerrainTileType::Path || Tile.TileType == ETerrainTileType::Tower)
	{
		return false;
	}

	if (IsInsideCenterRadius(Tile.X, Tile.Y))
	{
		return false;
	}

	const bool bIsEdge = Tile.X == 0 || Tile.Y == 0 || Tile.X == GridWidth - 1 || Tile.Y == GridHeight - 1;
	return !bIsEdge;
}

UHierarchicalInstancedStaticMeshComponent* AProceduralTerrainGenerator::SelectInstancerForTile(const FTerrainTile& Tile) const
{
	if (Tile.TileType == ETerrainTileType::Path || Tile.TileType == ETerrainTileType::Tower)
	{
		return PathTileInstancer.Get();
	}

	if (Tile.TileType == ETerrainTileType::Elevated)
	{
		return ElevatedTileInstancer.Get();
	}

	return GroundTileInstancer.Get();
}

bool AProceduralTerrainGenerator::IsValidCoordinate(int32 X, int32 Y) const
{
	return X >= 0 && Y >= 0 && X < GridWidth && Y < GridHeight;
}

FVector AProceduralTerrainGenerator::GridToWorldLocation(int32 X, int32 Y, int32 HeightLevel) const
{
	const float LocalX = (static_cast<float>(X) - (GridWidth - 1) * 0.5f) * TileSize;
	const float LocalY = (static_cast<float>(Y) - (GridHeight - 1) * 0.5f) * TileSize;
	const float LocalZ = GetTileWorldHeight(HeightLevel) * 0.5f;
	return GetActorLocation() + FVector(LocalX, LocalY, LocalZ);
}

FVector AProceduralTerrainGenerator::GetTileSurfaceLocation(int32 X, int32 Y) const
{
	const FTerrainTile* Tile = GetTile(X, Y);
	const float TileHeight = Tile ? GetRenderedTileHeight(*Tile) : PathTileHeight;
	const float LocalX = (static_cast<float>(X) - (GridWidth - 1) * 0.5f) * TileSize;
	const float LocalY = (static_cast<float>(Y) - (GridHeight - 1) * 0.5f) * TileSize;
	return GetActorLocation() + FVector(LocalX, LocalY, TileHeight);
}

bool AProceduralTerrainGenerator::WorldToGrid(const FVector& WorldLocation, int32& OutX, int32& OutY) const
{
	const FVector Local = WorldLocation - GetActorLocation();
	const float OriginOffsetX = (GridWidth - 1) * 0.5f * TileSize;
	const float OriginOffsetY = (GridHeight - 1) * 0.5f * TileSize;

	OutX = FMath::RoundToInt((Local.X + OriginOffsetX) / TileSize);
	OutY = FMath::RoundToInt((Local.Y + OriginOffsetY) / TileSize);
	return IsValidCoordinate(OutX, OutY);
}

FIntPoint AProceduralTerrainGenerator::GetCenterGridPosition() const
{
	return FIntPoint(GridWidth / 2, GridHeight / 2);
}

FVector AProceduralTerrainGenerator::GetGridCenterWorldLocation() const
{
	const FIntPoint Center = GetCenterGridPosition();
	return GridToWorldLocation(Center.X, Center.Y, 0);
}

const FTerrainTile* AProceduralTerrainGenerator::GetTile(int32 X, int32 Y) const
{
	if (!IsValidCoordinate(X, Y))
	{
		return nullptr;
	}

	const int32 Index = GetTileIndex(X, Y);
	return TerrainTiles.IsValidIndex(Index) ? &TerrainTiles[Index] : nullptr;
}

FTerrainTile* AProceduralTerrainGenerator::GetTile(int32 X, int32 Y)
{
	if (!IsValidCoordinate(X, Y))
	{
		return nullptr;
	}

	const int32 Index = GetTileIndex(X, Y);
	return TerrainTiles.IsValidIndex(Index) ? &TerrainTiles[Index] : nullptr;
}
