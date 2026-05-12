# DragonSlayer — Game Architecture (Part 2: Systems)

Part 2 фіксує не абстрактні ідеї, а поточний system map реального проекту.

---

## 1. Core Runtime Flow

| Система | Reusable Base | Project Layer |
|---|---|---|
| GameInstance | `UPlatformerGameInstance` | `UDragonSlayerGameInstance` |
| GameMode | `APlatformerGameModeBase` | `APlatformerGameMode` |
| PlayerController | `APlatformerPlayerControllerBase` | `APlatformerPlayerController` |
| HUD / menus | reusable UI bases | `ADragonSlayerHUD`, main menu classes, project widgets |

### Responsibility Split

- плагін дає flow hooks: respawn, checkpoint registry/subsystem, pickup sink, input context base, pause/respawn hooks;
- проект додає конкретний UI spawn flow, startup level routing, project-specific pause/developer behaviour.

Checkpoint ownership:

- `APlatformerCheckpoint` is the reusable environment actor.
- `UPlatformerCheckpointSubsystem` is the world-local runtime owner for active checkpoint, restore from save, and respawn.
- Project-specific checkpoint presentation classes should inherit directly from `APlatformerCheckpoint`.
- `UPlatformerSaveGame::LastCheckpoint` stores current level and checkpoint identity; project save classes extend this payload only for project-specific progression.

---

## 2. Character Stack

### Reusable Layer

- `APlatformerCharacterBase`
- `APlatformerCombatCharacterBase`
- `USideViewMovementComponent`
- `UPlatformerTraversalComponent`
- `UPlatformerTraversalMovementComponent`

### Project Layer

- `ADragonCharacter`
- `UDragonFormComponent`
- `UDragonOverdriveComponent`
- `APlayableDragonCharacter`

### Practical Reading

- side-view movement shell, camera rig і combat bootstrap приходять із плагіна;
- DragonSlayer додає форми, Overdrive, shot logic, project attributes і current playable bindings.

### Ladder FSM (Reusable Layer)

- `APlatformerCharacterBase` володіє єдиним `EPlatformerLadderState` enum: `None / Mounting / Climbing / JumpBoost / CrouchBoost / TopFinish`.
- Усі переходи проходять через `SetLadderState(NewState)`. Tick керується одним методом `TickLadderState(DeltaTime)`. Animation booleans (`bShouldLadderStart/Loop/End`) обчислюються з enum через геттери `ShouldPlayLadder*Animation()`.
- Public input API залишається стабільним для project layer: `EnterLadder`, `ExitLadder`, `HandleLadderClimbInput`, `HandleLadderHorizontalExitInput`, `PerformLadderJump`, `PerformLadderCrouch`, `IsOnLadder`, `IsLadderTopFinishActive`.
- Per-ladder data на `APlatformerLadder`: `ClimbSpeed`, `JumpClimbSpeed/Duration`, `CrouchClimbSpeed/Duration`, `LadderMaxFlySpeed` (decoupled clamp на час mount'у), `MountDuration`, `TopFinishDuration`, `ReentryCooldownDuration`, `HorizontalExitInputThreshold`. `MaxFlySpeed` персонажа зберігається на mount і відновлюється на dismount.
- Re-entry cooldown активується на TopFinish enter та на bottom/sideways dismount; блокує `TryEnterAvailableLadder` поки `ReentryCooldownDuration` не сплине.

### Animation Resolution (Reusable Layer)

- `UGA_PlatformerCombatAbilityBase::ResolveAbilityMontage` шукає монтаж за `FGameplayTag` у трьох порядках: 1) `UPlatformerAnimInstance::AnimData` (якщо AnimBP наслідує платформерну базу); 2) `APlatformerCharacterBase::AnimDataAsset` напряму (для ванільних AnimBP); 3) caller-supplied fallback.
- `PlayAbilityAnimation` програє знайдений montage через `UAbilitySystemComponent::PlayMontage`, якщо ASC доступний. Прямий `ACharacter::PlayAnimMontage` лишається fallback для non-GAS avatar path.
- `PlayAbilityAnimation` логує warning через `LogPlatformerCombatAbility`, якщо тег не зарезолвлено по жодному шляху, — silent fail прибраний.
- `APlatformerCombatCharacterBase::BroadcastCombatHitReceivedEvent` є reusable hook для `Event.Combat.HitReceived`. Dragon викликає його після реального damage, enemy layer — тільки після власного stagger threshold.
- `UGA_PlatformerChargeShot::InputReleased` грає `Anim.Combat.RangedShot` після успішного пострілу (опціонально — лише якщо тег є в DA), щоб charge release і base shot ділили один animation entry.
- `APlatformerCharacterBase` owns playable death sequencing: input is disabled first, `Anim.Combat.Death` is played from the character/AnimBP AnimData, then the flow waits montage duration + `DeathPostAnimationDelay` (`0.3s` default) before calling `APlatformerPlayerControllerBase::HandleControlledCharacterDeath()`. DragonSlayer-specific restart/defeat-menu behavior stays in `APlatformerPlayerController`.

---

## 3. GAS Stack

### Reusable In Plugin

- base abilities (`GA_Dash`, `GA_Jump`, `GA_Crouch`, etc.)
- reusable combat ability base classes
- reusable platformer attribute set
- reusable ability-set infrastructure

### Project-Specific In DragonSlayer

- `UGA_DragonBaseShot`
- `UGA_DragonChargeShot`
- `UGA_FormSwitch`
- `UGA_OverdriveActivate`
- `UGA_HitReaction`
- `UDragonAbilitySet`
- `UDragonAttributeSet`
- `UEnemyAttributeSet`
- `DeveloperDamageGameplayEffect`

### Rule

Якщо ability або effect має сенс поза DragonSlayer, він може бути кандидатом у плагін. Якщо він знає про Dragon forms, Overdrive або Dragon projectile flow, він має лишатися у проекті.

---

## 4. AI Stack

### Reusable In Plugin

- `APlatformerEnemyBase`
- `APlatformerEnemyMelee`
- `APlatformerEnemyRanged`
- `APlatformerEnemyFlying`
- `APlatformerBossBase`
- reusable enemy archetype asset shell

### Project-Specific In DragonSlayer

- `AEnemyMelee`
- `AEnemyRanged`
- `AEnemyFlying`
- `ABossBase`
- `UEnemyArchetypeAsset`

### AI Tech

- `StateTree`
- `GameplayStateTree`
- `AIModule`
- optional EQS/helper logic when needed

AI має залишатися читабельним і добре тюнитись через data assets/state tree, а не через жорстко прошиті гілки поведінки.

---

## 5. Traversal & Environment

Базові platformer interactions належать плагіну:

- ladder
- moving platform / triggered lift / closing door
- jump pad
- conveyor
- spikes / danger blocks / gravity volumes
- teleporter
- pickups
- grapple points
- surface block variants

Shared environment path data also belongs to the plugin. `UPlatformerPathComponent` is the reusable local-space path component used by enemy patrols, moving platform / triggered lift / closing door routes, and camera volume point pairs. Each `FPlatformerPathPoint` owns `PointLocation`, `PointDelay`, and `SpeedScale`, while the component owns `bRepeatPath`. It also owns a single editor debug render proxy that draws red lines between points, point markers, and index labels without adding child components to runtime actors. The component visualizer adds selectable point handles in editor viewports, while the debug proxy returns no scene proxy in game worlds, so path visualization is hidden during play. Moving platform-derived actors read route, delays, speed scale, and repeat mode from the component; actor-local `PointA` / `PointB` scene components and duplicate point delay/repeat properties are no longer part of the movement model.

DragonSlayer може додавати project-specific glue на рівні playable character або level flow, але не повинен копіювати ці актори в runtime module без сильної причини.

---

## 6. Save & Progression

### Reusable Layer

- `UPlatformerSaveGame`
- save settings/service contracts
- save validation and developer settings storage infrastructure

### Project Layer

- `UDragonSlayerSaveGame`
- `FPlayerProgressionData`
- project startup flow у `UDragonSlayerGameInstance`

### What The Project Save Tracks

- unlocked regions
- level completion states
- unlocked forms
- purchased upgrades
- collected secrets
- upgrade currency
- total deaths

---

## 7. UI & Developer Tooling

### Reusable UI In Plugin

- `UPlatformerDeveloperSettingsWidget`
- `UPlatformerHealthWidget`
- `UPlatformerDefeatWidget`
- developer parameter/vector/checkbox widgets

### Project UI In DragonSlayer

- `UPlatformerUI`
- `UPauseWidget`
- `UDeveloperSettingsWidget`
- main menu widget/controller/HUD/game mode
- `ADragonSlayerHUD`

### UI Rule

Reusable widgets і tool widgets живуть у плагіні.
Project-specific UX, visual language, data binding і menu flow живуть у `Source/DragonSlayer`.

---

## 8. Camera

- Камерний shell: `APlatformerCameraManager`
- Character rig: `USpringArmComponent` + `UCameraComponent` у `APlatformerCharacterBase`
- Поточна поведінка: smooth follow + movement look-ahead
- Поточне правило: hard X clamping через `Camera XMin Bounds` / `Camera XMax Bounds` не використовується
- Facing-relative spring-arm offsets stay on the character rig. `APlatformerCameraManager` smooths the resulting world-space rig offset across X/Y/Z so turnarounds do not snap the camera depth or focus. X/Y rig offset smoothing uses `HorizontalOffsetInterpSpeedStart`.

---

## 9. Directory-Level Checklist

Перед новою системою перевір:

1. Чи є вже схожий reusable shell у `CookieBrosPlatformer`?
2. Чи знає новий код про Dragon-specific дані?
3. Чи не дублюється вже існуюча mechanic або widget base?
4. Чи можна розв'язати зв'язок через existing interface / GAS / DataAsset?

Part 2 описує system ownership. Part 3 визначає production rules для щоденних змін.
