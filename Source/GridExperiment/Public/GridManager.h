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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Interaction")
	FKey ToggleObstacleKey = EKeys::LeftMouseButton;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid|Debug")
	bool bDrawInteractionTrace = true;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid")
	TArray<FGridCell> Cells;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid|Control")
	bool bRaytraceByCursor = true;

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


private:
	void DrawGridDebug() const;

	void HandleGridInteraction();

	bool TryGetLookAtGridCoordCursor(FGridCoord& OutCoord) const;

	bool TryGetLookAtGridCoordCamera(FGridCoord& OutCoord) const;
};