# Grid Systems Experiment

A focused Unreal Engine 5 C++ gameplay systems experiment for testing reusable grid logic across multiple gameplay scenarios.

The project started as a UE5 C++ learning exercise and has grown into a small grid-framework prototype: one shared `AGridManager` core provides grid data, coordinate conversion, obstacle state, and A* pathfinding, while each level uses a different Player Controller / scenario layer to test a different gameplay use case.

## Project Goals

The goal of this experiment is not to build a full game. The goal is to prove a small, explainable gameplay system that can support different grid-based rules.

Core goals:

- Store grid data in C++ instead of spawning one Actor per cell.
- Convert world-space cursor interaction into logical grid coordinates.
- Edit cell walkability through player input.
- Set pathfinding start and goal cells.
- Run A* pathfinding on the grid.
- Visualize grid state, obstacles, and path results with debug drawing.
- Reuse one grid/pathfinding core across multiple gameplay scenarios.

## Current Status

### Core Grid Framework

Implemented:

- UE5 C++ project created from scratch.
- `AGridManager` C++ Actor implemented.
- Grid stored as `TArray<FGridCell>`.
- Logical grid coordinates implemented with `FGridCoord`.
- World position to grid coordinate conversion.
- Grid coordinate to world position conversion.
- Grid rendered with `DrawDebugBox`.
- Path rendered as connected debug lines between path cell centers.
- Event-driven input through Enhanced Input / IMC.
- Top-down pawn with WASD movement.
- Mouse cursor raytrace using screen deprojection and line trace.
- Cursor hit location converted into grid coordinate.
- Obstacle toggle interaction.
- Start cell selection.
- Goal cell selection.
- One-shot A* pathfinding.
- Path validity check.
- Scenario-level rule logic built on top of the shared C++ grid core.

### Scenario 1: Tower Defense Path Rerouting

Completed as the first gameplay scenario.

This level demonstrates dynamic path rerouting on a shared C++ grid system. The player can set a start cell, set a goal cell, and toggle obstacle cells. Whenever the grid changes, the scenario recalculates the route. If a new obstacle would completely block the path from start to goal, the placement is rejected and the obstacle change is rolled back.

Implemented:

- Dedicated Tower Defense Player Controller scenario layer.
- Fixed route visualization between start and goal cells.
- Automatic path recalculation after grid edits.
- Obstacle placement validation.
- Invalid blockage rollback when a placement would fully block the route.
- UI guide for controls, color meaning, and scenario purpose.
- Blue route line drawn from `CurrentPath[i]` to `CurrentPath[i + 1]`.

## Controls

Current Tower Defense Path Rerouting level:

| Input | Action |
|---|---|
| Left Mouse Button | Toggle obstacle on the hovered cell |
| 1 | Set hovered cell as start |
| 2 | Set hovered cell as goal |
| WASD | Move top-down pawn |

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

This allows player input, raytrace hits, pathfinding, and scenario rules to operate on the same grid data model.

### Event-Driven Input

Input is handled outside the `GridManager`.

The Player Controller / Pawn Blueprint owns Enhanced Input events and cursor raytracing. The `GridManager` owns the reusable grid data and pathfinding logic.

Current input flow:

```text
Enhanced Input Action
→ Mouse position raytrace
→ Hit location
→ WorldToGrid()
→ Scenario rule
→ GridManager function call
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

- `bool`: whether the trace and conversion succeeded
- `FGridCoord`: the resulting grid coordinate

This prevents invalid trace results from accidentally modifying the grid.

## A* Pathfinding

The current A* implementation is a one-shot search.

When start and goal cells are valid, the system searches from start to goal while avoiding blocked cells.

Current pathfinding rules:

- 4-directional movement
- No diagonal movement
- Blocked cells are ignored
- Manhattan distance heuristic
- Cell movement cost supported through `MoveCost`

The final path is stored as:

```cpp
TArray<FGridCoord> CurrentPath;
```

The route is visualized by drawing debug lines between consecutive cells in the path:

```cpp
for (int32 i = 0; i < CurrentPath.Num() - 1; ++i)
{
    const FVector StartWorld = GridToWorld(CurrentPath[i]);
    const FVector EndWorld = GridToWorld(CurrentPath[i + 1]);

    DrawDebugLine(
        GetWorld(),
        StartWorld,
        EndWorld,
        FColor::Blue,
        false,
        0.0f,
        0,
        8.0f
    );
}
```

## Scenario 1: Tower Defense Path Rerouting

This scenario tests whether the shared grid core can support a tower-defense-style placement rule.

### Gameplay Rule

The player can edit obstacles, but the path from start to goal must remain valid.

Current rule flow:

```text
Player toggles obstacle
→ Scenario recalculates route
→ If path exists: keep obstacle
→ If path is blocked: rollback obstacle change
```

This turns the pathfinding system into a gameplay rule instead of a standalone algorithm demo.

### Scenario Architecture

The scenario logic is handled in the Player Controller for this level.

```text
PC_TowerDefensePathReroute
→ Handles input
→ Runs cursor raytrace
→ Calls GridManager operations
→ Recalculates path
→ Rolls back invalid obstacle placement
```

`AGridManager` remains the reusable core:

```text
AGridManager
→ Stores grid cells
→ Converts world/grid coordinates
→ Tracks walkability
→ Runs A*
→ Draws debug grid and route
```

This separation keeps the grid system reusable for future scenarios.

## Debug Visualization

Current debug colors:

| Color | Meaning |
|---|---|
| Green | Walkable cell |
| Red | Obstacle cell |
| Yellow | Start cell |
| Purple | Goal cell |
| Blue line | Current route |

The grid is rendered using `DrawDebugBox`, while the final route is rendered as connected `DrawDebugLine` segments. This makes the system easy to inspect without custom meshes or materials.

## Technical Highlights

- UE5 C++ Actor created from scratch.
- C++ `USTRUCT` data model for grid coordinates and cells.
- `UPROPERTY` / `UFUNCTION` exposure for Blueprint and Editor interaction.
- One-dimensional array indexing for 2D grid data.
- Event-driven Enhanced Input workflow.
- Mouse cursor raytrace to logical grid mapping.
- C++ A* pathfinding implementation.
- Runtime path validation.
- Invalid placement rollback.
- Debug-first visualization approach.
- Clear separation between input layer, scenario rule layer, and grid/pathfinding core.

## Why This Matters

This project focuses on a common gameplay programming problem: representing, editing, and querying grid data efficiently.

The key design decision is to avoid actor-based grid cells. Instead, the grid exists as data, and the world visualization is derived from that data.

The Tower Defense Path Rerouting scenario demonstrates how a generic grid/pathfinding system can become a gameplay rule system: the player can reshape the route, but the system rejects edits that would break the core route requirement.

This makes the project useful as a foundation for future pathfinding, cost terrain, flow field, tactical movement, placement, or tower-defense systems.

## Current Limitations

- A* currently runs instantly in one frame.
- Open and closed search sets are not visualized yet.
- No per-tick frontier expansion yet.
- Weighted terrain scenario is not implemented yet.
- Tactical movement / ability preview scenario is not implemented yet.
- Debug drawing is used instead of final gameplay visuals.
- The project is currently a systems experiment, not a full playable game.

## Planned Next Steps

### Scenario 2: Weighted Terrain Movement

Add terrain movement costs and show how A* chooses cheaper routes.

Planned features:

- Terrain types such as road, grass, forest, and blocked cells.
- Different `MoveCost` values per terrain type.
- Movement range based on movement budget.
- Path preview that prefers lower-cost routes.

### Scenario 3: Tactical Movement and Ability Preview

Add a unit-based tactical movement scenario.

Planned features:

- Select a unit.
- Show reachable cells.
- Hover a destination cell to preview path.
- Click to move the unit along the path over time.
- Hover to preview ability range / affected cells.

### Per-Tick A* Frontier Expansion

Refactor one-shot A* into a multi-frame search process:

```text
StartPathfinding()
→ Initialize search state
→ StepPathfinding() each Tick
→ Visualize open / closed / final path
```

This will make the search process visible and easier to present in a portfolio GIF.

## Portfolio Positioning

This project is a UE5 C++ gameplay programming study focused on reusable grid-based systems.

It demonstrates:

- Native C++ gameplay logic
- Grid data modeling
- Input-to-world interaction
- Pathfinding fundamentals
- Runtime route validation
- Scenario-level gameplay rules
- Debug visualization
- System-oriented thinking

The project is intentionally scoped small so that the implementation remains readable, explainable, and easy to extend.
