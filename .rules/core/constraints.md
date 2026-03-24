# Constraints: BurningCORE

## Overcomplexity Protection

- Простий код > розумний код; мінімум для поточної задачі
- Не створюй абстракції для одноразових операцій
- Новий Variant_* модуль → ⚠️ підтвердження

## Code Style

```cpp
// ✅ Forward declarations замість #include в .h
class USpringArmComponent;
class UCameraComponent;

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
// ✅ UCLASS(abstract) для базових класів
UCLASS(abstract)
class ACombatCharacter : public ACharacter, public ICombatAttacker, public ICombatDamageable

// ❌ Базовий клас без abstract
UCLASS()
class ACombatCharacter : public ACharacter
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
- Variant-специфічні: `{Variant}ClassName` (CombatCharacter, PlatformingCharacter)
- Log categories: `DECLARE_LOG_CATEGORY_EXTERN(LogВаріант, Log, All)`
- Input Actions: `{Дія}Action` (JumpAction, MoveAction, ComboAttackAction)

**Imports:** matching .h першим, потім CoreMinimal.h, потім Engine, потім проектні

## Defensive Coding

- ✅ `GetController() != nullptr` перед доступом до контролера
- ✅ `Cast<>()` з перевіркою результату + UE_LOG при збої
- ✅ `meta = (ClampMin, ClampMax)` для числових UPROPERTY
- ❌ Defensive coding між внутрішніми модулями одного Variant

## Path Policy

- Можливості > шляхи: «combat system» > «Variant_Combat/Gameplay/»
- Критичний шлях → `<!-- VERIFY -->` маркер
- PublicIncludePaths в Build.cs <!-- VERIFY -->
