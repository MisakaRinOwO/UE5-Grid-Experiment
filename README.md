# Grid Systems Experiment

A focused Unreal Engine 5 C++ gameplay systems experiment for testing data-driven grid interaction, cursor-to-grid mapping, obstacle editing, and one-shot A* pathfinding visualization.

This project was built to strengthen my UE5 C++ workflow and gameplay programming fundamentals, especially the transition from Blueprint-driven implementation toward native C++ system logic.

## Project Goals

The goal of this experiment is not to build a full tactics game, but to prove a small, explainable gameplay system:

* Store grid data in C++ instead of spawning one Actor per cell.
* Convert world-space cursor interaction into logical grid coordinates.
* Edit cell walkability through player input.
* Set pathfinding start and goal cells.
* Run A* pathfinding on the grid.
* Visualize grid state, obstacles, and path results with debug drawing.

## Current Status

Implemented through Lesson 3:

* UE5 C++ project created from scratch.
* `AGridManager` C++ Actor implemented.
* Grid stored as `TArray<FGridCell>`.
* Logical grid coordinates implemented with `FGridCoord`.
* World position to grid coordinate conversion.
* Grid coordinate to world position conversion.
* Debug grid rendering with `DrawDebugBox`.
* Event-driven input through Enhanced Input / IMC.
* Top-down pawn with WASD movement.
* Mouse cursor raytrace using screen deprojection and line trace.
* Cursor hit location converted into grid coordinate.
* Obstacle toggle interaction.
* Start cell selection.
* Goal cell selection.
* One-shot A* pathfinding.
* Debug visualization for walkable cells, blocked cells, start, goal, and final path.

## Controls

| Input             | Action                                     |
| ----------------- | ------------------------------------------ |
| Left Mouse Button | Toggle obstacle on the hovered grid cell   |
| 1                 | Set hovered grid cell as pathfinding start |
| 2                 | Set hovered grid cell as pathfinding goal  |
| Space             | Run A* pathfinding                         |
| WASD              | Move top-down pawn                         |

## Core System Design

### Data-Driven Grid

The grid is managed by a single `AGridManager` Actor.

Instead of spawning one Actor for every grid cell, each cell is stored as data:

```cpp
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
```

The grid is stored in a one-dimensional `TArray<FGridCell>`:

```cpp
Index = Y * GridWidth + X;
```

This keeps the system lightweight and avoids the scalability problem of actor-per-cell grid implementations.

### Coordinate Conversion

The system separates logical grid space from world space.

`GridToWorld()` converts a logical grid coordinate into a world-space cell center.

`WorldToGrid()` converts a world-space hit location into a logical grid coordinate.

This allows player input, raytrace hits, and future gameplay systems to interact with the same grid data model.

### Event-Driven Input

Input is handled outside the `GridManager`.

The Player Controller / Pawn Blueprint owns Enhanced Input events and cursor raytracing. The `GridManager` owns only the grid data and gameplay logic.

Current input flow:

```text
Enhanced Input Action
→ Mouse position raytrace
→ Hit location
→ WorldToGrid()
→ ToggleObstacle / SetStartCoord / SetGoalCoord / FindPath
```

This keeps input handling separate from the grid/pathfinding system.

### Cursor Raytrace

The current interaction model uses mouse position deprojection:

```text
Screen mouse position
→ Deproject to world position and direction
→ Line trace by channel
→ Hit location
→ Convert hit location to grid coordinate
```

The raytrace function returns both:

* `bool`: whether the trace and conversion succeeded
* `FGridCoord`: the resulting grid coordinate

This prevents invalid trace results from accidentally modifying the grid.

## A* Pathfinding

The current A* implementation is a one-shot search.

When the player presses Space, the system searches from the selected start cell to the selected goal cell while avoiding blocked cells.

Current pathfinding rules:

* 4-directional movement
* No diagonal movement
* Blocked cells are ignored
* Manhattan distance heuristic
* Cell movement cost supported through `MoveCost`

The final path is stored as:

```cpp
TArray<FGridCoord> CurrentPath;
```

and visualized through debug drawing.

## Debug Visualization

Current debug colors:

| Color  | Meaning       |
| ------ | ------------- |
| Green  | Walkable cell |
| Red    | Blocked cell  |
| Yellow | Start cell    |
| Purple | Goal cell     |
| Blue   | Final A* path |

The grid is rendered using `DrawDebugBox`, which makes the system easy to inspect without needing custom meshes or materials.

## Technical Highlights

* UE5 C++ Actor created from scratch.
* C++ `USTRUCT` data model for grid coordinates and cells.
* `UPROPERTY` / `UFUNCTION` exposure for Blueprint and Editor interaction.
* One-dimensional array indexing for 2D grid data.
* Event-driven Enhanced Input workflow.
* Mouse cursor raytrace to logical grid mapping.
* C++ A* pathfinding implementation.
* Debug-first visualization approach.
* Clear separation between input layer and grid/pathfinding logic.

## Why This Matters

This project focuses on a common gameplay programming problem: representing and interacting with a grid efficiently.

The key design decision is to avoid actor-based grid cells. Instead, the grid exists as data, and the world visualization is derived from that data.

This makes the system easier to scale, easier to reason about, and better suited for future pathfinding, cost terrain, flow field, tactics, or placement systems.

## Current Limitations

* A* currently runs instantly in one frame.
* Open and closed search sets are not visualized yet.
* No per-tick frontier expansion yet.
* No cost terrain editing yet.
* No diagonal movement option yet.
* Debug drawing is used instead of final gameplay visuals.
* The project is currently a systems experiment, not a full playable game.

## Planned Next Steps

### Per-Tick A* Frontier Expansion

Refactor one-shot A* into a multi-frame search process:

```text
StartPathfinding()
→ Initialize search state
→ StepPathfinding() each Tick
→ Visualize open / closed / final path
```

This will make the search process visible and easier to present in a portfolio GIF.

### Open / Closed Debug Visualization

Add separate debug colors for:

* Open frontier
* Closed / visited cells
* Final path

### Cost Terrain or Flow Field

Possible follow-up experiment:

* Add terrain movement costs and show how A* chooses cheaper routes.
* Or implement a flow field from goal cell to all reachable cells.

## Portfolio Positioning

This project is a UE5 C++ gameplay programming study focused on grid-based systems.

It demonstrates:

* Native C++ gameplay logic
* Grid data modeling
* Input-to-world interaction
* Pathfinding fundamentals
* Debug visualization
* System-oriented thinking

The project is intentionally scoped small so that the implementation remains readable, explainable, and easy to extend.
