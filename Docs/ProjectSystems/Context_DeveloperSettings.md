# Context: Developer Settings / WBP_DeveloperSettings

## Purpose

`WBP_DeveloperSettings` is the runtime game-design tuning panel for fast iteration in PIE/builds. It lets a designer edit character, camera manager, combat, charge-shot, traversal, and common test settings, save them into named slots, load them later, and apply them immediately to the current runtime targets.

This system is intentionally developer/tooling infrastructure, not player progression. Do not mix it with `UDragonSlayerSaveGame` progression payloads.

## Ownership

Reusable foundation lives in `Plugins/CookieBrosPlatformer`:

- `Content/Blueprints/UI/WBP_DeveloperSettings.uasset`
- `Source/CookieBrosPlatformer/Public/UI/PlatformerDeveloperSettingsWidget.h`
- `Source/CookieBrosPlatformer/Private/UI/PlatformerDeveloperSettingsWidget.cpp`
- `Source/CookieBrosPlatformer/Public/Developer/DeveloperPlatformerSettingsTypes.h`
- `Source/CookieBrosPlatformer/Public/Core/PlatformerDeveloperSettingsSubsystem.h`
- `Source/CookieBrosPlatformer/Private/Core/PlatformerDeveloperSettingsSubsystem.cpp`
- `Source/CookieBrosPlatformer/Public/Core/SaveGame/SaveDeveloperSettings.h`
- `Source/CookieBrosPlatformer/Private/Core/SaveGame/SaveDeveloperSettings.cpp`
- `Source/CookieBrosPlatformer/Public/Core/SaveGame/SaveDeveloperSettingsSlotIndex.h`
- runtime apply/capture hooks in `APlatformerCharacterBase` and `APlatformerCameraManager`

DragonSlayer project glue lives in `Source/DragonSlayer`:

- `Source/DragonSlayer/UI/PauseMenu/DeveloperSettingsWidget.h` subclasses `UPlatformerDeveloperSettingsWidget` without extra logic.
- `Source/DragonSlayer/Core/UI/DragonSlayerHUD.cpp` spawns `/CookieBrosPlatformer/Blueprints/UI/WBP_DeveloperSettings`.
- `Source/DragonSlayer/Platformer/Base/PlatformerPlayerController.cpp` toggles the panel and reads `bAutoRestartLevel`.
- `Source/DragonSlayer/Platformer/Character/PlayableDragonCharacter.cpp` extends character apply/capture for Dragon production gameplay: charge-shot and traversal settings.

Rule of thumb: generic tunable platformer settings go to the plugin. Dragon-only settings may be captured/applied in `APlayableDragonCharacter` or project-specific components.

## Runtime Opening Flow

1. `APlatformerPlayerController` binds `IA_ToggleDeveloper` in `SetupInputComponent`.
2. `HandleDeveloperSettingsToggleRequested()` calls `ADragonSlayerHUD::ToggleDeveloperSettingsWidget()`.
3. `ADragonSlayerHUD` creates `UDeveloperSettingsWidget` from `/CookieBrosPlatformer/Blueprints/UI/WBP_DeveloperSettings`.
4. HUD adds the widget to viewport, switches input to `GameAndUI`, shows the cursor, and pauses the game.
5. Closing the widget restores `GameOnly`, hides the cursor, and unpauses.

## Main C++ Widget Class

`UPlatformerDeveloperSettingsWidget` owns the reusable widget behavior:

- `RefreshDeveloperSettingsWidget()`
- `LoadDeveloperSettingsSnapshotIntoWidgets()`
- `PatchWorkingCopyFromWidgets()`
- `ApplyDeveloperSettingsSnapshotToTargets()`
- button handlers for Save, Save As, Load, Delete, Close
- widget bindings such as `Movement_MaxWalkSpeed`, `Movement_ChangeDirectionSpeed`, `CameraManager_HOffset`, `Dash_DashSpeed`, etc.

Blueprint child widgets must use the exact C++ binding names. Optional new controls should usually use `BindWidgetOptional` unless the widget must not compile without them.

## Snapshot Data Model

The saved payload is `FPlatformerDeveloperSettingsSnapshot` in `DeveloperPlatformerSettingsTypes.h`.

It contains:

- `CharacterSettings` (`FDeveloperPlatformerCharacterSettings`)
- `CameraManagerSettings` (`FDeveloperPlatformerCameraManagerSettings`)
- `bHasSavedCameraManagerSettings`
- `bHasSavedCombatSettings`
- `bHasSavedChargeShotSettings`
- `bHasSavedJumpHorizontalSpeed`
- `bHasSavedCrouchCapsuleScale`
- `bHasSavedJumpTrajectoryPreview`
- `bHasSavedTraversalSettings`
- `bAutoRestartLevel`

`FDeveloperPlatformerCharacterSettings` groups:

- `DeveloperCameraSettings`
- `DeveloperCharacterMovementSettings`
- `DeveloperCombatSettings`
- `DeveloperChargeShotSettings`
- `DeveloperLedgeSettings`
- `DeveloperDashSettings`
- `DeveloperWallSettings`

`DeveloperLedgeSettings` is `FPlatformerLedgeTraversalSettings`. Its widget bindings live in the `Ledge_*` section of `UPlatformerDeveloperSettingsWidget`, including:

- `Ledge_DetectionDistance`
- `Ledge_MaxReachHeight`
- `Ledge_MinHangHeight`
- `Ledge_ForwardProbeRadius`
- `Ledge_TopSurfaceForwardOffset`
- `Ledge_TopPointHorizontalOffset`
- `Ledge_HangForwardOffset`
- `Ledge_HangVerticalOffset`
- `Ledge_ClimbSpeed`
- `Ledge_ForgivenessWindow`
- `Ledge_RegrabCooldown`

`Ledge_TopSurfaceForwardOffset` controls where the top-surface probe checks for a valid ledge top. `Ledge_TopPointHorizontalOffset` controls the character's X displacement from the hang point to the top/climb target after the ledge is found. Keep both in `FPlatformerLedgeTraversalSettings` with `SaveGame` so they persist through developer setting slots.

`FDeveloperPlatformerCharacterMovementSettings` currently includes standard `UCharacterMovementComponent` values plus plugin-side platformer values, including:

- `DeveloperMovementMaxWalkSpeed`
- `DeveloperMovementChangeDirectionSpeed`
- `DeveloperMovementMaxFlySpeed`
- `DeveloperMovementMaxAcceleration`
- `DeveloperMovementBrakingDecelerationWalking`
- `DeveloperMovementJumpZVelocity`
- `DeveloperMovementJumpApexGravityMultiplier`
- `DeveloperMovementJumpHorizontalSpeed`
- `DeveloperMovementCrouchCapsuleScale`
- `DeveloperMovementShowJumpTrajectory`
- `DeveloperMovementGravityScale`
- `DeveloperMovementMass`
- `DeveloperMovementBrakingFrictionFactor`
- `DeveloperMovementGroundFriction`
- `DeveloperMovementAirControl`

## Save/Load Storage

`UPlatformerDeveloperSettingsSubsystem` is a `UGameInstanceSubsystem` and owns named slots.

SaveGame slot names:

- slot index: `SaveDeveloperSettings_Index`
- legacy single slot: `SaveDeveloperSettings_1`
- per-slot payload: `SaveDeveloperSettings_Slot_<Guid>`
- user index: `0`

Objects:

- `USaveDeveloperSettingsSlotIndex` stores available slot descriptors and current slot id.
- `USaveDeveloperSettings` stores `DataVersion`, `SlotId`, and `FPlatformerDeveloperSettingsSnapshot`.
- `USaveDeveloperSettings::ResolveSnapshot()` preserves migration from legacy fields.

Subsystem operations:

- `TryLoadCurrentSnapshot()`
- `SaveCurrent()`
- `SaveAs()`
- `LoadSlot()`
- `DeleteSlot()`
- `MigrateLegacyDeveloperSettingsIfNeeded()`

## Widget Save/Load Flow

On construct:

1. `RefreshDeveloperSettingsWidget()` asks the subsystem for the current saved snapshot.
2. If a current slot exists, the widget uses that snapshot.
3. If not, it calls `CaptureDeveloperSettingsSnapshotFromRuntime()`.
4. `LoadDeveloperSettingsSnapshotIntoWidgets()` pushes values into the visual controls.

Save:

1. `HandleDeveloperSaveClicked()`.
2. `PatchWorkingCopyFromWidgets()` reads all editable widget values into `WorkingCopy`.
3. `UPlatformerDeveloperSettingsSubsystem::SaveCurrent(WorkingCopy)`.
4. `ApplyDeveloperSettingsSnapshotToTargets(WorkingCopy)` applies runtime changes immediately.

Save As:

1. Same patch step.
2. `SaveAs(RequestedSlotName, WorkingCopy, SavedSlot)`.
3. The new/updated slot becomes current.
4. Runtime targets are updated immediately.

Load:

1. `HandleDeveloperLoadClicked()` resolves selected slot by display name.
2. `LoadSlot(SlotId, LoadedSnapshot)` loads and marks it current.
3. The widget applies the snapshot to runtime targets and refreshes controls.

Delete:

1. Deletes selected slot payload.
2. If the deleted slot was current, the widget falls back to runtime-captured values.
3. Slot UI refreshes.

## Runtime Apply Targets

`UPlatformerDeveloperSettingsWidget::ApplyDeveloperSettingsSnapshotToTargets()` applies to:

- current pawn cast to `APlatformerCharacterBase`
- current `PlayerCameraManager` cast to `APlatformerCameraManager`

Character apply:

- `APlatformerCharacterBase::ApplyDeveloperSettingsSnapshot()` applies camera settings, movement settings, and combat settings.
- It clears unsupported legacy/missing fields using the boolean `bHasSaved...` flags.
- `APlayableDragonCharacter::ApplyDeveloperSettingsSnapshot()` calls `Super`, then applies project-specific charge-shot and traversal settings only when the matching saved flags are present.

Camera manager apply:

- `APlatformerCameraManager::ApplyDeveloperCameraManagerSettings()` applies projection, ortho width, idle threshold, look-ahead offsets, interpolation speeds, dead zone, bound box, and crouch smoothing.
- The camera manager converts designer-facing `DeveloperCameraManagerIdleSpeedThreshold` from m/s to cm/s by multiplying by `100`.

Camera volumes:

- `APlatformerCameraVolume` uses `FPlatformerCameraVolumeSettings` for placed level camera overrides.
- The volume settings include camera rig values (`CameraLocation`, `CameraRotation`, `SpringArmLocation`, `SpringArmRotation`, `SpringArmLength`) and camera manager values with designer-facing names (`HorizontalOffset`, `DeadZoneWidth`, etc.).
- Entering `NewCameraVolume` blends the current `APlatformerCameraManager` settings to the actor's target settings over `BlendTime` using EaseInEaseOut.
- Entering `DefaultCameraVolume` blends back to the camera manager settings captured before the first override.
- This is transient runtime level behavior. It must not save into `UPlatformerDeveloperSettingsSubsystem` slots.

## Runtime Auto-Load

Character:

- `APlatformerCharacterBase::BeginPlay()` calls `LoadAndApplyDeveloperSettings()`.
- It loads the current developer snapshot and applies it to the character.
- This means saved movement/combat/camera-rig tuning is restored when a character spawns.

Camera manager:

- `APlatformerCameraManager::BeginPlay()` calls `LoadAndApplyDeveloperCameraManagerSettings()`.
- It applies `CameraManagerSettings` only if `bHasSavedCameraManagerSettings` is true.

Auto restart:

- `APlatformerPlayerController::ShouldAutoRestartLevelAfterDefeat()` reads the current snapshot and returns `bAutoRestartLevel`.

## Character Settings Apply Details

`APlatformerCharacterBase::ApplyDeveloperCharacterMovementSettings()` writes to:

- `UCharacterMovementComponent::MaxWalkSpeed`
- `MaxFlySpeed`
- `MaxAcceleration`
- `BrakingDecelerationWalking`
- `JumpZVelocity`
- `Mass`
- `BrakingFrictionFactor`
- `GroundFriction`
- `AirControl`

If the movement component is `USideViewMovementComponent`, it also applies:

- `JumpApexGravityMultiplier`
- `DeveloperMovementChangeDirectionSpeed` through `SetChangeDirectionSpeed()`
- base gravity through `SetBaseGravityScale()`

If not side-view, it falls back to `MovementComponent->GravityScale`.

Additional character values:

- crouch capsule scale is handled through `SetDeveloperCrouchCapsuleScaleOverride()`
- jump horizontal speed through `SetDeveloperJumpHorizontalSpeedOverride()`
- trajectory visibility through `SetShowJumpTrajectoryPreview()`

`DeveloperMovementChangeDirectionSpeed` is designer-facing speed in degrees per second. `USideViewMovementComponent` converts it into the existing `TurnAroundSeconds` value:

```text
TurnAroundSeconds = 180 / DeveloperMovementChangeDirectionSpeed
```

Default `1200 deg/s` equals a 180-degree turn in `0.15s`.

## Camera Rig vs Camera Manager

There are two camera-related setting groups:

1. Character camera rig settings in `FDeveloperPlatformerCameraSettings`
   - spring arm length
   - spring arm location/rotation
   - camera FOV
   - camera relative location/rotation
   - applied by `APlatformerCharacterBase`
   - spring arm location remains character-local, so X offsets can bias the view toward the character's facing direction

2. Camera manager settings in `FDeveloperPlatformerCameraManagerSettings`
   - projection mode
   - ortho width
   - idle speed threshold
   - horizontal/vertical look-ahead offsets
   - interpolation speeds
   - dead zone and bound box
   - crouch interpolation speed
   - applied by `APlatformerCameraManager`
   - smooths the world-space camera rig offset from the character rig across X/Y/Z, including the depth shift produced while a local spring-arm X offset rotates during turnarounds; X/Y rig offset smoothing uses `HorizontalOffsetInterpSpeedStart`

Do not put camera manager follow behavior into character settings unless it is truly part of the character rig.

## Adding A New Parameter

Use this checklist when extending `WBP_DeveloperSettings`.

1. Add a `UPROPERTY(..., SaveGame)` field to the correct settings struct in `DeveloperPlatformerSettingsTypes.h`.
2. Add a matching widget pointer to `UPlatformerDeveloperSettingsWidget` with the exact Blueprint widget name.
3. Load it in `LoadDeveloper...SettingsIntoWidgets()`.
4. Read it in `BuildDeveloper...SettingsFromWidgets()`.
5. Apply it to runtime in the correct target:
   - character base for reusable character/movement/combat/camera-rig settings
   - camera manager for follow/projection settings
   - `APlayableDragonCharacter` or Dragon components only for Dragon-specific settings
6. Capture it from runtime in the matching `CaptureDeveloper...Settings()` method.
7. If the field is optional or added after old saves exist, decide whether a `bHasSaved...` flag/fallback is needed.
8. Rebuild the project and test Save, Save As, Load, Delete, close/reopen, and new level/respawn restoration.

## Common Pitfalls

- Do not put core gameplay rules in the widget. The widget only edits data and sends snapshots to runtime targets.
- Do not create Dragon-specific fields in plugin structs unless the setting is reusable.
- Do not bypass `FPlatformerDeveloperSettingsSnapshot`; save/load should go through the subsystem.
- Keep Blueprint widget names identical to C++ binding names.
- If a field was added after existing saves, old slots may contain default struct values. Use `bHasSaved...` fallback flags when default values would be misleading or destructive.
- Character and camera manager are applied separately; changing one does not automatically update the other unless `ApplyDeveloperSettingsSnapshotToTargets()` reaches both targets.

## Quick Read Map For Agents

Start here:

- `UPlatformerDeveloperSettingsWidget::RefreshDeveloperSettingsWidget`
- `UPlatformerDeveloperSettingsWidget::PatchWorkingCopyFromWidgets`
- `UPlatformerDeveloperSettingsWidget::ApplyDeveloperSettingsSnapshotToTargets`
- `UPlatformerDeveloperSettingsSubsystem::SaveAs`
- `UPlatformerDeveloperSettingsSubsystem::TryLoadCurrentSnapshot`
- `APlatformerCharacterBase::ApplyDeveloperSettingsSnapshot`
- `APlatformerCharacterBase::ApplyDeveloperCharacterMovementSettings`
- `APlatformerCameraManager::ApplyDeveloperCameraManagerSettings`
- `APlayableDragonCharacter::ApplyDeveloperSettingsSnapshot`

The safest extension path is to mirror an existing nearby parameter such as `Movement_MaxWalkSpeed`, `Movement_ChangeDirectionSpeed`, `CameraManager_HOffset`, or `Dash_DashSpeed`.
