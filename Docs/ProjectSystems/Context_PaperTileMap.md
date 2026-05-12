# Context: Paper TileMap Level Import

## Purpose

The Paper TileMap import system turns a Paper2D `UPaperTileMap` into a generated platformer `.umap`. It is editor tooling for fast level blockout: designers author tiles, assign TileSet UserData names, map those names to platformer actor classes, and generate a level under `/Game/PlatformerLevels`.

The importer is intentionally an editor-side generation pipeline. Do not put runtime gameplay rules in it; generated actors should own their runtime behavior after spawn.

## Ownership

Reusable editor tooling lives in `Plugins/CookieBrosPlatformer`:

- `Source/CookieBrosLevelEditor/Private/PaperTileMap/PaperTileMapLevelImporter.h`
- `Source/CookieBrosLevelEditor/Private/PaperTileMap/PaperTileMapLevelImporter.cpp`
- `Source/CookieBrosLevelEditor/Private/PaperTileMap/SPaperTileMapImportWidget.h`
- `Source/CookieBrosLevelEditor/Private/PaperTileMap/SPaperTileMapImportWidget.cpp`
- `Source/CookieBrosLevelEditor/Private/CookieBrosLevelEditor.cpp` registers the tab and command line entry point.
- `Source/CookieBrosPlatformer/Public/TileMap/TileSetAsset.h` defines the import mapping asset used by the importer.

The default mapping asset is:

- `/CookieBrosPlatformer/TilesData/DA_PlatformerTilesSet.DA_PlatformerTilesSet`

Generated content is project content:

- `/Game/PlatformerLevels/<TileMapName>`

## Entry Points

Editor UI:

1. `FCookieBrosLevelEditorModule` registers the `CookieBros.PaperTileMapImport` nomad tab.
2. `OpenPaperTileMapImportTab()` opens the tab.
3. `SpawnPaperTileMapImportTab()` creates `SPaperTileMapImportWidget`.
4. The widget gathers TileMap, import mapping, block size, optional GameMode override, and output level name.
5. `HandleGenerateLevelClicked()` calls `FCookieBrosPaperTileMapLevelImporter::GenerateLevelFromTileMapAsset()`.

Command line:

- `-CookieBrosImportPaperTileMap=<ObjectPath>`
- `-CookieBrosImportTileSet=<ObjectPath>`
- `-CookieBrosImportGameMode=<ClassObjectPath>`
- `-CookieBrosImportOutputLevel=<PackagePath>`

`FCookieBrosPaperTileMapLevelImporter::HasCommandLineRequest()` detects the command line flow, and `RunFromCommandLine()` executes it.

## Widget Settings

`SPaperTileMapImportWidget` stores the last editor choices in `GEditorPerProjectIni` under:

- `CookieBrosLevelEditor.PaperTileMapImport`
- `LastSelectedTileMap`
- `LastBlockSizeX`
- `LastBlockSizeY`
- `LastBlockSizeZ`
- `LastGameModeOverride`

The UI output path is always built as:

```text
/Game/PlatformerLevels/<SanitizedTileMapName>
```

The widget does not expose arbitrary output folders. Keep that constraint unless designers explicitly need multiple generated roots.

## Import Mapping

The importer reads the topmost rendered tile layer at each tile coordinate and resolves:

- `FPaperTileInfo`
- TileSet `UserDataName`
- source layer index
- ramp descriptor

`UTileSetAsset::FindRuleByUserDataName()` maps a `UserDataName` to an actor class. If a tile has no valid mapping, the importer falls back to `APlatformerBlock` and logs a warning.

Supported actor kinds are resolved from the mapped actor class:

- block / platformer block base
- half block
- soft platform
- spikes
- ramp
- ladder / ladder top
- player start
- moving platform
- triggered lift
- stream
- gravity volume
- camera volume
- switch
- generic actor fallback

Ramp metadata is parsed from tile UserData names shaped like:

```text
Ramp_<AngleDegrees>_<SegmentIndex>
```

`HalfBlockTop` and `HalfBlockBot` generate `APlatformerHalfBlock`. The importer also resolves these names as built-in rules so generated TileMaps still work when the mapping asset has not been updated yet.

`Ladder` and `LadderTop` TileSet names are normalized into a single generated `APlatformerLadderTop` actor per vertical ladder run. The `LadderTop` mapping rule is used when it points to an `APlatformerLadderTop` class; otherwise the importer falls back to native `APlatformerLadderTop`.

## Generation Flow

1. Validate the TileMap, import mapping asset, output package path, and orthogonal projection.
2. Load or create the target map.
3. Destroy actors tagged with `CookieBros.GeneratedFromPaperTileMap`.
4. Resolve cells from rendered layers.
5. Build prepared spawn records from tile positions and tile permutation flags.
6. Merge compatible adjacent cells:
   - horizontal runs for blocks, half blocks, soft platforms, moving platforms, triggered lifts
   - horizontal runs for spikes as one `APlatformerSpikes` actor with repeated mesh instances, not one stretched visual mesh
   - vertical runs for ladders, spawning only the run's `LadderTop` actor
   - rectangles for streams, gravity volumes, and camera volumes
7. Anchor the generated layout around the primary PlayerStart when present.
8. Spawn lighting/setup actor and generated actors.
9. Configure actor size/shape based on merged span and block size.
10. Tag generated actors and save the map.

## Coordinate And Size Rules

TileMap local X maps to world X. TileMap local Y is converted into world Z. World Y is depth.

Designer-facing block size comes from `FPaperTileMapGenerationSettings::BlockSize`:

- `BlockSize.X`: tile width in world X
- `BlockSize.Y`: interactive volume depth where relevant
- `BlockSize.Z`: tile height in world Z

Solid blocks and ramps use fixed depth:

```text
ImportedBlockDepth = 500
```

Ladders use `BlockSize.Y` as the rear climb volume offset. The generated `APlatformerLadderTop` ladder mesh and climb volume extend down across the merged vertical TileMap span. Streams, gravity volumes, and camera volumes use deep rectangular sizes.

## Actor Configuration

`ConfigureSpawnedActor()` applies generated size data after spawn:

- `APlatformerRamp::SetRampSize()` and `SetRampAngleDegrees()`
- `APlatformerBlockBase::SetBlockSize()`
- `APlatformerHalfBlock::SetPlatformSize()` and `SetHalfTop()`
- `APlatformerSoftPlatform::SetPlatformSize()`
- `APlatformerSpikes::SetSpikeSize()`
- `APlatformerTriggeredLift::SetPlatformSize()` and `SetTriggerSize()`
- `APlatformerMovingPlatform::SetPlatformSize()`
- `APlatformerStream::SetVolumeSize()`
- `APlatformerGravityVolume::SetVolumeSize()`
- `APlatformerCameraVolume::SetVolumeSizeNew()` and `SetVolumeSizeDefault()`
- `APlatformerLadder::SetLadderSize()`, climb volume offset, and snap-depth behavior
- `APlatformerLadderTop::SetTopBlockDepth()` and `SetTopSectionEnabled()`
- `APlatformerSwitch::SetTriggerExtent()`

Generic actor classes are spawned but not specially configured.

`APlatformerSpikes` is the exception to the old "scale one mesh to the merged span" mental model: generated horizontal spike runs still become one actor for clean ownership and a single damage volume, but the visual mesh is repeated internally as HISM instances across `SpikeSize.X` and `SpikeSize.Y`, similar to ladder mesh generation.

For a ladder run whose topmost TileMap cell is `LadderTop`, the top section stays enabled: the top platform mesh, drop-through checks, and top entry volume are active. For a run made only from regular `Ladder` cells, the generated actor still uses `APlatformerLadderTop`, but its top section is hidden and collision-disabled so it behaves like a plain ladder without a walkable cap.

## Regeneration Rules

Generated actors receive:

```text
CookieBros.GeneratedFromPaperTileMap
```

Before each generation pass, the target world destroys only actors with this tag. Hand-placed actors without the tag survive regeneration.

## Common Pitfalls

- The importer supports orthogonal Paper TileMaps only.
- TileSet UserData names are the contract. Changing names without updating `UTileSetAsset` rules changes generated actor classes.
- The generated root is fixed to `/Game/PlatformerLevels`.
- Do not hand-edit generated actors that still carry the generated tag unless those edits are disposable.
- Merged actors may not line up one-to-one with source tiles. Debug by checking prepared spawn merging first.
- PlayerStart anchoring changes level translation. If the whole generated level seems offset, inspect the first PlayerStart tile and its support tile.

## Quick Read Map For Agents

Start here:

- `SPaperTileMapImportWidget::HandleGenerateLevelClicked`
- `FCookieBrosPaperTileMapLevelImporter::GenerateLevelFromTileMapAsset`
- `TryGetResolvedTileCell`
- `TryResolveImportMappingSpawnRecipe`
- `BuildMergedPreparedSpawns`
- `ConfigureSpawnedActor`
- `DestroyPreviouslyGeneratedActors`
- `RunImportFromArgs`

The safest extension path is to add a new mapped actor kind only when it needs custom generation-time sizing or placement. If the actor can configure itself from defaults, let it remain a `GenericActor`.
