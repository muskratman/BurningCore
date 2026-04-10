# Constraints: DragonSlayer

## Overcomplexity Protection

- Простий код > розумний код; мінімум для поточної задачі
- Не створюй абстракції для одноразових операцій
- Не дублюй reusable platformer logic у `Source/DragonSlayer`, якщо її можна винести в `Plugins/CookieBrosPlatformer`
- Не тягни DragonSlayer-specific gameplay у плагін, якщо механіка не має очевидної цінності для повторного використання

## Code Style

```cpp
// ✅ Forward declarations замість #include в .h
class USpringArmComponent;
class UCameraComponent;
class UDragonFormComponent;

// ❌ Зайві #include в хедері
#include "GameFramework/SpringArmComponent.h"
```

```cpp
// ✅ UPROPERTY з Category, meta, Units
UPROPERTY(EditAnywhere, Category="Melee Attack|Trace", meta = (ClampMin = 0, ClampMax = 500, Units="cm"))
float MeleeTraceDistance = 75.0f;

// ❌ UPROPERTY без категорії та обмежень
UPROPERTY(EditAnywhere)
float MeleeTraceDistance;
```

```cpp
// ✅ UCLASS(Abstract) для reusable або project base classes
UCLASS(abstract)
class ADragonCharacter : public APlatformerCharacterBase

// ❌ Базовий клас без abstract
UCLASS()
class ADragonCharacter : public APlatformerCharacterBase
```

```cpp
// ✅ FORCEINLINE геттери для компонентів
FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

// ❌ Аутлайн геттер без причини
USpringArmComponent* GetCameraBoom() const;
```

```cpp
// ✅ Interface секції чітко позначені
// ~begin CombatAttacker interface
virtual void DoAttackTrace(FName DamageSourceBone) override;
// ~end CombatAttacker interface

// ❌ Override без маркерів інтерфейсу
virtual void DoAttackTrace(FName DamageSourceBone) override;
```

**Naming:**
- Класи: `A` (Actor), `U` (UObject), `I` (Interface), `F` (structs), `E` (enums)
- Файли: PascalCase, ім'я = клас без префіксу (`CombatCharacter.h`)
- Reusable platformer foundation: `Platformer*`, `CookieBros*`
- Project-specific layer: `Dragon*`, `DragonSlayer*`
- Log categories: `DECLARE_LOG_CATEGORY_EXTERN(LogВаріант, Log, All)`
- Input Actions: `{Дія}Action` (JumpAction, MoveAction, ComboAttackAction)

**Imports:** matching .h першим, потім CoreMinimal.h, потім Engine, потім проектні

## Defensive Coding

- ✅ `GetController() != nullptr` перед доступом до контролера
- ✅ `Cast<>()` з перевіркою результату + UE_LOG при збої
- ✅ `meta = (ClampMin, ClampMax)` для числових UPROPERTY
- ❌ Переносити project-specific знання в reusable plugin без потреби

## Path Policy

- Reusable platformer features → `Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/...`
- DragonSlayer-specific gameplay → `Source/DragonSlayer/...`
- UI проекту → `Source/DragonSlayer/UI` або `Source/DragonSlayer/Core/UI`
- Документація та ролі → `.rules/` і `Docs/`
