# Roles: BurningCORE

## Architect (Архітектор)

**Область:** весь проект
**Фокус:** архітектура, базові класи, модульна структура, review, рефакторинг

| Читає | Пише |
|---|---|
| Все | Base класи (BurningCORE*.h/.cpp) |
| | Build.cs, .uproject |
| | Структурні рішення (нові Variant_*) |
| | .rules/ файли |

**Коли активувати:** зміни в базових класах, нові модулі, архітектурні рішення, Build.cs

---

## Gameplay

**Область:** Variant_*/ (крім UI/)
**Фокус:** ігрові механіки, AI (StateTree), анімація, інтерфейси

| Читає | Пише |
|---|---|
| .rules/ | Variant_*/{Gameplay,AI,Animation,Interfaces}/*.h/.cpp |
| Variant_*/ (весь) | Нові gameplay класи |
| Base класи (read-only) | AnimNotify, StateTree tasks, EQS contexts |

**Коли активувати:** нові механіки, AI поведінка, combat система, platforming логіка, анімації

**⚠️ Ескалація до Architect:** зміни в базових класах, новий Variant_*, нові модулі в Build.cs

---

## UI

**Область:** UMG віджети, HUD, меню
**Фокус:** UMG widgets (C++ base), Slate-елементи, HUD layout

| Читає | Пише |
|---|---|
| .rules/ | Variant_*/UI/*.h/.cpp |
| Variant_*/UI/ | Content/UI/ (Blueprint widgets) |
| Base класи (read-only) | Нові UMG/Slate класи |

**Коли активувати:** health bars, HUD елементи, меню, UI-анімації

**⚠️ Ескалація до Architect:** нові UI-модулі поза Variant_*/UI/, зміни в Build.cs (Slate)

## Conflict Resolution

1. Ролі НЕ перетинаються по write-доступу
2. Gameplay НЕ чіпає UI/, UI НЕ чіпає Gameplay/
3. Зміни в Base класах → тільки Architect
4. При конфлікті: Architect має пріоритет
