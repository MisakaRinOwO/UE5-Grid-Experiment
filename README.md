# Grid Systems Experiment

A focused Unreal Engine 5 C++ gameplay systems experiment for testing reusable grid logic across multiple gameplay scenarios.

The project started as a UE5 C++ learning exercise and has grown into a small grid-framework prototype: one shared `AGridManager` core provides grid data, coordinate conversion, obstacle state, terrain movement cost, reachable-cell queries, and A* pathfinding. Each level uses a different Player Controller / scenario layer to test a different gameplay use case.

## Project Goals

The goal of this experiment is not to build a full game. The goal is to prove a small, explainable gameplay system that can support different grid-based rules.

Core goals:

- Store grid data in C++ instead of spawning one Actor per cell.
- Convert world-space cursor interaction into logical grid coordinates.
- Edit cell walkability and movement cost through player input.
- Set pathfinding start and goal cells.
- Run cost-aware A* pathfinding on the grid.
- Calculate reachable cells under a movement budget.
- Visualize grid state, terrain cost, obstacles, reachable cells, and path results with debug drawing.
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
- Cell movement cost displayed with debug text.
- Path rendered as connected debug lines between path cell centers.
- Event-driven input through Enhanced Input / IMC.
- Top-down pawn with WASD movement.
- Mouse cursor raytrace using screen deprojection and line trace.
- Cursor hit location converted into grid coordinate.
- Obstacle toggle interaction.
- Start cell selection.
- Goal cell selection.
- One-shot cost-aware A* pathfinding.
- Optional 8-direction movement support.
- Reachable-cell range calculation under a movement budget.
- Scenario-level rule logic built on top of the shared C++ grid core.

### Scenario 1: Tower Defense Path Rerouting

Completed.

This level demonstrates dynamic path rerouting on a shared C++ grid system. The player can set a start cell, set a goal cell, and toggle obstacle cells. Whenever the grid changes, the scenario recalculates the route. If a new obstacle would completely block the path from start to goal, the placement is rejected and the obstacle change is rolled back.

Implemented:

- Dedicated Tower Defense Player Controller scenario layer.
- Fixed route visualization between start and goal cells.
- Automatic path recalculation after grid edits.
- Obstacle placement validation.
- Invalid blockage rollback when a placement would fully block the route.
- UI guide for controls, color meaning, and scenario purpose.
- Blue route line drawn from `CurrentPath[i]` to `CurrentPath[i + 1]`.

### Scenario 2: Weighted Terrain Movement

Completed.

This level demonstrates terrain movement cost, movement budget, reachable-cell range, and hover path preview on the same shared C++ grid core. The player can edit terrain costs at runtime, adjust movement budget through a UI slider, select a start cell, view all reachable cells, and hover reachable targets to preview the cost-aware route.

Implemented:

- Dedicated Weighted Terrain Player Controller scenario layer.
- Tab-based mode switching.
- Terrain Editor mode.
- Movement Range mode.
- Mode-specific LMB behavior.
- Editor-editable movement cost list through `MoveCostList`.
- Blocked terrain state after the final cost-list entry.
- Blocked cell reset back to cost index 0 when clicked again.
- Movement budget slider UI.
- Reachable-cell range calculation using Dijkstra-style cost expansion.
- Cyan reachable-cell debug visualization.
- Current terrain cost text drawn at the center of each cell.
- Hover path preview in Movement Range mode.
- Current path cost display.
- Optional 8-direction pathfinding support.
- Chebyshev heuristic for equal-cost diagonal/cardinal movement.
- Soft corner-cutting rule for diagonal movement.

## Scenario Controls

### Tower Defense Path Rerouting

| Input | Action |
|---|---|
| Left Mouse Button | Toggle obstacle on the hovered cell |
| 1 | Set hovered cell as start |
| 2 | Set hovered cell as goal |
| WASD | Move top-down pawn |

### Weighted Terrain Movement

| Input | Terrain Editor Mode | Movement Range Mode |
|---|---|---|
| Tab | Switch to Movement Range mode | Switch to Terrain Editor mode |
| Left Mouse Button | Cycle terrain movement cost / blocked state | Set start cell |
| Mouse Hover | Inspect terrain cost | Preview path if hovered cell is reachable |
| Movement Budget Slider | Adjust movement budget | Adjust movement budget |
| WASD | Move top-down pawn | Move top-down pawn |

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    int32 MoveCostListIndex = 0;
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

This allows player input, raytrace hits, pathfinding, movement budget checks, and scenario rules to operate on the same grid data model.

### Event-Driven Input

Input is handled outside the `GridManager`.

The Player Controller / Pawn Blueprint owns Enhanced Input events and cursor raytracing. The `GridManager` owns the reusable grid data, movement rules, and pathfinding logic.

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

- 4-direction movement by default.
- Optional 8-direction movement for weighted terrain preview.
- Blocked cells are ignored.
- 4-direction mode uses Manhattan-style heuristic.
- 8-direction mode uses Chebyshev-style heuristic because diagonal and cardinal movement currently use the destination cell's `MoveCost` without an extra geometric multiplier.
- Cell movement cost is accumulated through `MoveCost`.

Cost-aware path update:

```cpp
NewGCost = PathNodes[CurrentIndex].GCost + Cells[NeighborIndex].MoveCost;
```

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

## Movement Budget and Reachable Cells

Weighted terrain movement uses a Dijkstra-style expansion rather than normal BFS because terrain costs are not uniform.

The core rule:

```text
Start cost = 0
Expand the open cell with the lowest known accumulated cost
NewCost = CurrentCost + Neighbor.MoveCost
If NewCost <= MovementBudget, the neighbor is reachable
Blocked cells are ignored
```

Implemented functions include:

- `FindReachableCells`
- `IsCoordInReachableCells`
- `ClearReachableCells`

The reachable range is stored as:

```cpp
TArray<FGridCoord> ReachableCells;
```

This lets the Weighted Terrain level show all cells reachable under the current movement budget. The Tactical Movement scenario can later reuse the same system for unit movement range.

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
→ Tracks walkability and movement cost
→ Runs A*
→ Calculates reachable cells
→ Draws debug grid and route
```

This separation keeps the grid system reusable for future scenarios.

## Scenario 2: Weighted Terrain Movement

This scenario tests whether the shared grid core can support cost-based movement rules.

### Gameplay Rule

The player can edit terrain cost and then test how movement budget affects reachable range and path choice.

Current rule flow:

```text
Terrain Editor mode
→ LMB cycles selected cell through MoveCostList values
→ Last cost entry turns into blocked
→ Blocked cell clicked again resets to cost index 0

Movement Range mode
→ LMB sets start cell
→ Reachable cells update from MovementBudget
→ Hover reachable target
→ Cost-aware path preview appears
→ Current path cost is displayed
```

### Terrain Cost Cycle

Default example:

```text
1 → 2 → 3 → Blocked → 1
```

The cost list is editor-editable, so designers can test different terrain-cost sets without changing code.

### Scenario Architecture

The scenario logic is handled in a dedicated Player Controller for this level.

```text
PC_WeightedTerrainMovement
→ Handles mode switching
→ Owns UI slider interaction
→ Runs cursor raytrace
→ Calls GridManager terrain / movement functions
→ Enables hover path preview only in Movement Range mode
```

`AGridManager` remains the reusable core:

```text
AGridManager
→ Stores cost values
→ Cycles terrain cost
→ Calculates reachable cells
→ Runs cost-aware A*
→ Stores CurrentPathCost
→ Draws terrain cost text, reachable cells, and route line
```

## Debug Visualization

Current debug colors:

| Color | Meaning |
|---|---|
| Green | Walkable cell / low-cost terrain |
| Red | Obstacle / blocked cell |
| Yellow | Start cell |
| Purple | Goal / hovered target cell |
| Cyan | Reachable cell under current movement budget |
| Blue line | Current route |
| White text | Cell movement cost |
| X | Blocked cell |

The grid is rendered using `DrawDebugBox`, cost text is rendered using `DrawDebugString`, and routes are rendered as connected `DrawDebugLine` segments. This makes the system easy to inspect without custom meshes or materials.

## Technical Highlights

- UE5 C++ Actor created from scratch.
- C++ `USTRUCT` data model for grid coordinates and cells.
- `UPROPERTY` / `UFUNCTION` exposure for Blueprint and Editor interaction.
- One-dimensional array indexing for 2D grid data.
- Event-driven Enhanced Input workflow.
- Mouse cursor raytrace to logical grid mapping.
- C++ A* pathfinding implementation.
- Cost-aware route calculation.
- Dijkstra-style reachable range calculation.
- Runtime path validation.
- Invalid placement rollback.
- Editor-editable terrain cost list.
- Optional 8-direction movement.
- Hover path preview.
- Debug-first visualization approach.
- Clear separation between input layer, scenario rule layer, and grid/pathfinding core.

## Why This Matters

This project focuses on a common gameplay programming problem: representing, editing, and querying grid data efficiently.

The key design decision is to avoid actor-based grid cells. Instead, the grid exists as data, and the world visualization is derived from that data.

The Tower Defense Path Rerouting scenario demonstrates how a generic grid/pathfinding system can become a placement-rule system: the player can reshape the route, but the system rejects edits that would break the core route requirement.

The Weighted Terrain Movement scenario demonstrates how the same grid core can become a movement-rule system: terrain cost and movement budget determine which cells are reachable and which route is cheapest.

This makes the project useful as a foundation for future pathfinding, cost terrain, flow field, tactical movement, placement, or tower-defense systems.

## Current Limitations

- A* currently runs instantly in one frame.
- Open and closed search sets are not visualized yet.
- No per-tick frontier expansion yet.
- Tactical movement / ability preview scenario is not implemented yet.
- Debug drawing is used instead of final gameplay visuals.
- The project is currently a systems experiment, not a full playable game.

## Planned Next Steps

### Scenario 3: Tactical Movement and Ability Preview

Add a unit-based tactical movement scenario.

Planned features:

- Select a unit.
- Show reachable cells using the existing movement budget system.
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
- Movement-budget rules
- Scenario-level gameplay rules
- Debug visualization
- System-oriented thinking

The project is intentionally scoped small so that the implementation remains readable, explainable, and easy to extend.
