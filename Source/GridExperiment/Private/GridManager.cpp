#include "GridManager.h"
#include "DrawDebugHelpers.h"

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

		const FColor CellColor = Cell.bBlocked ? FColor::Red : FColor::Green;

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