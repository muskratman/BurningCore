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

- плагін дає flow hooks: respawn, checkpoint registry, pickup sink, input context base, pause/respawn hooks;
- проект додає конкретний UI spawn flow, startup level routing, project-specific pause/developer behaviour.

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

---

## 9. Directory-Level Checklist

Перед новою системою перевір:

1. Чи є вже схожий reusable shell у `CookieBrosPlatformer`?
2. Чи знає новий код про Dragon-specific дані?
3. Чи не дублюється вже існуюча mechanic або widget base?
4. Чи можна розв'язати зв'язок через existing interface / GAS / DataAsset?

Part 2 описує system ownership. Part 3 визначає production rules для щоденних змін.
