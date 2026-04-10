# DragonSlayer — Game Architecture (Part 1: Foundation)

> **UE 5.6 · 3D Side-View Action Platformer · CookieBrosPlatformer + DragonSlayer · C++ First**

---

## 1. Architectural Thesis

DragonSlayer будується не як один великий runtime module, а як комбінація:

- reusable platformer foundation у `Plugins/CookieBrosPlatformer`
- project-specific реалізації у `Source/DragonSlayer`

Це означає:

- generic platformer systems не дублюються в проекті;
- DragonSlayer-specific gameplay не протікає в reusable base;
- нові фічі за замовчуванням додаються через inheritance, а не через копіювання shell-класів.

---

## 2. Foundation Lives In The Plugin

Базовий шар плагіна містить опори, на які спирається проект:

| Base Class / System | Призначення |
|---|---|
| `UPlatformerGameInstance` | save/load shell, startup glue, save progress service |
| `APlatformerGameModeBase` | checkpoints, boss encounter registry, pickup flow |
| `APlatformerPlayerControllerBase` | input mapping contexts, pause/respawn hooks |
| `APlatformerCharacterBase` | camera rig, side-view movement shell, combat bootstrap |
| `APlatformerEnemyBase` + variants | reusable enemy shells |
| `APlatformerCameraManager` | reusable side-view camera behaviour |
| `UPlatformerSaveGame` | save payload shell |
| `UPlatformerDeveloperSettingsWidget` та інші UI bases | reusable widgets для health/dev/settings |
| traversal/environment actors | ladders, moving platforms, hazards, pickups, teleports |

---

## 3. Project Anchors Live In `Source/DragonSlayer`

Проектний модуль додає все, що робить гру саме DragonSlayer:

| Class / Folder | Призначення |
|---|---|
| `UDragonSlayerGameInstance` | конкретний тип сейва, startup routing |
| `ADragonCharacter` | базовий герой проекту |
| `APlayableDragonCharacter` | production pawn із current input/traversal glue |
| `UDragonFormComponent` | система форм |
| `UDragonOverdriveComponent` | система Overdrive |
| `UDragonSlayerSaveGame` | progression payload проекту |
| `UDragonFormDataAsset` | Dragon-specific form tuning |
| `UEnemyArchetypeAsset` | concrete enemy tuning поверх reusable base asset |
| `AEnemyMelee`, `AEnemyRanged`, `AEnemyFlying`, `ABossBase` | project enemies поверх reusable shells |
| `UI/`, `Core/UI/` | HUD, pause, main menu, game-specific UI |

---

## 4. Extension Model

Preferred path для нової системи:

1. Знайти reusable shell у `CookieBrosPlatformer`.
2. Якщо shell покриває задачу, розширити його у `Source/DragonSlayer`.
3. Якщо reusable shell бракує, спочатку оцінити, чи ця відсутня логіка справді generic.
4. Якщо логіка generic для платформера, додати її в плагін.
5. Якщо логіка специфічна для DragonSlayer, додати її в проектний модуль.

Короткі приклади:

- `APlatformerCharacterBase` → `ADragonCharacter`
- `ADragonCharacter` → `APlayableDragonCharacter`
- `UPlatformerSaveGame` → `UDragonSlayerSaveGame`
- `APlatformerEnemyMelee` → `AEnemyMelee`

---

## 5. Foundation Rules

- Системна логіка живе в C++.
- Blueprint відповідає за presentation, assembly і asset wiring.
- `StateTree` є основним AI path.
- `GAS` є основним combat/ability path.
- Інтерфейси та data assets краще за hard references, коли контракт має жити між кількома акторами.
- Камера залишається side-view shell у плагіні; project module лише використовує її або розширює через існуючі hooks.

---

## 6. Current Production Focus

На поточному етапі production-first фокус — Dragon gameplay.

Це означає:

- не треба ускладнювати reusable або project layers заради гіпотетичних майбутніх кампаній;
- якщо майбутні системи для інших героїв з'являться, вони повинні сідати на вже наявну платформерну основу, а не переписувати її.

Part 1 фіксує фундамент: що є reusable foundation, а що є project layer. Конкретні системи і production workflow описані в Part 2 і Part 3.
