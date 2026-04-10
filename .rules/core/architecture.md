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
| **Source/DragonSlayer/Systems** | `UDragonSlayerSaveGame`, checkpoints, progression state |
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
