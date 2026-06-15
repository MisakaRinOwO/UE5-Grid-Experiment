# GridExperiment — UE5 C++ Grid Systems

A UE5 C++ project for learning and experimenting with grid-based algorithms. Built as a deliberate practice project to develop UE5 C++ workflow skills alongside algorithm implementation.

## Current State

### GridManager (AGridManager)
- `FGridCoord` struct: 2D integer coordinate
- `FGridCell` struct: coord + blocked flag + move cost
- `TArray<FGridCell>` grid storage, initialized from `GridWidth × GridHeight`
- `WorldToGrid`: world position → grid coord (floor division, actor-relative)
- `GridToWorld`: grid coord → world center position
- `CoordToIndex`: 2D coord → flat array index
- `IsValidCoord`: bounds check
- Per-tick debug draw: `DrawDebugBox` per cell, red if blocked, green if passable
- All properties exposed via `UPROPERTY` / `UFUNCTION` for Blueprint access

## Planned

- [ ] A* pathfinding with per-tick frontier visualization
- [ ] Obstacle painting (click to toggle cell blocked state)
- [ ] Flow field generation
- [ ] Path cost visualization

## Stack

- Unreal Engine 5
- C++
- DrawDebugHelpers for visualization
