# BurningCORE — Game Architecture (Part 1: Foundation)

> **UE 5.7 · 3D Side-View Action Platformer · GAS · C++ First**

---

## 1. High-Level Architecture Summary

### Template Conversion Strategy

Third Person Template provides: `ACharacter` with `USpringArmComponent` + `UCameraComponent`, `EnhancedInput` bindings, `AGameModeBase`, `APlayerController`. We preserve the skeletal structure but replace every gameplay-facing piece:

| Keep | Replace | Add |
|---|---|---|
| Project scaffolding, Build targets | Free-orbit camera → fixed side-view | GAS plugin + AbilitySystemGlobals |
| EnhancedInput plugin | 3D movement → plane-constrained movement | Custom CharacterMovementComponent |
| Basic Character inheritance | Template GameMode → custom GameMode | Form system, Overdrive, Progression |
| UMG plugin for UI | Template input mappings → action platformer inputs | StateTree AI, Data Assets pipeline |

### Architectural Pillars

```
┌─────────────────────────────────────────────────────┐
│                  Game Instance                       │
│         (Save/Load, Global Settings, Audio)         │
├─────────────────────────────────────────────────────┤
│  GameMode ←→ GameState ←→ PlayerController          │
│     │            │              │                    │
│  Spawn/Rules   World State   Input/Camera           │
│     │            │              │                    │
│  EnemyManager  Checkpoint    DragonCharacter         │
│  BossManager   Progression      │                   │
│                                 ├── SideViewCMC     │
│                                 ├── AbilitySystem   │
│                                 ├── FormComponent   │
│                                 └── OverdriveComp   │
├─────────────────────────────────────────────────────┤
│              Data Layer (Assets + Tables)            │
│  FormDefs · AbilitySets · EnemyArchetypes · Levels  │
└─────────────────────────────────────────────────────┘
```

### Preserving 2D Readability in 3D

1. **Plane constraint** on Character Movement Component — player moves on XZ plane (Y=depth locked per-section)
2. **Fixed side camera** — orthographic feel via long focal length, minimal perspective distortion
3. **Depth lanes** — enemies and hazards exist at the same Y-depth as the player, or in clearly telegraphed depth lanes during special encounters
4. **Screen-space threat readability** — all projectiles, telegraphs, and hitboxes are designed to read cleanly from the side
5. **No Z-fighting ambiguity** — foreground/background elements use distinct visual layers

---

## 2. Core Gameplay Framework

### Class Responsibility Map

#### UBurningCOREGameInstance : UGameInstance (C++)

| Responsibility | NOT here |
|---|---|
| SaveGame management (load/save/slot selection) | Gameplay logic |
| Global audio manager references | Spawning |
| Persistent player progression cache | Input handling |
| Settings (video, audio, controls) | Camera |
| Region/level unlock state (cached from save) | HUD |

```cpp
UCLASS()
class UBurningCOREGameInstance : public UGameInstance
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Save")
    bool SaveProgress();
    
    UFUNCTION(BlueprintCallable, Category="Save")
    bool LoadProgress(int32 SlotIndex);
    
    UPROPERTY()
    TObjectPtr<UBurningCORESaveGame> CurrentSave;
    
    // Cached progression data for fast access
    UPROPERTY()
    FPlayerProgressionData CachedProgression;
};
```

#### ABurningCOREGameMode : AGameModeBase (C++)

| Responsibility | NOT here |
|---|---|
| Pawn/Controller class defaults | Save/Load |
| Respawn logic (checkpoint-based) | UI management |
| Enemy spawn orchestration (delegates to EnemyManager) | Progression tracking |
| Level flow rules (start, complete, fail) | Camera control |
| Boss encounter activation | Input binding |

```cpp
UCLASS()
class ABurningCOREGameMode : public AGameModeBase
{
    GENERATED_BODY()
public:
    void RespawnPlayerAtCheckpoint();
    void ActivateBossEncounter(ABossBase* Boss);
    void OnLevelCompleted();
    
protected:
    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<ADragonCharacter> DragonCharacterClass;
    
    UPROPERTY()
    TObjectPtr<ACheckpointActor> LastCheckpoint;
};
```

#### ABurningCOREGameState : AGameState (C++)

| Responsibility | NOT here |
|---|---|
| Current level score/ranking | Save persistence |
| Active enemy count | Spawning logic |
| Checkpoint registry for current level | Input |
| Boss phase tracking (current level) | Progression |
| Collected secrets for current run | Camera |

#### ABurningCOREPlayerController : APlayerController (C++)

| Responsibility | NOT here |
|---|---|
| Enhanced Input setup and mapping context | Gameplay abilities |
| Camera management (owns camera actor/component) | Movement physics |
| HUD/UI management | Enemy AI |
| Pause/menu flow | Save/Load |
| Input buffering support | Damage calculation |

```cpp
UCLASS()
class ABurningCOREPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void SetupInputComponent() override;
    
    void SwitchToSideViewCamera(AActor* CameraTarget);
    void TransitionCamera(UCameraRailAsset* Rail, float Duration);
    
protected:
    UPROPERTY(EditDefaultsOnly, Category="Input")
    TObjectPtr<UInputMappingContext> PlatformerMappingContext;
    
    UPROPERTY(EditDefaultsOnly, Category="Input")
    int32 MappingContextPriority = 0;
    
    UPROPERTY()
    TObjectPtr<UBurningCOREHUD> GameHUD;
};
```

#### ABurningCOREPlayerState : APlayerState (C++)

| Responsibility | NOT here |
|---|---|
| Per-session player stats (deaths, time) | Movement |
| Form unlock status (runtime mirror of save) | Camera |
| Active upgrade loadout | Spawning |

#### ADragonCharacter : ACharacter (C++, BP-derived for visuals)

| Responsibility | NOT here |
|---|---|
| Owns AbilitySystemComponent | Level flow |
| Owns FormComponent, OverdriveComponent | UI rendering |
| Custom SideViewCharacterMovementComponent | Save/Load |
| Animation interface (ABP talks to character) | Enemy management |
| Collision setup for side-view | Score tracking |

#### AEnemyBase : ACharacter (C++, BP-derived per archetype)

| Responsibility | NOT here |
|---|---|
| Owns its own AbilitySystemComponent | Player progression |
| StateTree AI component | Camera control |
| Health AttributeSet | Save/Load |
| Hit reaction / stagger logic | UI HUD |
| Drop/reward definitions (data-driven) | Form system |

#### ABossBase : AEnemyBase (C++)

| Responsibility | NOT here |
|---|---|
| Phase management (data-driven phases) | Player movement |
| Arena boundaries | Checkpoint logic |
| Scripted attack sequences via StateTree | Save/Load |
| Health gates between phases | Global progression |
| Camera shake/events triggers | HUD layout |

#### UBurningCORESaveGame : USaveGame (C++)

Stores: unlocked regions, unlocked levels, unlocked forms, upgrade purchases, collected secrets, best times, checkpoint data. Serializes via USaveGameSystem.

#### Managers (UWorldSubsystem or spawned actors)

| Manager | Type | Responsibility |
|---|---|---|
| UProgressionSubsystem | UWorldSubsystem | Upgrade state, form unlocks, region/level availability |
| ACheckpointManager | Actor (placed in level) | Track/activate checkpoints, respawn position |
| AEncounterManager | Actor (per-arena) | Spawn waves, track active enemies, trigger rewards |

---

## 3. GAS Architecture

### ASC Ownership Model

**ASC lives on the Character pawn** (not PlayerState).

ЧОМУ: single-player game, no need for ASC persistence across pawn respawns at the PlayerState level. On respawn, GameMode spawns a fresh DragonCharacter and re-initializes abilities from saved progression. Simpler, fewer indirection layers, perfectly adequate for single-player action platformer.

```cpp
// In ADragonCharacter
UPROPERTY(VisibleAnywhere, Category="Abilities")
TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

UPROPERTY()
TObjectPtr<UDragonAttributeSet> AttributeSet;
```

### Attribute Sets

#### UDragonAttributeSet : UAttributeSet

```cpp
UCLASS()
class UDragonAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    // Vitals
    UPROPERTY(BlueprintReadOnly, Category="Vitals")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, Health)
    
    UPROPERTY(BlueprintReadOnly, Category="Vitals")
    FGameplayAttributeData MaxHealth;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, MaxHealth)
    
    // Overdrive Resource
    UPROPERTY(BlueprintReadOnly, Category="Overdrive")
    FGameplayAttributeData OverdriveEnergy;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, OverdriveEnergy)
    
    UPROPERTY(BlueprintReadOnly, Category="Overdrive")
    FGameplayAttributeData MaxOverdriveEnergy;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, MaxOverdriveEnergy)
    
    // Combat
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    FGameplayAttributeData BaseDamage;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, BaseDamage)
    
    UPROPERTY(BlueprintReadOnly, Category="Combat")
    FGameplayAttributeData ChargeMultiplier;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, ChargeMultiplier)
    
    // Meta (incoming damage, used for pre-processing)
    UPROPERTY(BlueprintReadOnly, Category="Meta")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(UDragonAttributeSet, IncomingDamage)
    
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
};
```

#### UEnemyAttributeSet : UAttributeSet

Minimal: `Health`, `MaxHealth`, `BaseDamage`, `IncomingDamage`. Enemies are simpler — no overdrive, no charge.

### Gameplay Tag Taxonomy

```
State.
  State.Alive
  State.Dead
  State.Invulnerable
  State.Staggered
  State.Launched

Movement.
  Movement.Grounded
  Movement.Airborne
  Movement.WallSlide
  Movement.Dashing

Action.
  Action.Shooting
  Action.Charging
  Action.Charged       // charge complete, ready to release
  Action.FormSwitch
  Action.OverdriveActive
  Action.Interacting

Ability.
  Ability.Shoot.Base
  Ability.Shoot.Charge
  Ability.Shoot.Form   // modified by active form
  Ability.Shoot.Overdrive
  Ability.Movement.Jump
  Ability.Movement.Dash
  Ability.Movement.WallJump
  Ability.Overdrive.Activate
  Ability.Overdrive.Deactivate
  Ability.Form.Switch

Form.
  Form.Base
  Form.Fire
  Form.Ice
  Form.Electric
  // extensible

Status.
  Status.Burning
  Status.Frozen
  Status.Shocked
  Status.Poisoned
  // each with .Stack1, .Stack2, .Stack3 for threshold logic

Cooldown.
  Cooldown.Dash
  Cooldown.FormSwitch
  Cooldown.Overdrive

Damage.
  Damage.Physical
  Damage.Fire
  Damage.Ice
  Damage.Electric

Enemy.
  Enemy.Type.Melee
  Enemy.Type.Ranged
  Enemy.Type.Flying
  Enemy.Type.Shielded
  Enemy.Type.Elite
  Enemy.Type.Boss

Boss.
  Boss.Phase.1
  Boss.Phase.2
  Boss.Phase.3
  Boss.Phase.Enrage

Event.
  Event.Hit.Dealt
  Event.Hit.Received
  Event.Kill
  Event.Death
  Event.Checkpoint.Reached
  Event.Level.Complete
```

### Gameplay Effects Strategy

| Category | Type | Example |
|---|---|---|
| **Instant damage** | Instant | GE_DragonBaseShotDamage — applies IncomingDamage |
| **Status application** | Duration | GE_ApplyBurning — adds Status.Burning tag, ticks damage |
| **Status stacking** | Duration + stacking | GE_BurningStack — stacking by count, threshold triggers |
| **Attribute buff** | Duration | GE_OverdriveDamageBuff — +50% BaseDamage during Overdrive |
| **Heal** | Instant | GE_CheckpointHeal — sets Health = MaxHealth |
| **Upgrade permanent** | Infinite | GE_UpgradeBaseDamage — permanent +BaseDamage |
| **Cooldown** | Duration | GE_DashCooldown — sets Cooldown.Dash tag |

### Ability Categories

| Category | Activation | Examples |
|---|---|---|
| **Combat Active** | Input-triggered | BaseShot, ChargeShot, FormShot |
| **Movement Active** | Input-triggered | Jump, Dash, WallJump |
| **Mode Toggle** | Input-triggered, maintained | OverdriveActivate, FormSwitch |
| **Passive** | Auto-granted, listen for events | OverdriveGain (on-kill/on-hit), PassiveRegen |
| **Reaction** | Event-triggered | HitReaction, DeathAbility, StaggerAbility |

### Input Binding via Enhanced Input

```cpp
// AbilityInputAction mapping — bind InputAction to AbilitySpec
USTRUCT(BlueprintType)
struct FAbilityInputBinding
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UInputAction> InputAction;
    
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<UGameplayAbility> AbilityClass;
    
    UPROPERTY(EditDefaultsOnly)
    EAbilityActivationPolicy ActivationPolicy; // OnPressed, OnReleased, WhileHeld
};
```

The PlayerController sets up `UEnhancedInputComponent` bindings that call `ASC->TryActivateAbilityByClass()` or use input tags. Charge abilities use `WhileHeld` → `OnReleased` pattern.

### Unlockable Abilities & Upgrades

Abilities are granted via `ASC->GiveAbility()` when the player unlocks them from progression. Upgrades apply permanent GEs or modify ability levels:

```cpp
// Granting abilities from a set
void ADragonCharacter::ApplyAbilitySet(const UDragonAbilitySet* Set)
{
    for (const auto& AbilityEntry : Set->Abilities)
    {
        FGameplayAbilitySpec Spec(AbilityEntry.AbilityClass, AbilityEntry.Level);
        AbilitySystemComponent->GiveAbility(Spec);
    }
    for (const auto& EffectEntry : Set->Effects)
    {
        AbilitySystemComponent->ApplyGameplayEffectToSelf(
            EffectEntry.EffectClass->GetDefaultObject<UGameplayEffect>(),
            EffectEntry.Level, AbilitySystemComponent->MakeEffectContext());
    }
}
```

---

## 4. Dragon Character Architecture

### Component Breakdown

```
ADragonCharacter : ACharacter
├── UAbilitySystemComponent*        ← GAS core
├── UDragonAttributeSet*            ← Health, Overdrive, Damage attrs
├── USideViewMovementComponent*     ← Custom CMC (replaces default)
├── UDragonFormComponent*           ← Active form, form switching logic
├── UDragonOverdriveComponent*      ← Overdrive resource management
├── USpringArmComponent*            ← Side-view camera boom
├── UCameraComponent*               ← Follow camera
├── UCapsuleComponent*              ← Collision (inherited)
└── USkeletalMeshComponent*         ← Mesh (inherited)
```

### What Goes Where

| System | Location | ЧОМУ |
|---|---|---|
| Movement physics, plane lock | USideViewMovementComponent | CMC extension, engine-integrated |
| Ability execution | GAS (UGameplayAbility subclasses) | Decoupled, data-driven, reusable |
| Health, damage, overdrive tracking | UDragonAttributeSet + GE pipeline | Standard GAS pattern |
| Form state & switching | UDragonFormComponent | Isolated concern, clean data access |
| Overdrive fill/drain/activation | UDragonOverdriveComponent | Isolated resource management |
| Animation parameters | AnimBP reads from character/GAS tags | ABP queries, character doesn't push |
| Projectile config per form | UDragonFormDataAsset | Data-driven, no hardcoded form logic |
| Upgrade effects | GameplayEffects (permanent) | Standard GAS upgrade path |
| Visual FX/SFX for forms | GameplayCues + BP | Visual layer, not gameplay logic |

### UDragonFormComponent

```cpp
UCLASS(ClassGroup="Dragon", meta=(BlueprintSpawnableComponent))
class UDragonFormComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Form")
    void SwitchForm(FGameplayTag NewFormTag);
    
    UFUNCTION(BlueprintPure, Category="Form")
    const UDragonFormDataAsset* GetActiveFormData() const;
    
    UFUNCTION(BlueprintPure, Category="Form")
    FGameplayTag GetActiveFormTag() const { return ActiveFormTag; }
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFormChanged, FGameplayTag, OldForm, FGameplayTag, NewForm);
    UPROPERTY(BlueprintAssignable, Category="Form")
    FOnFormChanged OnFormChanged;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category="Form")
    TMap<FGameplayTag, TObjectPtr<UDragonFormDataAsset>> FormRegistry;
    
    UPROPERTY(VisibleAnywhere, Category="Form")
    FGameplayTag ActiveFormTag;
};
```

### UDragonOverdriveComponent

```cpp
UCLASS(ClassGroup="Dragon", meta=(BlueprintSpawnableComponent))
class UDragonOverdriveComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Overdrive")
    void AddOverdriveEnergy(float Amount); // called on kills, hits, pickups
    
    UFUNCTION(BlueprintCallable, Category="Overdrive")
    bool TryActivateOverdrive(); // checks threshold, applies GE
    
    UFUNCTION(BlueprintCallable, Category="Overdrive")
    void DeactivateOverdrive();
    
    UFUNCTION(BlueprintPure, Category="Overdrive")
    bool IsOverdriveActive() const;
    
    UFUNCTION(BlueprintPure, Category="Overdrive")
    float GetOverdrivePercent() const;
    
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnOverdriveChanged, bool, bActive);
    UPROPERTY(BlueprintAssignable, Category="Overdrive")
    FOnOverdriveChanged OnOverdriveStateChanged;
    
protected:
    UPROPERTY(EditDefaultsOnly, Category="Overdrive",
        meta=(ClampMin=0, ClampMax=100))
    float ActivationThreshold = 100.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Overdrive",
        meta=(ClampMin=0, ClampMax=50, Units="units/s"))
    float DrainRate = 10.0f;
    
    // Runtime managed through AttributeSet OverdriveEnergy
};
```

### Data Assets

```cpp
// Form definition — data, not code
UCLASS()
class UDragonFormDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FGameplayTag FormTag; // Form.Fire, Form.Ice, etc.
    
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FText DisplayName;
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    TSubclassOf<ADragonProjectile> ProjectileClass;
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    TSubclassOf<ADragonProjectile> ChargeProjectileClass;
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    TSubclassOf<UGameplayEffect> OnHitStatusEffect; // burning, frozen, etc.
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    int32 StatusStacksPerHit = 1;
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    int32 StatusThreshold = 3; // stacks needed to trigger full effect
    
    UPROPERTY(EditDefaultsOnly, Category="Overdrive")
    TSubclassOf<ADragonProjectile> OverdriveProjectileClass;
    
    UPROPERTY(EditDefaultsOnly, Category="Overdrive")
    float OverdriveDamageMultiplier = 1.5f;
    
    UPROPERTY(EditDefaultsOnly, Category="Charge")
    float ChargeTime = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Visuals")
    TObjectPtr<UMaterialInterface> CharacterOverlayMaterial;
    
    UPROPERTY(EditDefaultsOnly, Category="Visuals")
    FLinearColor FormColor;
    
    UPROPERTY(EditDefaultsOnly, Category="Visuals")
    TObjectPtr<UNiagaraSystem> FormAuraVFX;
};
```

### Animation Blueprint Contract

ABP queries the character/GAS, never writes to gameplay state:

| ABP reads | Source |
|---|---|
| MovementSpeed, IsFalling, IsGrounded | CMC |
| ActiveFormTag | FormComponent |
| IsOverdriveActive | OverdriveComponent |
| IsCharging, ChargePercent | GAS tags + ability state |
| IsShooting | GAS tag Action.Shooting |
| IsStaggered, IsLaunched | GAS state tags |
| HP percent | AttributeSet |

---

## 5. 3D Side-View Movement and Camera Architecture

### Movement Plane Constraint

**Approach:** Custom `USideViewMovementComponent` extending `UCharacterMovementComponent` with built-in plane constraint.

```cpp
UCLASS()
class USideViewMovementComponent : public UCharacterMovementComponent
{
    GENERATED_BODY()
public:
    USideViewMovementComponent();
    
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
    
    // The Y-coordinate to lock the character to
    UPROPERTY(EditAnywhere, Category="SideView", meta=(Units="cm"))
    float LockedDepthY = 0.0f;
    
    // Allow temporary depth lane shifts (boss encounters)
    UFUNCTION(BlueprintCallable, Category="SideView")
    void SetDepthLane(float NewY, float TransitionTime = 0.3f);
    
    // Platformer feel tuning
    UPROPERTY(EditAnywhere, Category="SideView|Jump",
        meta=(ClampMin=0, ClampMax=2000, Units="cm/s"))
    float JumpApexGravityMultiplier = 0.5f; // lighter gravity at apex for better feel
    
    UPROPERTY(EditAnywhere, Category="SideView|Jump",
        meta=(ClampMin=0, ClampMax=3, Units="s"))
    float CoyoteTime = 0.1f; // grace period after leaving edge
    
    UPROPERTY(EditAnywhere, Category="SideView|Jump",
        meta=(ClampMin=0, ClampMax=3, Units="s"))
    float JumpBufferTime = 0.15f; // pre-land jump buffer
    
    UPROPERTY(EditAnywhere, Category="SideView|Air",
        meta=(ClampMin=0, ClampMax=1))
    float AirControlMultiplier = 0.85f; // strong air control for arcade feel
    
protected:
    virtual void PhysCustom(float DeltaTime, int32 Iterations) override;
    void EnforceDepthLock(float DeltaTime);
    void ApplyApexGravityModification();
    
    float TargetDepthY;
    float DepthTransitionSpeed;
    float LastGroundedTime; // for coyote time
    float LastJumpInputTime; // for jump buffer
};
```

### Key Movement Decisions

| Decision | Choice | ЧОМУ |
|---|---|---|
| Lock axis | Y-axis (depth) hard-locked | Side-view = all action on XZ |
| Depth lanes | Optional per-section override | Boss arenas may use multiple lanes |
| Can enemies leave plane? | Yes, briefly (dodge, flank) | Adds visual depth without breaking gameplay |
| Wall interaction | WallSlide + WallJump via GAS ability | Core traversal tool for vertical platforming |
| Knockback | XZ only, never along Y | Preserve plane constraint |
| Coyote time | 0.1s default | Standard platformer feel |
| Jump buffer | 0.15s default | Reduces input frustration |
| Apex gravity | 50% at apex | Floatier jump peak, classic platformer feel |
| Air control | 85% | Near-full air control for arcade responsiveness |

### Camera Architecture

```cpp
// Side-view camera setup in constructor
CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
CameraBoom->SetupAttachment(RootComponent);
CameraBoom->TargetArmLength = 800.0f;
CameraBoom->SetRelativeRotation(FRotator(0.0f, -90.0f, 0.0f)); // look from side
CameraBoom->bDoCollisionTest = false; // no collision for fixed camera
CameraBoom->bUsePawnControlRotation = false;
CameraBoom->bInheritPitch = false;
CameraBoom->bInheritYaw = false;
CameraBoom->bInheritRoll = false;

FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
FollowCamera->bUsePawnControlRotation = false;
```

**Camera behaviors by context:**

| Context | Camera Behavior |
|---|---|
| Normal traversal | Smooth follow with look-ahead in movement direction |
| Vertical sections | Camera follows vertically, wider zoom |
| Boss arena | Locked to arena center, zoom out to show full arena |
| Tight corridors | Zoom in, reduce look-ahead |
| Cinematics | Camera rails / custom sequences |

Use `ACameraVolume` trigger volumes to switch camera params:

```cpp
UCLASS()
class ACameraZoneVolume : public ATriggerVolume
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Camera")
    float TargetArmLength = 800.0f;
    
    UPROPERTY(EditAnywhere, Category="Camera")
    FVector CameraOffset = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, Category="Camera")
    float TransitionTime = 0.5f;
    
    UPROPERTY(EditAnywhere, Category="Camera")
    bool bLockToArenaCenterX = false;
    
    UPROPERTY(EditAnywhere, Category="Camera")
    FVector ArenaCenterOverride;
};
```

---

## 6. Combat Architecture

### Projectile System

```cpp
UCLASS(abstract)
class ADragonProjectile : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<USphereComponent> CollisionSphere;
    
    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UProjectileMovementComponent> ProjectileMovement;
    
    UPROPERTY(VisibleAnywhere, Category="Components")
    TObjectPtr<UNiagaraComponent> TrailVFX;
    
    // Set by the firing ability
    UPROPERTY()
    FGameplayEffectSpecHandle DamageEffectSpec;
    
    UPROPERTY()
    FGameplayEffectSpecHandle StatusEffectSpec; // form-specific status
    
    UPROPERTY(EditDefaultsOnly, Category="Projectile",
        meta=(ClampMin=0, ClampMax=5000, Units="cm/s"))
    float Speed = 2000.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Projectile")
    bool bPiercing = false; // some forms pierce
    
    UPROPERTY(EditDefaultsOnly, Category="Projectile",
        meta=(ClampMin=0, ClampMax=10, Units="s"))
    float Lifetime = 3.0f;
    
protected:
    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
    
    void ApplyDamageAndStatus(AActor* Target);
};
```

### Hit Detection

**Projectiles:** `UProjectileMovementComponent` + `USphereComponent` overlap/hit events. Simple, reliable, pool-friendly.

**Melee (enemy attacks):** Sphere trace from AnimNotify at specific animation frames (existing pattern from project).

**Boss attacks:** Mix of projectiles + area-of-effect volumes + sweep traces.

### Damage Pipeline

```
Ability fires → Creates GE Spec (Damage.Physical + form Damage type)
    → Projectile carries GE Spec
    → On hit: ASC->ApplyGameplayEffectSpecToTarget()
    → Target's AttributeSet::PostGameplayEffectExecute()
        → Clamp damage, apply IncomingDamage to Health
        → Check death (Health <= 0)
        → Trigger hit reaction (State.Staggered tag)
    → Status GE applied separately (stacking)
        → Status stack count checked
        → Threshold reached → trigger status burst effect
```

### Enemy Hurt/Stagger Logic

```cpp
// In UEnemyAttributeSet::PostGameplayEffectExecute
if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
{
    float Damage = GetIncomingDamage();
    SetIncomingDamage(0.0f);
    
    float NewHealth = FMath::Clamp(GetHealth() - Damage, 0.0f, GetMaxHealth());
    SetHealth(NewHealth);
    
    if (NewHealth <= 0.0f)
    {
        // Add State.Dead tag, trigger death ability
        ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag("State.Dead"));
    }
    else if (Damage >= StaggerThreshold)
    {
        // Trigger stagger via gameplay event
        FGameplayEventData EventData;
        EventData.EventMagnitude = Damage;
        ASC->HandleGameplayEvent(
            FGameplayTag::RequestGameplayTag("Event.Hit.Received"), &EventData);
    }
}
```

### Status Stack System

Forms apply status effects that stack. At threshold → burst:

| Form | Status | Stacks/Hit | Threshold | Burst Effect |
|---|---|---|---|---|
| Fire | Status.Burning | 1 | 3 | Ignite: DoT + knockback |
| Ice | Status.Frozen | 1 | 3 | Freeze: brief immobilize |
| Electric | Status.Shocked | 1 | 3 | Chain: damage nearby enemies |

Stacking implemented via GE stacking policy (`EGameplayEffectStackingType::AggregateByTarget`, limit = threshold).

### Screen Readability Rules

1. **Max active player projectiles on screen:** 6 (configurable). Oldest recycled via object pool.
2. **Enemy telegraph minimum:** 0.4s visual warning before damage frame
3. **Boss telegraph minimum:** 0.6s for arena-wide attacks
4. **Color coding:** player projectiles = warm (based on form), enemy projectiles = cool/red
5. **Depth:** all gameplay-relevant projectiles on player's Y-depth plane
6. **Decorative projectiles** (boss VFX, environment) can exist at other depths but never deal damage

### Dragon Attack Architecture

| Attack | Trigger | Ability | Projectile Source |
|---|---|---|---|
| Base shot | Tap Shoot | GA_DragonBaseShot | FormData.ProjectileClass |
| Charge shot | Hold+Release Shoot | GA_DragonChargeShot | FormData.ChargeProjectileClass |
| Form shot | Tap Shoot (form active) | Same GA_DragonBaseShot | FormData drives different projectile |
| Overdrive shot | Tap Shoot (overdrive active) | Same GA_DragonBaseShot | FormData.OverdriveProjectileClass |
| Overdrive + Form | Both active | Same GA_DragonBaseShot | Logic: overdrive variant of current form |

The base shot ability reads from `FormComponent->GetActiveFormData()` to determine which projectile to spawn and what GE specs to attach. **One ability class, multiple data-driven behaviors.**
