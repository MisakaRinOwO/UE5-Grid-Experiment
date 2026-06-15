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

void AGridManager::HandleGridInteraction()
{
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	if (!PlayerController)
	{
		return;
	}

	if (!PlayerController->WasInputKeyJustPressed(ToggleObstacleKey))
	{
		return;
	}

	FGridCoord HitCoord;

	if (bRaytraceByCursor) {
		if (TryGetLookAtGridCoordCursor(HitCoord))
		{
			ToggleObstacle(HitCoord);
		}
	}
	else {
		if (TryGetLookAtGridCoordCamera(HitCoord))
		{
			ToggleObstacle(HitCoord);
		}
	}

}

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