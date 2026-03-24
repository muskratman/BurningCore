# Patterns: BurningCORE UE5 C++

## Pattern 1: Variant Character (наслідування від base)

```cpp
// ✅ Variant наслідує base, додає свою логіку
UCLASS(abstract)
class ASideScrollingCharacter : public ACharacter, public ISideScrollingDamageable
{
    GENERATED_BODY()
    // Variant-specific components та properties
};

// ❌ Variant редагує base клас напряму
// Ніколи не додавай Variant-логіку в ABurningCORECharacter
```

## Pattern 2: UE5 Interface

```cpp
// ✅ Мінімальний інтерфейс — pure virtual
UINTERFACE(MinimalAPI, NotBlueprintable)
class UCombatDamageable : public UInterface
{
    GENERATED_BODY()
};

class ICombatDamageable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Damageable")
    virtual void ApplyDamage(float Damage, AActor* DamageCauser,
        const FVector& DamageLocation, const FVector& DamageImpulse) = 0;
};

// ❌ Інтерфейс з реалізацією або з полями
```

## Pattern 3: EnhancedInput — Do*() delegation

```cpp
// ✅ Input callback → public Do*() метод
void ASideScrollingCharacter::Move(const FInputActionValue& Value)
{
    FVector2D MovementVector = Value.Get<FVector2D>();
    DoMove(MovementVector.X, MovementVector.Y);  // делегація
}

UFUNCTION(BlueprintCallable, Category="Input")
virtual void DoMove(float Right, float Forward);  // AI/UI теж може викликати

// ❌ Логіка руху прямо в Input callback
void Move(const FInputActionValue& Value)
{
    // ... вся логіка руху тут — AI не зможе використати
}
```

## Pattern 4: UPROPERTY — категорії та meta

```cpp
// ✅ Повна специфікація
UPROPERTY(EditAnywhere, Category="Melee Attack|Damage",
    meta = (ClampMin = 0, ClampMax = 100))
float MeleeDamage = 1.0f;

UPROPERTY(EditAnywhere, Category="Camera",
    meta = (ClampMin = 0, ClampMax = 1000, Units = "cm"))
float DeathCameraDistance = 400.0f;

// ❌ Без категорії, без clamp, без default
UPROPERTY(EditAnywhere)
float MeleeDamage;
```

## Pattern 5: Component getters

```cpp
// ✅ FORCEINLINE getter
FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

// ✅ Private component з AllowPrivateAccess
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
    meta = (AllowPrivateAccess = "true"))
USpringArmComponent* CameraBoom;
```

## Pattern 6: AnimNotify для gameplay events

```cpp
// ✅ Окремий AnimNotify клас на кожну подію
class UAnimNotify_DoAttackTrace : public UAnimNotify  // trace
class UAnimNotify_CheckCombo : public UAnimNotify     // combo window
class UAnimNotify_CheckChargedAttack : public UAnimNotify  // charge check

// ❌ Один God-AnimNotify з enum для типу
```

## Pattern 7: Blueprint hooks (C++ → BP)

```cpp
// ✅ BlueprintImplementableEvent для візуальних ефектів
UFUNCTION(BlueprintImplementableEvent, Category="Combat")
void DealtDamage(float Damage, const FVector& ImpactPoint);

UFUNCTION(BlueprintImplementableEvent, Category="Combat")
void ReceivedDamage(float Damage, const FVector& ImpactPoint, const FVector& DamageDirection);

// ❌ Хардкод VFX/SFX в C++
```

## Anti-Patterns

| ❌ Не роби | ✅ Замість | ЧОМУ |
|---|---|---|
| Логіку Variant в base клас | Наслідуй і override | Масштабованість |
| Hard ref на assets в C++ | BP derived + BlueprintImplementableEvent | Гнучкість |
| BehaviorTree (в цьому проекті) | StateTree | Вибір архітектора |
| God-class з усіма механіками | Interface + composition | Розв'язка |
| Raw pointer без forward decl | Forward declaration в .h | Компіляція |
