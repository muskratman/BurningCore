# Patterns: DragonSlayer UE5 C++

## Pattern 1: Project-Specific Character Extends Reusable Base

```cpp
// ✅ DragonSlayer character наслідує reusable platformer shell
UCLASS()
class DRAGONSLAYER_API ADragonCharacter : public APlatformerCharacterBase
{
    GENERATED_BODY()
    // Dragon forms, overdrive, project-specific combat data
};

// ❌ Копіювати camera/combat/movement shell з плагіна в project module
```

## Pattern 2: Project Glue Extends Platformer Flow Shell

```cpp
// ✅ Project mode/controller розширюють reusable flow classes
UCLASS()
class APlatformerGameMode : public APlatformerGameModeBase
{
    GENERATED_BODY()
    // UI spawning, project pickup flow, level-specific hooks
};

// ❌ Створювати окремий паралельний framework у Source без reuse base class
```

## Pattern 3: Data Assets Reuse Plugin Contracts

```cpp
// ✅ Project asset розширює reusable archetype shell
UCLASS()
class DRAGONSLAYER_API UEnemyArchetypeAsset : public UPlatformerEnemyArchetypeAsset
{
    GENERATED_BODY()
    // DragonSlayer-specific stats, tags, rewards
};

// ❌ Дублювати в project module уже існуючий reusable DataAsset base
```

## Pattern 4: Thin EnhancedInput Glue

```cpp
// ✅ Input handler thinly translates EnhancedInput data into gameplay actions
void APlayableDragonCharacter::Input_Move(const FInputActionValue& Value)
{
    const FVector2D MoveVector = Value.Get<FVector2D>();
    AddMovementInput(FVector(1.0f, 0.0f, 0.0f), MoveVector.X);
}

// ❌ Хардкодити великий шмат gameplay/system logic прямо в bind setup
```

## Pattern 5: UPROPERTY — категорії та meta

```cpp
// ✅ Повна специфікація
UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Projectile",
    meta = (ClampMin = 0.0, Units = "cm"))
float ProjectileMaxDistance = 1500.0f;

// ❌ Без категорії, без clamp, без default
UPROPERTY(EditAnywhere)
float ProjectileMaxDistance;
```

## Pattern 6: Component Getters

```cpp
// ✅ FORCEINLINE getter
FORCEINLINE UDragonFormComponent* GetFormComponent() const { return FormComponent; }

// ✅ Private component з AllowPrivateAccess
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
    meta = (AllowPrivateAccess = "true"))
TObjectPtr<UDragonFormComponent> FormComponent;
```

## Pattern 7: Save/Data/UI Derivation Over Duplication

```cpp
// ✅ Project save/widget classes derive from reusable platformer shells
class DRAGONSLAYER_API UDragonSlayerSaveGame : public UPlatformerSaveGame;
class DRAGONSLAYER_API UDeveloperSettingsWidget : public UPlatformerDeveloperSettingsWidget;

// ❌ Копіювати reusable save/widget implementation в Source замість наслідування
```

## Anti-Patterns

| ❌ Не роби | ✅ Замість | ЧОМУ |
|---|---|---|
| Generic platformer logic у `Source/DragonSlayer` | Винось у `Plugins/CookieBrosPlatformer` | Reuse |
| Dragon-specific logic у плагін | Тримай у `Source/DragonSlayer` | Чисті межі |
| Паралельний framework поруч з plugin shell | Наслідуй існуючі `Platformer*` base classes | Менше дублювання |
| BehaviorTree (в цьому проекті) | StateTree | Узгодженість стеку |
| Raw pointer без forward decl | Forward declaration в `.h` | Компіляція |
