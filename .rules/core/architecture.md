# Architecture: BurningCORE

## Project Shape

3D side-scrolling action platformer з варіантною архітектурою.
Єдиний C++ модуль `BurningCORE` з трьома ігровими варіантами.
Кожен варіант — незалежний набір механік поверх спільного UE5 фреймворку.

## Variant Architecture

```
Source/BurningCORE/
├── BurningCORECharacter    ← abstract base (камера, рух, EnhancedInput)
├── BurningCOREGameMode     ← abstract base GameMode
├── BurningCOREPlayerController ← abstract base Controller
├── Variant_Combat/         ← melee combat: combo, charged attacks, damage, AI
│   ├── AI/                 ← CombatEnemy, CombatAIController, StateTree, EQS
│   ├── Animation/          ← AnimNotify (combo, charged, trace)
│   ├── Gameplay/           ← volumes, checkpoints, damageable objects
│   ├── Interfaces/         ← ICombatAttacker, ICombatDamageable
│   └── UI/                 ← CombatLifeBar (UMG)
├── Variant_Platforming/    ← platforming mechanics
│   └── Animation/          ← platforming-specific AnimNotify
└── Variant_SideScrolling/  ← side-scrolling mode (основний gameplay)
    ├── AI/                 ← side-scrolling AI
    ├── Gameplay/           ← side-scrolling gameplay objects
    ├── Interfaces/         ← side-scrolling interfaces
    └── UI/                 ← side-scrolling HUD
```

## Layers

| Шар | Відповідальність |
|---|---|
| **Base** | ABurningCORECharacter (abstract), GameMode, PlayerController |
| **Variant** | Конкретна реалізація: CombatCharacter, PlatformingCharacter, SideScrollingCharacter |
| **AI** | StateTree, GameplayStateTree, EQS contexts, AIController |
| **Gameplay** | Volumes, spawners, interactables, checkpoints |
| **Interfaces** | UE5 interfaces (UINTERFACE): ICombatAttacker, ICombatDamageable |
| **Animation** | AnimNotify (не AnimBP — BP-only) |
| **UI** | UMG widgets (C++ base + BP layout) |

## Key Decisions

- **Abstract base + Variant наслідування** — кожен Variant_* наслідує base класи, не редагуючи їх. ЧОМУ: масштабованість, паралельна розробка варіантів.
- **Interfaces замість hard dependencies** — ICombatAttacker/ICombatDamageable. ЧОМУ: розв'язання зв'язків між підсистемами.
- **EnhancedInput** з Do*() паттерном (Move→DoMove, Look→DoLook). ЧОМУ: дозволяє UI та AI викликати ту ж логіку без InputAction.
- **StateTree для AI** замість BehaviorTree. ЧОМУ: вибір розробника для UE 5.7+.
- **C++ base + BP derived** для Characters та UI. ЧОМУ: логіка в C++ для агента, візуали/ассети в BP.
- **PublicIncludePaths** перераховані в Build.cs. ЧОМУ: flat includes між варіантами.
