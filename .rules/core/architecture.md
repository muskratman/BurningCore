# Architecture: DragonSlayer

## Project Shape

DragonSlayer — 3D side-scrolling action platformer на Unreal Engine 5.6.
Архітектура двошарова:

- `Plugins/CookieBrosPlatformer` — reusable platformer foundation
- `Source/DragonSlayer` — project-specific gameplay, progression, UI і Dragon контент

Production path проходить через reusable shell у плагіні та конкретну реалізацію в модулі `DragonSlayer`.

## Runtime Structure

```
DragonSlayer.uproject
├── EngineAssociation = 5.6
├── Runtime module: Source/DragonSlayer/
└── Plugin: Plugins/CookieBrosPlatformer/
    ├── CookieBrosPlatformer      ← reusable runtime mechanics
    └── CookieBrosLevelEditor     ← editor/import tooling
```

## Ownership Split

| Зона | Що тут живе |
|---|---|
| **Plugins/CookieBrosPlatformer** | Generic platformer classes: `APlatformerGameModeBase`, `APlatformerPlayerControllerBase`, `APlatformerCharacterBase`, `APlatformerEnemyBase`, traversal, environment actors, camera manager, save shell, developer widgets, interfaces |
| **Source/DragonSlayer/Core** | Project-local framework glue: `UDragonSlayerGameInstance`, main menu flow, HUD, developer settings |
| **Source/DragonSlayer/Character** | `ADragonCharacter`, forms, overdrive, hero-specific components |
| **Source/DragonSlayer/GAS + Data** | Dragon-specific abilities, attributes, effects, data assets |
| **Source/DragonSlayer/AI + Projectiles** | Concrete enemies, bosses, projectiles built on platformer shells |
| **Source/DragonSlayer/Systems** | `UDragonSlayerSaveGame`, project-specific checkpoint presentation subclasses, progression state |
| **Source/DragonSlayer/Platformer** | Production glue for current game flow: `APlatformerGameMode`, `APlatformerPlayerController`, `APlayableDragonCharacter` |
| **Source/DragonSlayer/UI + Core/UI** | Game-specific runtime HUD, pause menu, main menu, widgets derived from reusable UI shells |

## Key Decisions

- **Reusable platformer foundation у плагіні** — будь-яка механіка без DragonSlayer-специфіки має жити в `CookieBrosPlatformer`. ЧОМУ: це база для платформерів, а не тільки для цього проекту.
- **Project-specific logic у `Source/DragonSlayer`** — усе, що знає про Dragon, Overdrive, форми, конкретну прогресію, HUD або проектні data assets, залишається в модулі проекту. ЧОМУ: не засмічує reusable layer.
- **Inheritance поверх плагіна** — проект розширює shell-класи замість копіювання: `ADragonCharacter : APlatformerCharacterBase`, `UDragonSlayerSaveGame : UPlatformerSaveGame`, `AEnemyMelee : APlatformerEnemyMelee`. ЧОМУ: мінімум дублювання.
- **Interfaces + GAS + DataAssets** — зв'язки між combat, AI, projectiles і environment проходять через інтерфейси та data-driven конфігурацію. ЧОМУ: слабка зв'язність і простіше тюнити.
- **StateTree для AI** — базовий підхід для проекту на UE 5.6. ЧОМУ: узгоджено з поточним стеком.
- **C++ base + Blueprint derived** — системна логіка в C++, ассети і presentation у BP/UMG. ЧОМУ: зручно для агента і production pipeline.

## Placement Rule

Перед додаванням нового коду завжди перевір:

1. Якщо це generic platformer mechanic, reusable widget, save shell, traversal, environment actor або camera/controller shell → `Plugins/CookieBrosPlatformer`.
2. Якщо це DragonSlayer-specific hero logic, enemy flavour, progression, HUD, menu, save payload або сюжетний/контентний glue → `Source/DragonSlayer`.
3. Якщо зміна зачіпає обидві сторони одночасно, рішення належить Architect.

## Reusable Environment Helpers

- Shared editor/runtime helpers for generic platformer actors belong in `Plugins/CookieBrosPlatformer`.
- `UPlatformerPathComponent` is the reusable owner for local-space path points, per-point delay/speed data, repeat mode, and preview settings. Enemy patrol, moving platform / triggered lift / closing door routes, and camera volume point pairs should reuse it instead of adding actor-local path state.
- Debug viewport drawing for `UPlatformerPathComponent` should stay inside that single component through an editor-only debug render proxy. Do not add spline mesh, sphere, or text child components to runtime environment actors solely for path preview.
- If a platformer actor moves between path points, runtime movement should read from `UPlatformerPathComponent::PathPoints` directly. Do not add separate `PointA` / `PointB`, per-point delay, per-segment speed, or repeat properties on the actor unless they carry non-debug gameplay behavior that path points cannot express.

## Camera Rig

- `APlatformerCharacterBase` owns the reusable character-local camera rig. `USpringArmComponent` location offsets may be facing-relative, so `SpringArmLocation.X` can bias the view toward the character's facing direction.
- `APlatformerCameraManager` must smooth the resulting world-space camera rig offset across X/Y/Z. X/Y rig offset smoothing uses `HorizontalOffsetInterpSpeedStart`. Do not hard-assign the depth component during turnarounds, because local spring-arm X offsets rotate through world Y while the character changes yaw.
- Follow behavior, horizontal/vertical look-ahead, dead zone, bound box, projection, and smoothing belong to `APlatformerCameraManager`. Do not put movement-driven camera follow behavior into character rig transforms.

## Ladder Lifecycle

- `APlatformerCharacterBase` owns a single `EPlatformerLadderState` enum (`None / Mounting / Climbing / JumpBoost / CrouchBoost / TopFinish`) as the source of truth for ladder behavior. All transitions go through `SetLadderState`; the per-tick driver is `TickLadderState`. Animation flags are derived from the enum via `ShouldPlayLadder*Animation()` getters — do not introduce parallel ladder state booleans on the character.
- Per-ladder tuning lives on `APlatformerLadder`: `ClimbSpeed`, `JumpClimbSpeed/Duration`, dedicated `CrouchClimbSpeed/Duration` (do NOT reuse jump fields for crouch boost), `LadderMaxFlySpeed` for decoupled MaxFlySpeed clamp during mount, `MountDuration`, `TopFinishDuration`, `ReentryCooldownDuration`, and `HorizontalExitInputThreshold`.
- PaperTileMap generation treats TileSet `Ladder` / `LadderTop` as one vertical `APlatformerLadderTop` run. Only the generated LadderTop actor should exist; its ladder mesh and climb volume extend across the run. When the run's top tile is plain `Ladder`, disable the LadderTop top section instead of spawning a separate plain ladder actor.
- Air grabs skip `MountDuration` and transition directly from `Mounting` to `Climbing`; grounded entries use the configured mount timer so the start animation can play.
- Project input glue must route active/available ladder input before generic `UPlatformerTraversalMovementComponent` input handling. Ladder may cancel traversal on entry as a mutual-exclusion handoff, but ladder state transitions must not depend on `APlatformerLedgeGrab` or traversal custom modes.
- TopFinish must not toggle `Controller->SetIgnoreMoveInput/IgnoreLookInput` — input suppression during finish is owned by the FSM (callers gate themselves on `IsLadderTopFinishActive()`). Project input glue should still pass vertical ladder input/release events into `HandleLadderClimbInput` while TopFinish is active so the input-release gate can clear.
- Side-view facing restoration must stay suspended for both `IsOnLadder()` and `IsLadderTopFinishActive()`; otherwise the actor can rotate out of ladder pose while the end animation plays.
- Reentry cooldown is armed on every dismount path and blocks `TryEnterAvailableLadder` until expired.
- Abilities that should not run while climbing must block `PlatformerGameplayTags::State_Movement_Ladder` and also guard against `APlatformerCharacterBase::IsOnLadder()` / `IsLadderTopFinishActive()` in `CanActivateAbility` when they have native activation checks. Charged abilities must also guard their release/execute path so a charge started before mounting cannot fire from the ladder. Do not rely on project input alone for ladder combat/movement suppression.

## Ability Animation Resolution

- All combat abilities resolve montages through `UGA_PlatformerCombatAbilityBase::ResolveAbilityMontage(ActorInfo, AnimTag, FallbackMontage)`. Resolution order: `UPlatformerAnimInstance::AnimData` → `APlatformerCharacterBase::AnimDataAsset` → caller fallback. This works whether or not the AnimBP inherits the platformer base.
- `PlayAbilityAnimation` must play resolved montages through `UAbilitySystemComponent::PlayMontage` when an ASC is available. Direct `ACharacter::PlayAnimMontage` is only a fallback for non-GAS avatars; GAS montage state, prediction, and replication belong to ASC.
- `PlayAbilityAnimation` logs a warning via `LogPlatformerCombatAbility` when no path resolves the tag — silent fail is not acceptable for animation-driven combat.
- `ABP_PlatformerBase` owns the `DefaultSlot` upper-body overlay. Its `Layered Blend Per Bone` branch filter must start at `spine_01` with `Blend Depth = 0` for the current UE 5.6 mannequin setup. Do not use `Blend Depth = -1`; that excludes the branch, so montages can log a valid duration while remaining invisible in the final pose.
- `APlatformerCombatCharacterBase::BroadcastCombatHitReceivedEvent` is the reusable hook for sending `Event.Combat.HitReceived` to GAS. Character subclasses decide when to call it: Dragon calls it on real damage, enemies call it only after their stagger threshold.
- `UGA_PlatformerChargeShot` plays `Anim.Combat.RangedShot` on successful release so charge and base shot share one DA entry. Keep this one-tag-per-effect convention; do not introduce charge-only animation tags unless visually divergent.
- Playable character death flow is reusable foundation in `APlatformerCharacterBase`: disable player input first, keep the mesh visible, play `Anim.Combat.Death` from `AnimDataAsset` / `UPlatformerAnimInstance::AnimData`, wait for montage duration plus `DeathPostAnimationDelay` (`0.3s` default), then call `APlatformerPlayerControllerBase::HandleControlledCharacterDeath()`. Project controllers decide whether that hook opens defeat/menu UI or reloads the level.
