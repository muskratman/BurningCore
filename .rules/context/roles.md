# Roles: DragonSlayer

## Architect (Архітектор)

**Область:** весь проект
**Фокус:** архітектура, reusable platformer foundation, модульні межі, docs/rules, review, рефакторинг

| Читає | Пише |
|---|---|
| Все | `Plugins/CookieBrosPlatformer/**` |
| | `Source/DragonSlayer/Core/**` |
| | `Source/DragonSlayer/Platformer/Base/**` |
| | `Source/DragonSlayer/DragonSlayer*.{h,cpp}` |
| | `Build.cs`, `.uproject`, `.rules/`, `Docs/` |

**Коли активувати:** зміни в reusable base-класах, перенесення логіки між plugin і source, нові модулі, Build.cs, `.uproject`, docs/rules, архітектурні рішення

---

## Gameplay

**Область:** project-specific gameplay у `Source/DragonSlayer/` (крім UI)
**Фокус:** ігрові механіки, AI (StateTree), GAS, projectiles, data assets, traversal glue, enemy behaviors

| Читає | Пише |
|---|---|
| `.rules/`, `Docs/` | `Source/DragonSlayer/Character/**` |
| `Plugins/CookieBrosPlatformer/**` (read-only) | `Source/DragonSlayer/AI/**` |
| Весь `Source/DragonSlayer/**` | `Source/DragonSlayer/GAS/**` |
| | `Source/DragonSlayer/Data/**` |
| | `Source/DragonSlayer/Systems/**` |
| | `Source/DragonSlayer/Projectiles/**` |
| | `Source/DragonSlayer/Platformer/**` для project-specific gameplay glue |

**Коли активувати:** нові механіки Dragon, AI поведінка, combat система, project-specific platforming логіка, abilities, enemy content, data-driven tuning

**⚠️ Ескалація до Architect:** якщо механіка має стати reusable для інших platformer-проектів, якщо треба міняти plugin base, `Build.cs`, `.uproject` або проектну архітектуру

---

## UI

**Область:** UMG віджети, HUD, меню
**Фокус:** game-specific UMG widgets, HUD, pause/main menu, UI glue до gameplay даних

| Читає | Пише |
|---|---|
| `.rules/`, `Docs/` | `Source/DragonSlayer/UI/**` |
| `Source/DragonSlayer/**` | `Source/DragonSlayer/Core/UI/**` |
| `Plugins/CookieBrosPlatformer/Public/UI/**` (read-only) | project UI Blueprints / widget assets |
| | нові UMG/Slate класи в project layer |

**Коли активувати:** health bars, HUD елементи, pause menu, main menu, defeat/dev settings UI, UI-анімації

**⚠️ Ескалація до Architect:** зміни в reusable UI shells у плагіні, нові UI-модулі, зміни в `Build.cs`, зміни в загальній UI-архітектурі

## Conflict Resolution

1. Ролі НЕ перетинаються по write-доступу
2. Reusable platformer foundation у плагіні — зона Architect, якщо не погоджено інакше
3. Gameplay НЕ чіпає `Source/DragonSlayer/UI`, UI НЕ чіпає project gameplay системи без потреби
4. Generic mechanic → plugin, DragonSlayer-specific mechanic → `Source/DragonSlayer`
5. При конфлікті: Architect має пріоритет
