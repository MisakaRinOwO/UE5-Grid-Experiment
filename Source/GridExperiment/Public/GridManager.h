#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "InputCoreTypes.h"
#include "GridManager.generated.h"

USTRUCT(BlueprintType)
struct FGridCoord
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 X = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 Y = 0;
};

USTRUCT(BlueprintType)
struct FGridCell
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	FGridCoord Coord;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	bool bBlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float MoveCost = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 MoveCostListIndex = 0;
};
	
/*
Temp internal structure, no need to expose as USTRUCT
*/
struct FGridPathNode
{
	int32 CellIndex = -1;
	int32 ParentIndex = -1;

	float GCost = 0.0f;
	float HCost = 0.0f;

	bool bOpen = false;
	bool bClosed = false;

	float GetFCost() const
	{
		return GCost + HCost;
	}
};

UCLASS()
class GRIDEXPERIMENT_API AGridManager : public AActor
{
	GENERATED_BODY()

public:
	AGridManager();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridWidth = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	int32 GridHeight = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
	float CellSize = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	bool bDrawDebugGrid = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	float TraceDistance = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Terrain")
	TArray<float> MoveCostList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Terrain")
	float BlockedMoveCost = 99999.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	FKey ToggleObstacleKey = EKeys::LeftMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	FKey SetStartKey = EKeys::One;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	FKey SetGoalKey = EKeys::Two;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	FKey RunPathfindingKey = EKeys::SpaceBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	bool bDrawInteractionTrace = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<FGridCell> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Control")
	bool bRaytraceByCursor = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	bool bHasStartCoord = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	bool bHasGoalCoord = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	FGridCoord StartCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	FGridCoord GoalCoord;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
	TArray<FGridCoord> CurrentPath;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	void InitializeGrid();

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsValidCoord(FGridCoord Coord) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	int32 CoordToIndex(FGridCoord Coord) const;

	UFUNCTION(BlueprintPure, Category = "Grid")
	FVector GridToWorld(FGridCoord Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Grid")
	bool WorldToGrid(FVector WorldLocation, FGridCoord& OutCoord) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Interaction")
	bool ToggleObstacle(FGridCoord Coord);

	UFUNCTION(BlueprintCallable, Category = "Grid|Interaction")
	bool CycleGridCost(FGridCoord Coord);

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	bool SetStartCoord(FGridCoord Coord);

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	bool SetGoalCoord(FGridCoord Coord);

	UFUNCTION(BlueprintCallable, Category = "Pathfinding")
	bool FindPath();

	UFUNCTION(BlueprintPure, Category = "Grid")
	bool IsWalkableCoord(FGridCoord Coord) const;

	UFUNCTION(BlueprintCallable, Category = "Grid|Debug")
	void DrawGridDebug() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	bool bDrawCellCostText = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	float CellCostTextZOffset = 40.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	float CellCostTextScale = 1.2f;

private:
	void HandleGridInteraction();

	bool TryGetLookAtGridCoordCursor(FGridCoord& OutCoord) const;

	bool TryGetLookAtGridCoordCamera(FGridCoord& OutCoord) const;

	bool TryGetInteractionCoord(FGridCoord& OutCoord) const;

	void GetNeighbors(FGridCoord Coord, TArray<FGridCoord>& OutNeighbors) const;

	float GetHeuristicCost(FGridCoord From, FGridCoord To) const;

	void ReconstructPath(const TArray<FGridPathNode>& PathNodes, int32 GoalIndex);

	bool IsCoordInCurrentPath(FGridCoord Coord) const;
};