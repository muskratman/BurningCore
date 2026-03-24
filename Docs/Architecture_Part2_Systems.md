# BurningCORE — Game Architecture (Part 2: Systems)

> **Sections 7-12: Forms, Progression, Enemies, Data Pipeline, Animation, Folder Structure**

---

## 7. Form System Design

### Architecture Overview

Forms are **purely data-driven**. The `UDragonFormComponent` holds the active `FGameplayTag` and looks up the corresponding `UDragonFormDataAsset`. No form-specific C++ subclasses.

```
UDragonFormDataAsset (one per form)
├── FormTag: Form.Fire
├── ProjectileClass: BP_FireProjectile
├── ChargeProjectileClass: BP_FireChargeProjectile
├── OverdriveProjectileClass: BP_FireOverdriveProjectile
├── OnHitStatusEffect: GE_ApplyBurning
├── StatusStacksPerHit: 1
├── StatusThreshold: 3
├── ChargeTime: 1.2s
├── OverdriveDamageMultiplier: 1.5
├── FormColor: (1.0, 0.3, 0.0)
├── FormAuraVFX: NS_FireAura
├── CharacterOverlayMaterial: MI_FireOverlay
└── AbilitySet: DA_FireFormAbilities (optional extra abilities)
```

### Form Switching Flow

```
Input (FormSwitch) 
  → GA_FormSwitch activates
  → Checks Cooldown.FormSwitch tag
  → FormComponent->SwitchForm(NextFormTag)
  → FormComponent broadcasts OnFormChanged
  → GAS: remove old form tag, add new form tag
  → GAS: remove old form's passive GE, apply new form's passive GE
  → AnimBP picks up new form tag → visual transition
  → UI updates form indicator
  → GameplayCue fires form-switch VFX/SFX
```

### Base Form Importance

Base form (`Form.Base`) has **no special status effect** but highest raw damage scaling. Upgrades purchased throughout the game improve Base form's `BaseDamage` attribute permanently. This means:

- **Early game:** Forms > Base (status effects provide crowd control and burst)
- **Mid game:** Forms ≈ Base (upgrades closing the gap)
- **Late game:** Base > Forms in raw DPS (fully upgraded), but forms remain useful for utility/status

### Overdrive × Form Interaction

When Overdrive is active, the shot ability checks both `Action.OverdriveActive` tag and `FormComponent->GetActiveFormTag()`:

```cpp
// In GA_DragonBaseShot::ActivateAbility()
const UDragonFormDataAsset* FormData = FormComp->GetActiveFormData();
bool bOverdrive = ASC->HasMatchingGameplayTag(Tag_OverdriveActive);

TSubclassOf<ADragonProjectile> ProjectileToSpawn;
if (bOverdrive && FormData->OverdriveProjectileClass)
    ProjectileToSpawn = FormData->OverdriveProjectileClass;
else
    ProjectileToSpawn = FormData->ProjectileClass;

float DamageMult = bOverdrive ? FormData->OverdriveDamageMultiplier : 1.0f;
```

### Charge Behavior Per Form

Each `UDragonFormDataAsset` defines `ChargeTime` and `ChargeProjectileClass`. The charge ability `GA_DragonChargeShot` reads these:

- **Base:** Fast charge (0.8s), large single projectile, high damage
- **Fire:** Medium charge (1.2s), explosive fireball, AoE on impact
- **Ice:** Slow charge (1.5s), piercing ice lance, freezes on threshold
- **Electric:** Fast charge (0.6s), chain lightning, bounces between enemies

---

## 8. Progression and Game Structure

### World Structure

```
Main Menu → Save Slot Select → Hub

Hub (persistent level)
├── World Map Portal → World Map (UI overlay or sub-level)
│   ├── Region 1 (unlocked from start)
│   │   ├── Level 1-1 (tutorial)
│   │   ├── Level 1-2
│   │   ├── Level 1-3
│   │   └── Boss 1-B
│   ├── Region 2 (unlocked after Boss 1-B)
│   │   ├── Level 2-1
│   │   └── ...
│   └── Region N
├── Upgrade Shop NPC
├── Form Shrine (swap/preview forms)
├── Bestiary Terminal
└── Secret Vault (unlocked collectibles)
```

### Progression Data Model

```cpp
USTRUCT(BlueprintType)
struct FPlayerProgressionData
{
    GENERATED_BODY()
    
    // Region/Level unlock state
    UPROPERTY(SaveGame)
    TMap<FGameplayTag, bool> UnlockedRegions; // Region.1, Region.2...
    
    UPROPERTY(SaveGame)
    TMap<FGameplayTag, ELevelCompletionStatus> LevelStates;
    
    // Forms
    UPROPERTY(SaveGame)
    TSet<FGameplayTag> UnlockedForms;
    
    // Upgrades
    UPROPERTY(SaveGame)
    TSet<FGameplayTag> PurchasedUpgrades;
    
    // Collectibles
    UPROPERTY(SaveGame)
    TSet<FName> CollectedSecrets; // per-level secret IDs
    
    // Currency
    UPROPERTY(SaveGame)
    int32 UpgradeCurrency = 0;
    
    // Stats
    UPROPERTY(SaveGame)
    int32 TotalDeaths = 0;
    
    UPROPERTY(SaveGame)
    TMap<FGameplayTag, float> BestTimes; // per-level
    
    // Last checkpoint (for continue)
    UPROPERTY(SaveGame)
    FCheckpointSaveData LastCheckpoint;
};

UENUM(BlueprintType)
enum class ELevelCompletionStatus : uint8
{
    Locked,
    Unlocked,
    Completed,
    Perfected // all secrets found
};
```

### Checkpoint System

```cpp
UCLASS()
class ACheckpointActor : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditInstanceOnly, Category="Checkpoint")
    FGameplayTag CheckpointTag; // unique per level
    
    UPROPERTY(EditInstanceOnly, Category="Checkpoint")
    int32 CheckpointIndex = 0; // ordering within level
    
    UPROPERTY(EditAnywhere, Category="Checkpoint")
    bool bHealOnActivation = true;
    
    void ActivateCheckpoint(ADragonCharacter* Dragon);
    
protected:
    UFUNCTION()
    void OnOverlap(AActor* OverlappedActor, AActor* OtherActor);
    
    bool bActivated = false;
};
```

### Upgrade System

Upgrades are `UPrimaryDataAsset`:

```cpp
UCLASS()
class UUpgradeDataAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FGameplayTag UpgradeTag;
    
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FText DisplayName;
    
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FText Description;
    
    UPROPERTY(EditDefaultsOnly, Category="Cost")
    int32 Cost = 100;
    
    UPROPERTY(EditDefaultsOnly, Category="Requirements")
    TArray<FGameplayTag> RequiredUpgrades; // prerequisites
    
    UPROPERTY(EditDefaultsOnly, Category="Effect")
    TSubclassOf<UGameplayEffect> PermanentEffect; // applied on purchase
    
    UPROPERTY(EditDefaultsOnly, Category="Effect")
    TSubclassOf<UGameplayAbility> GrantedAbility; // if this upgrade unlocks ability
    
    UPROPERTY(EditDefaultsOnly, Category="UI")
    TObjectPtr<UTexture2D> Icon;
};
```

### Save/Load Flow

```
Save: GameInstance->CurrentSave->ProgressionData = Subsystem->GetProgressionData()
      → UGameplayStatics::SaveGameToSlot()

Load: UGameplayStatics::LoadGameFromSlot()
      → GameInstance->CurrentSave populated
      → GameInstance->CachedProgression populated
      → On level load: Subsystem reads cached data → grants abilities/upgrades to character
```

---

## 9. Enemy and Boss Architecture

### Enemy Hierarchy

```
AEnemyBase : ACharacter (C++)
├── UAbilitySystemComponent
├── UEnemyAttributeSet
├── UStateTreeComponent
├── Health, hit reactions, death, drops
│
├── AEnemyRanged : AEnemyBase     ← shoots projectiles, keeps distance
├── AEnemyMelee : AEnemyBase      ← charges player, melee swings
├── AEnemyFlying : AEnemyBase     ← airborne, swooping attacks
├── AEnemyShielded : AEnemyBase   ← frontal shield, must flank/charge-break
├── AEnemyElite : AEnemyBase      ← modifier system (faster, armored, status-immune)
└── ABossBase : AEnemyBase        ← phase system, arena logic, scripted sequences
```

### AI Architecture: StateTree

ЧОМУ StateTree over BehaviorTree: UE 5.7 actively develops StateTree, it offers better visual debugging, cleaner state transitions, and first-class support for gameplay tasks. Recommended for new projects.

```cpp
// AEnemyBase constructor
StateTreeComponent = CreateDefaultSubobject<UStateTreeComponent>(TEXT("StateTree"));
PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception"));
```

**Perception setup:**
- `UAISense_Sight` — primary detection (side-view range, narrow cone since action is lateral)
- `UAISense_Damage` — react to being hit even outside sight

### Enemy Archetype Data

```cpp
UCLASS()
class UEnemyArchetypeAsset : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditDefaultsOnly, Category="Identity")
    FGameplayTag EnemyTypeTag;
    
    UPROPERTY(EditDefaultsOnly, Category="Spawning")
    TSubclassOf<AEnemyBase> EnemyClass;
    
    UPROPERTY(EditDefaultsOnly, Category="Stats")
    float BaseHealth = 3.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Stats")
    float BaseDamage = 1.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Stats")
    float MoveSpeed = 400.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="AI")
    TObjectPtr<UStateTree> BehaviorTree;
    
    UPROPERTY(EditDefaultsOnly, Category="Combat")
    TArray<TSubclassOf<UGameplayAbility>> Abilities;
    
    UPROPERTY(EditDefaultsOnly, Category="Rewards")
    int32 CurrencyDrop = 10;
    
    UPROPERTY(EditDefaultsOnly, Category="Rewards")
    float OverdriveEnergyDrop = 5.0f;
    
    UPROPERTY(EditDefaultsOnly, Category="Modifiers")
    TArray<FGameplayTag> Immunities; // Status.Frozen immune, etc.
};
```

### Boss Phase System

```cpp
USTRUCT(BlueprintType)
struct FBossPhaseData
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    FGameplayTag PhaseTag; // Boss.Phase.1, Boss.Phase.2
    
    UPROPERTY(EditDefaultsOnly, meta=(ClampMin=0, ClampMax=1))
    float HealthThreshold; // transition when HP% <= this (1.0, 0.6, 0.3)
    
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UStateTree> PhaseBehavior;
    
    UPROPERTY(EditDefaultsOnly)
    TArray<TSubclassOf<UGameplayAbility>> PhaseAbilities;
    
    UPROPERTY(EditDefaultsOnly)
    TObjectPtr<UAnimMontage> PhaseTransitionMontage;
    
    UPROPERTY(EditDefaultsOnly)
    bool bInvulnerableDuringTransition = true;
};

UCLASS()
class ABossBase : public AEnemyBase
{
    GENERATED_BODY()
public:
    void CheckPhaseTransition();
    
protected:
    UPROPERTY(EditDefaultsOnly, Category="Boss")
    TArray<FBossPhaseData> Phases;
    
    UPROPERTY(VisibleAnywhere, Category="Boss")
    int32 CurrentPhaseIndex = 0;
    
    void TransitionToPhase(int32 PhaseIndex);
};
```

### Encounter Volumes

```cpp
UCLASS()
class AEncounterVolume : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditInstanceOnly, Category="Encounter")
    TArray<FEncounterWave> Waves;
    
    UPROPERTY(EditInstanceOnly, Category="Encounter")
    bool bLockCameraToArena = false;
    
    UPROPERTY(EditInstanceOnly, Category="Encounter")
    bool bBlockExitUntilCleared = true;
    
    void StartEncounter();
    void OnWaveCleared();
    void OnEncounterComplete();
};

USTRUCT(BlueprintType)
struct FEncounterWave
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    TArray<FEnemySpawnEntry> Enemies;
    
    UPROPERTY(EditAnywhere)
    float DelayBeforeWave = 1.0f;
};

USTRUCT(BlueprintType)
struct FEnemySpawnEntry
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere)
    TObjectPtr<UEnemyArchetypeAsset> Archetype;
    
    UPROPERTY(EditAnywhere)
    TObjectPtr<AActor> SpawnPoint; // ATargetPoint placed in level
    
    UPROPERTY(EditAnywhere)
    float SpawnDelay = 0.0f;
};
```

---

## 10. Data-Driven Content Pipeline

### What Goes Where

| Layer | Contents | ЧОМУ |
|---|---|---|
| **C++** | Base classes, GAS components, CMC, interfaces, core systems | Performance, type safety, architecture |
| **Blueprint** | Derived characters (visuals), specific projectile VFX, level scripts | Iteration speed, artist-friendly |
| **PrimaryDataAsset** | FormDefs, EnemyArchetypes, UpgradeDefs, AbilitySets, BossPhases | Data-driven, hot-reloadable, designer-owned |
| **DataTable** | Damage tuning, economy numbers, difficulty curves | Spreadsheet-friendly bulk data |
| **AnimBP** | State machines, blend spaces, montage playback | Animation-specific logic |
| **StateTree** | Enemy AI behavior, boss phase scripts | Visual state machine, designer-friendly |
| **Config structs** | Default values in DefaultGame.ini | Release-time tuning |

### Asset Type Registry

| Asset | Class | Owner |
|---|---|---|
| Dragon forms | UDragonFormDataAsset | Designer |
| Ability sets | UDragonAbilitySet | Designer + Programmer |
| Enemy archetypes | UEnemyArchetypeAsset | Designer |
| Boss definitions | UBossDataAsset (phases, arena config) | Designer |
| Upgrade definitions | UUpgradeDataAsset | Designer |
| Level metadata | ULevelMetadataAsset (region, order, rewards) | Designer |
| Region metadata | URegionDataAsset (name, boss, unlock rules) | Designer |
| Checkpoint configs | FCheckpointData (struct in level actor) | Level Designer |
| UI skin data | UUIThemeAsset | UI Artist |

---

## 11. Animation Architecture

### Anim Blueprint Structure

```
ABP_Dragon (AnimBlueprint)
├── Locomotion State Machine
│   ├── Idle
│   ├── Run (BlendSpace1D by Speed)
│   ├── Jump (JumpStart → JumpLoop → JumpLand)
│   ├── Fall
│   ├── WallSlide
│   └── Dash
├── Upper Body Layer (Linked Anim Layer)
│   ├── Idle_Upper
│   ├── Shooting (additive, driven by Action.Shooting tag)
│   ├── Charging (driven by Action.Charging tag, blend by ChargePercent)
│   └── FormSwitch (brief full-body override)
├── Hit Reaction (Montage Slot)
│   ├── HitReact_Light
│   ├── HitReact_Heavy
│   └── Launched
├── Full Body Override (Montage Slot, highest priority)
│   ├── Death
│   ├── Overdrive_Activate
│   └── BossIntro
└── Form Visual Layer
    ├── MaterialOverlay (driven by FormTag)
    └── AuraVFX attachment (driven by FormTag)
```

### Tag-Driven Animation Decisions

| GAS Tag | ABP Behavior |
|---|---|
| Movement.Grounded | Locomotion SM: Idle/Run |
| Movement.Airborne | Locomotion SM: Jump/Fall |
| Movement.WallSlide | Locomotion SM: WallSlide |
| Action.Shooting | Upper body: fire animation (additive) |
| Action.Charging | Upper body: charge loop, blend by percent |
| Action.OverdriveActive | Material pulse overlay, aura intensify |
| State.Staggered | Hit reaction montage |
| State.Dead | Death montage → ragdoll |
| Form.Fire/Ice/Electric | Material swap + aura color |

### Linked Anim Layers

Use **Linked Anim Layers** for upper body (shooting/charging) so locomotion and combat animation are independent. The lower body runs the locomotion state machine; the upper body layer handles aim/shoot/charge. This avoids combinatorial explosion of states.

### Montage Usage

| Montage | Priority | When |
|---|---|---|
| Death | Highest | State.Dead |
| Overdrive activation | High | One-shot on activate |
| Form switch | High | Brief transform VFX |
| Hit reaction | Medium | State.Staggered event |
| Charge release | Medium | ChargeShot fires |

---

## 12. Folder and Module Structure

### Source Structure

```
Source/
├── BurningCORE/                          ← Primary runtime module
│   ├── BurningCORE.Build.cs
│   ├── BurningCORE.h / .cpp              ← Module definition, log categories
│   │
│   ├── Core/                             ← Game framework
│   │   ├── BurningCOREGameInstance.h/.cpp
│   │   ├── BurningCOREGameMode.h/.cpp
│   │   ├── BurningCOREGameState.h/.cpp
│   │   ├── BurningCOREPlayerController.h/.cpp
│   │   └── BurningCOREPlayerState.h/.cpp
│   │
│   ├── Character/                        ← Dragon + base
│   │   ├── DragonCharacter.h/.cpp
│   │   ├── SideViewMovementComponent.h/.cpp
│   │   ├── DragonFormComponent.h/.cpp
│   │   └── DragonOverdriveComponent.h/.cpp
│   │
│   ├── GAS/                              ← Gameplay Ability System
│   │   ├── Abilities/
│   │   │   ├── GA_DragonBaseShot.h/.cpp
│   │   │   ├── GA_DragonChargeShot.h/.cpp
│   │   │   ├── GA_FormSwitch.h/.cpp
│   │   │   ├── GA_OverdriveActivate.h/.cpp
│   │   │   ├── GA_Jump.h/.cpp
│   │   │   ├── GA_Dash.h/.cpp
│   │   │   ├── GA_WallJump.h/.cpp
│   │   │   └── GA_HitReaction.h/.cpp
│   │   ├── Attributes/
│   │   │   ├── DragonAttributeSet.h/.cpp
│   │   │   └── EnemyAttributeSet.h/.cpp
│   │   ├── Effects/                      ← GE blueprints live in Content
│   │   ├── DragonAbilitySet.h/.cpp       ← Data asset: grant abilities + effects
│   │   └── BurningCOREAbilitySystemGlobals.h/.cpp
│   │
│   ├── Combat/                           ← Projectiles, damage
│   │   ├── DragonProjectile.h/.cpp
│   │   ├── EnemyProjectile.h/.cpp
│   │   └── DamageCalculation.h/.cpp      ← UGameplayEffectExecutionCalculation
│   │
│   ├── AI/                               ← Enemy AI
│   │   ├── EnemyBase.h/.cpp
│   │   ├── EnemyRanged.h/.cpp
│   │   ├── EnemyMelee.h/.cpp
│   │   ├── EnemyFlying.h/.cpp
│   │   ├── EnemyShielded.h/.cpp
│   │   ├── EnemyElite.h/.cpp
│   │   ├── BossBase.h/.cpp
│   │   └── StateTreeTasks/               ← Custom ST tasks
│   │       ├── STTask_AttackPlayer.h/.cpp
│   │       ├── STTask_Patrol.h/.cpp
│   │       └── STTask_Retreat.h/.cpp
│   │
│   ├── Systems/                          ← Game systems
│   │   ├── ProgressionSubsystem.h/.cpp
│   │   ├── CheckpointActor.h/.cpp
│   │   ├── EncounterVolume.h/.cpp
│   │   ├── CameraZoneVolume.h/.cpp
│   │   └── BurningCORESaveGame.h/.cpp
│   │
│   ├── Data/                             ← Data asset classes
│   │   ├── DragonFormDataAsset.h/.cpp
│   │   ├── EnemyArchetypeAsset.h/.cpp
│   │   ├── UpgradeDataAsset.h/.cpp
│   │   ├── LevelMetadataAsset.h/.cpp
│   │   ├── RegionDataAsset.h/.cpp
│   │   └── BossDataAsset.h/.cpp
│   │
│   ├── UI/                               ← C++ UI base classes
│   │   ├── BurningCOREHUD.h/.cpp
│   │   ├── DragonHealthBar.h/.cpp
│   │   ├── OverdriveGauge.h/.cpp
│   │   ├── FormIndicator.h/.cpp
│   │   ├── BossHealthBar.h/.cpp
│   │   └── UpgradeShopWidget.h/.cpp
│   │
│   └── Interfaces/                       ← UE interfaces
│       ├── Damageable.h/.cpp
│       ├── Interactable.h/.cpp
│       └── FormUser.h/.cpp
│
├── BurningCORE.Target.cs
└── BurningCOREEditor.Target.cs
```

### Content Structure

```
Content/
├── Dragon/
│   ├── Meshes/
│   ├── Animations/
│   │   ├── ABP_Dragon
│   │   ├── Montages/
│   │   └── BlendSpaces/
│   ├── Materials/
│   ├── VFX/
│   ├── Forms/                    ← DA_Form_Fire, DA_Form_Ice...
│   └── BP_DragonCharacter        ← BP derived from C++ ADragonCharacter
│
├── Enemies/
│   ├── Common/                   ← shared materials, VFX
│   ├── Ranged/                   ← mesh, anim, BP, archetype asset
│   ├── Melee/
│   ├── Flying/
│   ├── Shielded/
│   ├── Elite/
│   └── Bosses/
│       ├── Boss_Tutorial/
│       └── Boss_Region1/
│
├── GAS/
│   ├── Effects/                  ← GE_ blueprint assets
│   ├── Cues/                     ← GameplayCue_ assets
│   └── AbilitySets/              ← DA_AbilitySet_ assets
│
├── Projectiles/
│   ├── Dragon/                   ← BP_Projectile_Fire, etc.
│   └── Enemy/
│
├── Levels/
│   ├── Hub/
│   ├── Region1/
│   │   ├── L_R1_Level1
│   │   ├── L_R1_Level2
│   │   └── L_R1_Boss
│   └── Region2/
│
├── UI/
│   ├── HUD/
│   ├── Menus/
│   ├── Shop/
│   └── WorldMap/
│
├── Systems/
│   ├── Input/                    ← IMC_, IA_ assets
│   ├── DataTables/
│   └── SaveGame/
│
├── Environment/
│   ├── Tiles/
│   ├── Props/
│   ├── Backgrounds/
│   └── Hazards/
│
└── Dev/                          ← Test/prototype content (excluded from builds)
    ├── TestMaps/
    └── DebugTools/
```
