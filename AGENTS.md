# BurningCORE Rules Adapter

## Commands
- Build: Запустити Build task в IDE (або Ctrl+Shift+B / Ctrl+B)

## Testing
- Відсутній формальний testing framework на даній стадії прототипу. Ручний тест через Play in Editor (PIE).

## Project Shape
- 3D side-scrolling action platformer на UE 5.7 (C++).
- Архітектура побудована на *Base abstract* класах (`BurningCORECharacter`, `BurningCOREGameMode`) та паралельних модулях: `Variant_Combat`, `Variant_Platforming`, `Variant_SideScrolling`.
- Для AI використовується StateTree + GameplayStateTree.
- Зв'язки розв'язані через UINTERFACE (напр. `ICombatDamageable`).
- Деталі: `core/architecture.md`

## Code Style
```cpp
// ✅ Forward decl в .h, UPROPERTY з категоріями, FORCEINLINE геттери
UPROPERTY(EditAnywhere, Category="Combat|Damage", meta=(ClampMin=0, ClampMax=100))
float MeleeDamage = 1.0f;

FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

// ✅ EnhancedInput через Do* паттерн
void Move(const FInputActionValue& Value) { DoMove(Value.Get<FVector2D>().X, ...); }

// ❌ Не додавай логіку Variant-модуля в спільний базовий клас
```

## Git Workflow
- Стандартні комміти, збереження перед значними рефакторингами.
- Уникай коммітів тимчасових папок чи невідтестованого коду.

## Boundaries
| ✅ Можна | ⚠️ Спроси | ❌ Заборонено |
|---|---|---|
| Змінювати .h/.cpp в рамках поточної ролі (Architect/Gameplay/UI) | Змінювати `Build.cs` | Видаляти .uasset напряму |
| Додавати нові `Variant_*/` класи | Змінювати Base класи | Змінювати `Config/` |

**Low-priority folders:** Intermediate/, DerivedDataCache/, .vs/, Binaries/, Saved/

→ **Full rules:** `.rules/` (Source of Truth) — читай `context/roles.md` для перевірки поточної ролі.
