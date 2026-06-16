#include "GridManager.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();

	InitializeGrid();
}

void AGridManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	HandleGridInteraction();

	if (bDrawDebugGrid)
	{
		DrawGridDebug();
	}
}

void AGridManager::InitializeGrid()
{
	Cells.Empty();
	Cells.Reserve(GridWidth * GridHeight);

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			FGridCell NewCell;

			NewCell.Coord.X = X;
			NewCell.Coord.Y = Y;
			NewCell.bBlocked = false;
			NewCell.MoveCost = 1.0f;

			Cells.Add(NewCell);
		}
	}
}

bool AGridManager::IsValidCoord(FGridCoord Coord) const
{
	return Coord.X >= 0
		&& Coord.X < GridWidth
		&& Coord.Y >= 0
		&& Coord.Y < GridHeight;
}

int32 AGridManager::CoordToIndex(FGridCoord Coord) const
{
	return Coord.Y * GridWidth + Coord.X;
}

FVector AGridManager::GridToWorld(FGridCoord Coord) const
{
	const FVector Origin = GetActorLocation();

	const float WorldX = (Coord.X + 0.5f) * CellSize;
	const float WorldY = (Coord.Y + 0.5f) * CellSize;

	return Origin + FVector(WorldX, WorldY, 0.0f);
}

bool AGridManager::WorldToGrid(FVector WorldLocation, FGridCoord& OutCoord) const
{
	const FVector LocalLocation = WorldLocation - GetActorLocation();

	OutCoord.X = FMath::FloorToInt(LocalLocation.X / CellSize);
	OutCoord.Y = FMath::FloorToInt(LocalLocation.Y / CellSize);

	return IsValidCoord(OutCoord);
}

bool AGridManager::ToggleObstacle(FGridCoord Coord)
{
	if (!IsValidCoord(Coord))
	{
		return false;
	}

	const int32 Index = CoordToIndex(Coord);

	if (!Cells.IsValidIndex(Index))
	{
		return false;
	}

	Cells[Index].bBlocked = !Cells[Index].bBlocked;

	return true;
}

/*
Per-tick handler
*/
void AGridManager::HandleGridInteraction()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(RunPathfindingKey))
	{
		FindPath();
		return;
	}

	FGridCoord HitCoord;

	if (!TryGetInteractionCoord(HitCoord))
	{
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(ToggleObstacleKey))
	{
		ToggleObstacle(HitCoord);
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(SetStartKey))
	{
		SetStartCoord(HitCoord);
		return;
	}

	if (PlayerController->WasInputKeyJustPressed(SetGoalKey))
	{
		SetGoalCoord(HitCoord);
		return;
	}
}
/*
Interaction Helper
*/
bool AGridManager::TryGetInteractionCoord(FGridCoord& OutCoord) const
{
	if (bRaytraceByCursor)
	{
		return TryGetLookAtGridCoordCursor(OutCoord);
	}

	return TryGetLookAtGridCoordCamera(OutCoord);
}

/*
Start/Goal coord setter
*/
bool AGridManager::SetStartCoord(FGridCoord Coord)
{
	if (!IsWalkableCoord(Coord))
	{
		return false;
	}

	StartCoord = Coord;
	bHasStartCoord = true;

	CurrentPath.Empty();

	return true;
}

bool AGridManager::SetGoalCoord(FGridCoord Coord)
{
	if (!IsWalkableCoord(Coord))
	{
		return false;
	}

	GoalCoord = Coord;
	bHasGoalCoord = true;

	CurrentPath.Empty();

	return true;
}

/*
Walkable check
*/
bool AGridManager::IsWalkableCoord(FGridCoord Coord) const
{
	if (!IsValidCoord(Coord))
	{
		return false;
	}

	const int32 Index = CoordToIndex(Coord);

	if (!Cells.IsValidIndex(Index))
	{
		return false;
	}

	return !Cells[Index].bBlocked;
}

/*
Cursor and camera center raytrace method
*/
bool AGridManager::TryGetLookAtGridCoordCursor(FGridCoord& OutCoord) const
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController || !GetWorld())
	{
		return false;
	}

	float MouseX, MouseY;
	if (!PlayerController->GetMousePosition(MouseX, MouseY))
	{
		return false;
	}

	FVector WorldLocation, WorldDirection;
	if (!PlayerController->DeprojectScreenPositionToWorld(MouseX, MouseY, WorldLocation, WorldDirection))
	{
		return false;
	}

	const FVector TraceStart = WorldLocation;
	const FVector TraceEnd = TraceStart + WorldDirection * TraceDistance;

	FHitResult HitResult;
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility
	);

	if (bDrawInteractionTrace)
	{
		const FColor TraceColor = bHit ? FColor::Blue : FColor::Red;
		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			TraceColor,
			false,
			1.0f,
			0,
			2.0f
		);
	}

	if (!bHit)
	{
		return false;
	}

	return WorldToGrid(HitResult.ImpactPoint, OutCoord);
}

bool AGridManager::TryGetLookAtGridCoordCamera(FGridCoord& OutCoord) const
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController || !GetWorld())
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

	const FVector TraceStart = ViewLocation;
	const FVector TraceEnd = TraceStart + ViewRotation.Vector() * TraceDistance;

	FHitResult HitResult;

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility
	);

	if (bDrawInteractionTrace)
	{
		const FColor TraceColor = bHit ? FColor::Blue : FColor::Red;

		DrawDebugLine(
			GetWorld(),
			TraceStart,
			TraceEnd,
			TraceColor,
			false,
			1.0f,
			0,
			2.0f
		);
	}

	if (!bHit)
	{
		return false;
	}

	return WorldToGrid(HitResult.ImpactPoint, OutCoord);
}

/*
A* helper, 4-directional neighbor check
*/
void AGridManager::GetNeighbors(FGridCoord Coord, TArray<FGridCoord>& OutNeighbors) const
{
	OutNeighbors.Empty();

	const FGridCoord Right{ Coord.X + 1, Coord.Y };
	const FGridCoord Left{ Coord.X - 1, Coord.Y };
	const FGridCoord Up{ Coord.X, Coord.Y + 1 };
	const FGridCoord Down{ Coord.X, Coord.Y - 1 };

	if (IsWalkableCoord(Right))
	{
		OutNeighbors.Add(Right);
	}

	if (IsWalkableCoord(Left))
	{
		OutNeighbors.Add(Left);
	}

	if (IsWalkableCoord(Up))
	{
		OutNeighbors.Add(Up);
	}

	if (IsWalkableCoord(Down))
	{
		OutNeighbors.Add(Down);
	}
}

/// ------------ A* helpers ------------

/*
Heuristic value calculation(Manhatan distance)
*/
float AGridManager::GetHeuristicCost(FGridCoord From, FGridCoord To) const
{
	return FMath::Abs(From.X - To.X) + FMath::Abs(From.Y - To.Y);
}

void AGridManager::ReconstructPath(const TArray<FGridPathNode>& PathNodes, int32 GoalIndex)
{
	CurrentPath.Empty();

	int32 CurrentIndex = GoalIndex;

	while (PathNodes.IsValidIndex(CurrentIndex))
	{
		if (!Cells.IsValidIndex(CurrentIndex))
		{
			break;
		}

		CurrentPath.Insert(Cells[CurrentIndex].Coord, 0);

		CurrentIndex = PathNodes[CurrentIndex].ParentIndex;

		if (CurrentIndex == -1)
		{
			break;
		}
	}
}

bool AGridManager::IsCoordInCurrentPath(FGridCoord Coord) const
{
	for (const FGridCoord& PathCoord : CurrentPath)
	{
		if (PathCoord.X == Coord.X && PathCoord.Y == Coord.Y)
		{
			return true;
		}
	}

	return false;
}

/// ------------------------------------

/*
A* pathfinding algorithm
*/
bool AGridManager::FindPath()
{
	CurrentPath.Empty();

	if (!bHasStartCoord || !bHasGoalCoord)
	{
		return false;
	}

	if (!IsWalkableCoord(StartCoord) || !IsWalkableCoord(GoalCoord))
	{
		return false;
	}

	const int32 StartIndex = CoordToIndex(StartCoord);
	const int32 GoalIndex = CoordToIndex(GoalCoord);

	if (!Cells.IsValidIndex(StartIndex) || !Cells.IsValidIndex(GoalIndex))
	{
		return false;
	}

	TArray<FGridPathNode> PathNodes;
	PathNodes.SetNum(Cells.Num());

	const float InitialCost = 1000000000.0f;

	for (int32 Index = 0; Index < PathNodes.Num(); ++Index)
	{
		PathNodes[Index].CellIndex = Index;
		PathNodes[Index].ParentIndex = -1;
		PathNodes[Index].GCost = InitialCost;
		PathNodes[Index].HCost = 0.0f;
		PathNodes[Index].bOpen = false;
		PathNodes[Index].bClosed = false;
	}

	TArray<int32> OpenList;

	PathNodes[StartIndex].GCost = 0.0f;
	PathNodes[StartIndex].HCost = GetHeuristicCost(StartCoord, GoalCoord);
	PathNodes[StartIndex].bOpen = true;

	OpenList.Add(StartIndex);

	while (OpenList.Num() > 0)
	{
		int32 BestOpenListPosition = 0;
		int32 CurrentIndex = OpenList[0];

		for (int32 i = 1; i < OpenList.Num(); ++i)
		{
			const int32 CandidateIndex = OpenList[i];

			const bool bLowerFCost =
				PathNodes[CandidateIndex].GetFCost() < PathNodes[CurrentIndex].GetFCost();

			const bool bSameFCostLowerH =
				FMath::IsNearlyEqual(PathNodes[CandidateIndex].GetFCost(), PathNodes[CurrentIndex].GetFCost())
				&& PathNodes[CandidateIndex].HCost < PathNodes[CurrentIndex].HCost;

			if (bLowerFCost || bSameFCostLowerH)
			{
				CurrentIndex = CandidateIndex;
				BestOpenListPosition = i;
			}
		}

		OpenList.RemoveAtSwap(BestOpenListPosition);

		PathNodes[CurrentIndex].bOpen = false;
		PathNodes[CurrentIndex].bClosed = true;

		if (CurrentIndex == GoalIndex)
		{
			ReconstructPath(PathNodes, GoalIndex);
			return true;
		}

		const FGridCoord CurrentCoord = Cells[CurrentIndex].Coord;

		TArray<FGridCoord> Neighbors;
		GetNeighbors(CurrentCoord, Neighbors);

		for (const FGridCoord& NeighborCoord : Neighbors)
		{
			const int32 NeighborIndex = CoordToIndex(NeighborCoord);

			if (!Cells.IsValidIndex(NeighborIndex))
			{
				continue;
			}

			if (PathNodes[NeighborIndex].bClosed)
			{
				continue;
			}

			const float NewGCost =
				PathNodes[CurrentIndex].GCost + Cells[NeighborIndex].MoveCost;

			if (!PathNodes[NeighborIndex].bOpen || NewGCost < PathNodes[NeighborIndex].GCost)
			{
				PathNodes[NeighborIndex].GCost = NewGCost;
				PathNodes[NeighborIndex].HCost = GetHeuristicCost(NeighborCoord, GoalCoord);
				PathNodes[NeighborIndex].ParentIndex = CurrentIndex;

				if (!PathNodes[NeighborIndex].bOpen)
				{
					PathNodes[NeighborIndex].bOpen = true;
					OpenList.Add(NeighborIndex);
				}
			}
		}
	}

	return false;
}

/*
Visual debug
*/
void AGridManager::DrawGridDebug() const
{
	if (!GetWorld())
	{
		return;
	}

	for (const FGridCell& Cell : Cells)
	{
		const FVector Center = GridToWorld(Cell.Coord);
		const FVector Extent = FVector(CellSize * 0.5f, CellSize * 0.5f, 5.0f);

		FColor CellColor = FColor::Green;

		if (Cell.bBlocked)
		{
			CellColor = FColor::Red;
		}

		if (IsCoordInCurrentPath(Cell.Coord))
		{
			CellColor = FColor::Blue;
		}

		if (bHasStartCoord && Cell.Coord.X == StartCoord.X && Cell.Coord.Y == StartCoord.Y)
		{
			CellColor = FColor::Yellow;
		}

		if (bHasGoalCoord && Cell.Coord.X == GoalCoord.X && Cell.Coord.Y == GoalCoord.Y)
		{
			CellColor = FColor::Purple;
		}

		DrawDebugBox(
			GetWorld(),
			Center,
			Extent,
			CellColor,
			false,
			0.0f,
			0,
			2.0f
		);
	}
}