# DragonSlayer — Technical Architecture

> **Тип:** 3D side-scrolling action platformer  
> **Движок:** Unreal Engine 5.6  
> **Мова:** C++ (PCH: Explicit) + Blueprint only for layout/presentation  
> **Архітектурна модель:** reusable platformer plugin + project-specific runtime module  
> **Поточний production focus:** Dragon-first gameplay loop  

---

## 1. Modules & Ownership

Проект більше не є legacy single-module layout. Поточна структура така:

| Модуль / Плагін | Призначення |
|---|---|
| `Source/DragonSlayer` | Основний runtime module проекту |
| `Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer` | Reusable platformer foundation |
| `Plugins/CookieBrosPlatformer/Source/CookieBrosLevelEditor` | Editor/import tooling для рівнів і tilemap pipeline |

### `DragonSlayer.uproject`

- `EngineAssociation`: `5.6`
- Runtime module: `DragonSlayer`
- Enabled plugins: `StateTree`, `GameplayStateTree`, `GameplayAbilities`, `CookieBrosPlatformer`, `ModelingToolsEditorMode`

### `DragonSlayer.Build.cs`

Проектний модуль залежить від:

- `CookieBrosPlatformer`
- `EnhancedInput`
- `AIModule`
- `StateTreeModule`
- `GameplayStateTreeModule`
- `GameplayAbilities`, `GameplayTags`, `GameplayTasks`
- `UMG`, `Slate`, `SlateCore`
- `Niagara`
- `DeveloperSettings`

---

## 2. Placement Rule

Ключове правило проекту:

- **`Plugins/CookieBrosPlatformer`** зберігає базові механіки платформера, які мають сенс для повторного використання в інших проектах.
- **`Source/DragonSlayer`** зберігає все, що є проектно-специфічним для DragonSlayer.

### Що належить плагіну

- reusable `GameMode`, `PlayerController`, `GameInstance`
- reusable character/combat/enemy shells
- traversal і environment actors
- reusable camera manager
- save/load shell і developer settings infrastructure
- reusable UI base widgets
- interfaces, shared GAS helpers, generic projectiles

### Що належить проекту

- Dragon hero, форми, Overdrive
- Dragon-specific abilities, attributes, effects
- concrete enemies/bosses для цього проекту
- DragonSlayer save payload і progression model
- HUD, pause menu, main menu
- game-specific flow glue поверх reusable shell-класів

Якщо нова логіка не знає про Dragon, Overdrive, форми, конкретну прогресію або UI DragonSlayer, вона, ймовірно, має жити в плагіні.

---

## 3. Runtime Structure

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/
├── Core/                 reusable flow, save shell, developer settings
├── Character/            APlatformerCharacterBase, side-view movement shell
├── Combat/               reusable combat character base
├── AI/                   APlatformerEnemyBase, APlatformerBossBase, enemy shells
├── GAS/                  reusable abilities, attributes, ability sets
├── Interfaces/           damage/save/checkpoint/pickup contracts
├── Platformer/
│   ├── Camera/           APlatformerCameraManager
│   ├── Environment/      blocks, platforms, hazards, teleporter, ladder, etc.
│   ├── Character/        platformer interaction contracts
│   └── Systems/          checkpoint actor shell
├── Traversal/            traversal components and movement
├── UI/                   reusable dev/settings/health widgets
└── Projectiles/          base projectile shells

Source/DragonSlayer/
├── Core/                 UDragonSlayerGameInstance, developer settings, main menu, HUD
├── Character/            ADragonCharacter, DragonFormComponent, DragonOverdriveComponent
├── GAS/                  Dragon abilities, attributes, effects, ability set
├── AI/                   AEnemyMelee, AEnemyRanged, AEnemyFlying, ABossBase
├── Data/                 UDragonFormDataAsset, UEnemyArchetypeAsset
├── Systems/              UDragonSlayerSaveGame, ACheckpointActor
├── Projectiles/          Dragon projectile types
├── Platformer/
│   ├── Base/             APlatformerGameMode, APlatformerPlayerController
│   └── Character/        APlayableDragonCharacter
└── UI/                   runtime HUD + pause/developer widgets
```

---

## 4. Inheritance Model

Проект розширює плагін, а не копіює його.

| Reusable Base | Project Extension |
|---|---|
| `UPlatformerGameInstance` | `UDragonSlayerGameInstance` |
| `APlatformerGameModeBase` | `APlatformerGameMode` |
| `APlatformerPlayerControllerBase` | `APlatformerPlayerController` |
| `APlatformerCharacterBase` | `ADragonCharacter` |
| `ADragonCharacter` | `APlayableDragonCharacter` |
| `APlatformerEnemyMelee` | `AEnemyMelee` |
| `APlatformerEnemyRanged` | `AEnemyRanged` |
| `APlatformerEnemyFlying` | `AEnemyFlying` |
| `APlatformerBossBase` | `ABossBase` |
| `UPlatformerEnemyArchetypeAsset` | `UEnemyArchetypeAsset` |
| `UPlatformerSaveGame` | `UDragonSlayerSaveGame` |
| `UPlatformerDeveloperSettingsWidget` | `UDeveloperSettingsWidget` |

Це і є preferred path для нових систем: знайти правильний reusable shell і розширити його.

---

## 5. System Responsibilities

### Character & Combat

- `APlatformerCharacterBase` дає camera rig, side-view movement shell, combat/bootstrap і developer settings hooks.
- `ADragonCharacter` додає hero-specific компоненти: `UDragonFormComponent` і `UDragonOverdriveComponent`.
- `APlayableDragonCharacter` підключає current production input, traversal component і gameplay glue для playable build.

### GAS

- reusable платформа вже містить base abilities / attributes / ability-set infrastructure;
- `Source/DragonSlayer/GAS` додає Dragon-specific shot, charge shot, form switch, hit reaction, overdrive activation;
- damage, combat states і tuning проходять через GAS/DataAssets, а не через hardcoded UI logic.

### AI

- база ворогів і reusable enemy shells живуть у плагіні;
- concrete project enemies та boss-класи живуть у `Source/DragonSlayer/AI`;
- основний стек AI: `StateTree` + `GameplayStateTree` + `AIModule`.

### Data

- `UDragonFormDataAsset` описує бойові форми Dragon;
- `UEnemyArchetypeAsset` розширює reusable enemy archetype shell для конкретного проектного тюнінгу.

### Save & Progression

- плагін надає save shell і save service contracts;
- `UDragonSlayerSaveGame` зберігає progression payload: регіони, стани рівнів, форми, апгрейди, секрети, валюту, смерті;
- `UDragonSlayerGameInstance` прив'язує конкретний тип сейва і startup flow проекту.

### UI

- reusable developer/health/settings widgets знаходяться у плагіні;
- game HUD, pause menu, main menu і DragonSlayer-specific presentation живуть у `Source/DragonSlayer/UI` та `Source/DragonSlayer/Core/UI`.

### Camera

- `APlatformerCameraManager` живе у плагіні як reusable side-view camera shell;
- поточна камера використовує smooth follow і movement-based look-ahead;
- фіксація через `Camera XMin Bounds` / `Camera XMax Bounds` більше не є частиною актуальної поведінки.

---

## 6. Current Architectural Rules

- Generic platformer feature first goes to `CookieBrosPlatformer`.
- DragonSlayer-specific logic stays in `Source/DragonSlayer`.
- Не дублювати plugin base class у проекті, якщо достатньо inheritance.
- Не тягнути Dragon-specific knowledge в reusable plugin.
- Для AI використовується `StateTree`, не `BehaviorTree`.
- Основна логіка в C++, Blueprint використовуються для presentation/layout/asset wiring.

Цей документ є поточним source of truth для технічної структури проекту. Деталізацію по ролях дивись у `Docs/Roles/` та `.rules/context/roles.md`.
