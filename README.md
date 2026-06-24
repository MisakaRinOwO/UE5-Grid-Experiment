# Grid Systems Experiment

A focused Unreal Engine 5 C++ gameplay-systems experiment for testing reusable grid logic across multiple grid-based scenarios.

The project is not a full game. It is a small, inspectable systems prototype built around one reusable C++ grid core (`AGridManager`) and several scenario-specific Blueprint PlayerControllers. The goal is to prove that the same native grid/pathfinding layer can support tactical movement, weighted terrain movement, and tower-defense-style placement validation without rewriting the core system.

## Current Project Status

Implemented scenarios:

- **Tactical Movement & Ability Preview** - unit placement, selection, movement budget, path-following movement, occupancy update, and skill attack/effect preview.
- **Weighted Terrain Movement** - runtime terrain cost editing, movement budget slider, reachable-cell visualization, and cost-aware hover path preview.
- **Tower Defense Path Rerouting** - obstacle editing with immediate route recalculation and rollback when a placement fully blocks the path.

Core systems implemented in C++:

- Data-driven grid stored as `TArray<FGridCell>` instead of actor-per-cell.
- Logical/world coordinate conversion through `GridToWorld()` and `WorldToGrid()`.
- Cost-aware A* pathfinding with 4-direction and optional 8-direction movement.
- Dijkstra-style movement-budget expansion through `FindReachableCells()`.
- Occupancy tracking through `bIsOccupied` and `OccupyingCharacter` on `FGridCell`.
- Ability preview helpers through `FindCellsInRange()` and `UpdateAbilityPreview()`.
- Runtime terrain cost editing through an editor-configurable `MoveCostList`.
- Debug-first visualization with `DrawDebugBox`, `DrawDebugLine`, and `DrawDebugString`.

## Architecture

The project is structured in three layers:

```text
Enhanced Input / Pawn
-> PlayerController  (scenario rule layer - Blueprint)
-> AGridManager      (C++ core - data, pathfinding, queries, visualization)
```

Each level has a lightweight GameMode that binds the level-specific Pawn and PlayerController. Gameplay rules live in the PlayerController for that scenario; reusable grid state and algorithms live in `AGridManager`.

This keeps scenario rules from bleeding into the reusable C++ core. For example, Tower Defense rollback logic belongs to the Tower Defense PlayerController, while pathfinding and grid mutation functions remain reusable by the Weighted Terrain and Tactical Movement scenarios.

## Core C++ Grid Model

The grid uses logical coordinates and per-cell data structs:

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    bool bIsOccupied = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid")
    TObjectPtr<ACharacter> OccupyingCharacter = nullptr;
};
```

Cells are stored in a one-dimensional array:

```text
Index = Y * GridWidth + X
```

World-space interaction is derived from this logical grid:

- `GridToWorld()` converts a grid coordinate into a world-space cell center.
- `WorldToGrid()` converts a raytrace hit location into a grid coordinate.
- PlayerController Blueprints perform mouse raytrace, validate the hit, then call `AGridManager` functions with logical coordinates.

## Pathfinding

`FindPath()` implements synchronous A* in C++.

Pathfinding rules:

- 4-direction movement by default.
- Optional 8-direction movement through `bEnable8Directional`.
- 4-direction mode uses Manhattan distance scaled by minimum move cost.
- 8-direction mode uses Chebyshev distance scaled by minimum move cost.
- A path step pays the destination cell's `MoveCost`.
- Blocked cells are ignored.
- Occupied cells are rejected as movement goals.

Cost update:

```cpp
const float NewGCost =
    PathNodes[CurrentIndex].GCost + Cells[NeighborIndex].MoveCost;
```

8-direction movement uses a soft corner-cutting rule: a diagonal move is allowed if at least one adjacent cardinal cell is walkable. This fits the prototype's abstract terrain-cost model better than treating the grid as strict physical collision.

## Movement Budget Preview

Movement-range preview is separate from single-target pathfinding.

`FindReachableCells()` runs a Dijkstra-style cost expansion from a start cell:

```text
Start cost = 0
Expand the open cell with the lowest accumulated cost
NewCost = CurrentCost + Neighbor.MoveCost
If NewCost <= MovementBudget, the neighbor is reachable
Blocked cells are ignored
```

The result is stored in:

```cpp
TArray<FGridCoord> ReachableCells;
```

Hover preview then uses `IsCoordInReachableCells()` as a gate. If the hovered cell is reachable, `FindPath()` resolves the displayed cost-aware route and `GetPathCost()` reports its total movement cost.

## Ability Preview

Ability preview is independent from movement pathfinding state.

`UpdateAbilityPreview(OriginCoord, TargetCoord, AttackRange, EffectRange)` fills two separate arrays:

- `AbilityAttackRangeCells` - cells the selected unit can target.
- `AbilityEffectCells` - cells affected around the hovered target.

The current implementation uses `FindCellsInRange()` plus `IsCoordInRange()` to collect cells inside an unweighted grid radius. Terrain `MoveCost` does not affect skill range. If the hovered target is outside attack range, effect cells are suppressed.

This keeps movement preview (`StartCoord`, `GoalCoord`, `CurrentPath`) and ability preview (`AbilityOriginCoord`, `AbilityTargetCoord`, range/effect arrays) in separate state blocks.

## Scenario: Tactical Movement & Ability Preview

This scenario exercises the full grid core in one level.

Implemented:

- Terrain Editor mode for terrain cost editing and unit placement/removal.
- Placement of multiple Mage units on the grid.
- Unit selection by grid cell.
- Unit Data Asset driven movement budget.
- Skill Data Asset driven attack/effect range values.
- Movement range visualization.
- Cost-aware hover path preview.
- Step-by-step path-following movement over time.
- Occupancy update when movement finishes.
- Interaction lock while a unit is moving.
- Animation Blueprint idle/jog state driven by movement state.
- Skill attack/effect preview independent from movement preview.

Current limitation:

- Scout and Ranged unit presets exist as Data Assets/schema examples, but the in-game placement picker is not implemented yet. The current placement flow instantiates the Mage preset.

## Scenario: Weighted Terrain Movement

This scenario isolates terrain-cost editing and movement-budget path preview.

Implemented:

- Dedicated Weighted Terrain PlayerController.
- Tab-based mode switching.
- Terrain Editor mode for cycling cell costs.
- Movement Range mode for selecting a start cell and previewing movement.
- Editor-configurable `MoveCostList`.
- Default cost cycle: `1 -> 2 -> 3 -> Blocked -> 1`.
- Movement budget slider.
- Dijkstra-style reachable-cell calculation under the current movement budget.
- Cyan reachable-cell visualization.
- Hover path preview gated to reachable cells.
- Cost-aware A* route display and path-cost reporting.
- Optional 8-direction pathfinding with Chebyshev heuristic.

Important distinction:

- The reachable set is produced by the movement-budget expansion.
- The visible route to a hovered reachable cell is resolved by cost-aware A*.

## Scenario: Tower Defense Path Rerouting

This scenario tests pathfinding as a placement rule.

Implemented:

- Dedicated Tower Defense PlayerController.
- Start and goal cell selection.
- Obstacle toggle interaction.
- 4-direction A* pathfinding.
- Path recalculation after each obstacle edit.
- Rollback when an obstacle placement fully blocks the route.
- `PendingToggleObstacle` state so rollback targets the correct cell.

Rule flow:

```text
Player toggles obstacle
-> PlayerController stores pending coord
-> Run FindPath()
-> If path exists: keep edit
-> If path is blocked: toggle obstacle again to rollback
-> Re-run FindPath() to confirm restored route
```

## Scenario Controls

### Tower Defense Path Rerouting

| Input | Action |
|---|---|
| Left Mouse Button | Toggle obstacle on hovered cell |
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

### Tactical Movement & Ability Preview

The Tactical level is mode-driven. The exact bindings are implemented in the Tactical PlayerController Blueprint and in-level UI, but the scenario flow is:

```text
Terrain Editor -> edit terrain / place or remove Mage units
Movement Range -> select unit / preview reachable cells / move along path
Skill Attack   -> preview attack range and effect cells from selected unit skill data
```

## Debug Visualization

Debug colors used by the grid:

| Color | Meaning |
|---|---|
| Green | Walkable cell / low-cost terrain |
| Red | Obstacle / blocked cell |
| Yellow | Start, selected unit, or ability origin |
| Purple | Goal, hovered target, or ability target |
| Cyan | Reachable cell under current movement budget |
| Blue line | Current A* route |
| Silver | Ability attack range |
| Magenta | Ability effect area |
| White text | Cell movement cost |
| X | Blocked cell |

The project intentionally uses debug drawing instead of final art so the system state stays easy to inspect while iterating.

## How to Open

1. Open `GridExperiment.uproject` in Unreal Engine 5.
2. If prompted, rebuild project modules.
3. Open one of the scenario levels from the Unreal Editor.
4. Press Play and use the in-level UI/controls for the selected scenario.

If Visual Studio project files are missing or stale, regenerate project files from the `.uproject` context menu.

## Current Limitations

- This is a systems prototype, not a complete game.
- Debug visualization is used instead of production meshes/materials.
- A* currently runs synchronously in one frame.
- Open/closed pathfinding frontier visualization is not implemented.
- Tactical placement currently instantiates Mage units only; Scout/Ranged runtime selection is not implemented.
- Ability range currently uses an unweighted grid-radius helper, independent from terrain cost.

## Portfolio Case Study

A recruiter-facing breakdown of this project is available in the portfolio site:

<https://misakarinowo.github.io/Portfolio/projects/grid-systems-experiments/>

The portfolio version focuses on design rationale and presentation. This README focuses on the repo's current implementation state.
