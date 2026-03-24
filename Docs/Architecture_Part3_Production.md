# BurningCORE — Game Architecture (Part 3: Production)

> **Sections 13-17 + Final Blocks: Standards, Vertical Slice, Implementation, Scaffold, Risks**

---

## 13. Coding Standards and Project Rules

### Naming Conventions

| Category | Convention | Example |
|---|---|---|
| C++ classes | UE prefix (A/U/F/E/I) + PascalCase | `ADragonCharacter`, `UDragonFormComponent` |
| Source files | PascalCase, name = class minus prefix | `DragonCharacter.h/.cpp` |
| GameplayTags | Dot-separated hierarchy, PascalCase segments | `Ability.Shoot.Base`, `Form.Fire` |
| Abilities (GA) | `GA_` prefix | `GA_DragonBaseShot` |
| Effects (GE) | `GE_` prefix | `GE_ApplyBurning` |
| Cues (GC) | `GameplayCue.` tag namespace | `GameplayCue.Dragon.Shoot.Fire` |
| Data Assets | `DA_` prefix | `DA_Form_Fire`, `DA_Enemy_Ranged` |
| Blueprints | `BP_` prefix | `BP_DragonCharacter`, `BP_EnemyRanged` |
| Widgets | `W_` or `WBP_` prefix | `WBP_DragonHUD`, `WBP_BossHealthBar` |
| Materials | `M_` / instances `MI_` | `M_Dragon_Base`, `MI_FireOverlay` |
| Textures | `T_` prefix | `T_Dragon_Diffuse` |
| Niagara | `NS_` prefix | `NS_FireAura`, `NS_IceTrail` |
| AnimMontages | `AM_` prefix | `AM_Dragon_ChargeRelease` |
| AnimBP | `ABP_` prefix | `ABP_Dragon` |
| BlendSpaces | `BS_` prefix | `BS_Dragon_Locomotion` |
| Input Actions | `IA_` prefix | `IA_Move`, `IA_Shoot`, `IA_Jump` |
| Input Mapping Contexts | `IMC_` prefix | `IMC_Platformer` |
| Levels | `L_` prefix | `L_R1_Level1`, `L_Hub` |
| StateTree | `ST_` prefix | `ST_Enemy_Ranged`, `ST_Boss_Tutorial` |
| Enums | `E` prefix | `ELevelCompletionStatus` |

### Gameplay Tag Naming Rules

```
Namespace.Category.Specific

✅ Ability.Shoot.Base
✅ Status.Burning.Stack1
✅ Boss.Phase.2
✅ Event.Hit.Dealt

❌ ShootAbility            (no hierarchy)
❌ ability_shoot_base      (snake_case)
❌ Ability.Dragon.Shoot    (don't embed character name — tags should be reusable)
```

Exception: character-exclusive tags use character namespace only when needed for filtering: `Dragon.FormSwitch`, `Dragon.Overdrive`.

### Blueprint Usage Policy

| ✅ Use Blueprint for | ❌ Never put in Blueprint |
|---|---|
| Derived characters (mesh, anim, VFX refs) | Core gameplay math |
| Level-specific scripts and triggers | Damage calculation |
| AnimBP state machines | Ability activation logic |
| UI widget layouts | Save/Load |
| GameplayCue visual/audio handlers | Networking logic |
| Prototype iteration on tuning values | Architecture decisions |

### Dependency Rules

```
Interfaces → depend on nothing (pure contracts)
Data Assets → depend on interfaces + core types
GAS Abilities → depend on Data Assets + components
Components → depend on interfaces + Data Assets  
Characters → depend on components + GAS
AI → depends on GAS + interfaces
UI → depends on GAS attributes (read-only bind)
Systems → depend on Data Assets + save types

❌ UI must NEVER depend on concrete Character class (use interfaces/attributes)
❌ AI must NEVER depend on UI
❌ Characters must NEVER depend on AI classes
❌ Save system must NEVER depend on runtime actors
```

### Widget Restrictions

Widgets must NOT contain:
- Gameplay logic (damage calc, ability triggering)
- Direct references to character classes (use interfaces or attribute delegates)
- Save/Load operations
- Spawning logic

Widgets SHOULD:
- Bind to GAS attributes via `AsyncTask_ListenForAttributeChange` or delegates
- Read GameplayTags for state display
- Use `UCommonActivatableWidget` pattern for menu navigation

### Character Blueprint Restrictions

Character BPs must NOT contain:
- Complex gameplay graphs (move to C++ or GAS ability)
- UI creation logic (PlayerController's job)
- Camera management (PlayerController or dedicated CameraManager)
- AI behavior (StateTree's job)

Character BPs SHOULD contain:
- Mesh/material assignments
- Sound/VFX component references
- Animation montage references
- Default property overrides from C++

---

## 14. Vertical Slice Plan

### Scope: "Playable Region 1"

**Included in vertical slice:**

| System | Scope | Status |
|---|---|---|
| Dragon character | Full movement, base shot, charge shot, 1 form (Fire) | Implement now |
| SideView CMC | Plane lock, coyote time, jump buffer, air control | Implement now |
| Camera | Side-view follow, 1 camera zone (boss arena) | Implement now |
| GAS foundation | ASC, DragonAttributeSet, Input→Ability binding | Implement now |
| Base shot ability | GA_DragonBaseShot reading from FormData | Implement now |
| Charge shot | GA_DragonChargeShot with hold/release | Implement now |
| Form system | FormComponent + Form.Base + Form.Fire (2 forms) | Implement now |
| Overdrive | OverdriveComponent, activate/deactivate, drain, UI gauge | Implement now |
| Dash | GA_Dash with cooldown | Implement now |
| Jump | GA_Jump (GAS-managed for tag tracking) | Implement now |
| 3 enemy types | Ranged, Melee, Flying (with StateTree AI) | Implement now |
| 1 boss | Tutorial boss, 2 phases | Implement now |
| Encounter system | EncounterVolume with waves | Implement now |
| Checkpoints | CheckpointActor, respawn flow | Implement now |
| HUD | Health bar, Overdrive gauge, Form indicator, Boss health bar | Implement now |
| Tutorial level | L_R1_Level1 with encounters and checkpoints | Implement now |
| Hub | Minimal: spawn point + level portal | Stub |
| Save/Load | SaveGame structure, save on checkpoint, load on continue | Implement now |
| Upgrade system | 2 upgrades purchasable (BaseDamage +, MaxHealth +) | Implement now |
| World map | Static UI with 1 region | Stub |

**Deferred to post-slice:**

| System | ЧОМУ defer |
|---|---|
| WallJump/WallSlide | Not core for first playable |
| Ice Form, Electric Form | 1 form sufficient for slice validation |
| Elite enemies | Modifier system is a layer atop base enemies |
| Region 2+ levels | Content, not architecture |
| Bestiary | Polish feature |
| Secret vault UI | Polish feature |
| Best times / ranking | Polish feature |
| Advanced progression tree | 2 upgrades prove the system |
| Camera rails | Nice-to-have, zone volumes sufficient |

---

## 15. Step-by-Step Implementation Order

### Phase 0: Project Foundation (Week 1)

```
0.1  Clean Third Person Template — remove template-specific code
0.2  Enable GAS plugin (GameplayAbilities, GameplayTags, GameplayTasks)
0.3  Configure AbilitySystemGlobals in DefaultGame.ini
0.4  Create module structure (folders in Source/BurningCORE/)
0.5  Create log categories (LogBurningCORE, LogDragon, LogCombat, LogAI)
0.6  Create base interfaces: IDamageable, IInteractable
0.7  Create GameInstance, GameMode, GameState, PlayerController, PlayerState shells
0.8  Create BurningCORESaveGame with FPlayerProgressionData struct
0.9  Setup Enhanced Input: IMC_Platformer + all IA_ assets
0.10 Create test map: L_Dev_Sandbox (flat ground, test volumes)
```

### Phase 1: Dragon Core (Week 2)

```
1.1  USideViewMovementComponent — plane lock, jump feel params
1.2  ADragonCharacter — component setup, ASC, AttributeSet
1.3  Camera setup — side-view SpringArm, follow behavior
1.4  Basic locomotion — run, jump, fall (no GAS yet, raw input)
1.5  DragonAttributeSet — Health, MaxHealth, OverdriveEnergy, BaseDamage
1.6  Input→GAS binding — GA_Jump (managed jump for tags)
1.7  GA_Dash — input, cooldown tag, movement impulse
1.8  ABP_Dragon — locomotion SM (Idle, Run, Jump, Fall, Dash)
1.9  Test: Dragon moves, jumps, dashes in side-view on L_Dev_Sandbox
```

### Phase 2: Combat Foundation (Week 3)

```
2.1  ADragonProjectile — base class with collision, lifetime, pooling hook
2.2  UDragonFormDataAsset — data asset with projectile refs, charge time
2.3  UDragonFormComponent — form registry, active form, switching
2.4  DA_Form_Base — base form data asset (default projectile)
2.5  GA_DragonBaseShot — reads FormData, spawns projectile, applies GE
2.6  GE_DragonBaseShotDamage — instant damage via IncomingDamage
2.7  GA_DragonChargeShot — hold/release, charge timer, reads FormData
2.8  ABP_Dragon update — shooting (additive upper body), charging
2.9  Test: Dragon shoots, charges, projectiles deal damage to debug target
```

### Phase 3: Enemies (Week 4)

```
3.1  AEnemyBase — ASC, EnemyAttributeSet, StateTreeComponent, hit reaction
3.2  IDamageable implementation — AEnemyBase::ApplyDamage via GAS
3.3  UEnemyArchetypeAsset — data asset class
3.4  AEnemyRanged — keeps distance, fires projectiles (basic ST)
3.5  AEnemyMelee — approaches player, melee attack (basic ST)
3.6  AEnemyFlying — airborne patrol, swoop attack (basic ST)
3.7  ST_Enemy_Ranged / Melee / Flying — basic StateTree behaviors
3.8  DA_Enemy_Ranged / Melee / Flying — archetype data assets
3.9  Enemy death: drop currency, drop overdrive energy, VFX
3.10 Test: 3 enemy types behave correctly, take damage, die
```

### Phase 4: Forms + Overdrive (Week 5)

```
4.1  DA_Form_Fire — fire projectile class, GE_ApplyBurning
4.2  GA_FormSwitch — cycle Base↔Fire, cooldown
4.3  Status stacking — GE_BurningStack, threshold burst logic
4.4  UDragonOverdriveComponent — fill on kill/hit, drain on active
4.5  GA_OverdriveActivate — threshold check, apply buff GE, tag
4.6  Overdrive projectile variants (base+fire overdrive projectiles)
4.7  UI: WBP_DragonHUD (health, overdrive gauge, form indicator)
4.8  ABP_Dragon update — form visuals, overdrive aura
4.9  Test: form switching, status stacking, overdrive cycle
```

### Phase 5: Game Systems (Week 6)

```
5.1  ACheckpointActor — activation, heal, save position
5.2  AEncounterVolume — wave spawning, camera lock, exit block
5.3  ACameraZoneVolume — param switching on overlap
5.4  Respawn flow — GameMode respawns at last checkpoint
5.5  UProgressionSubsystem — upgrade purchase, form unlock tracking
5.6  UUpgradeDataAsset — 2 upgrades (damage+, health+)
5.7  Save/Load — GameInstance manages save slots
5.8  Test: checkpoint, respawn, encounter flow, save/load
```

### Phase 6: Boss + Level (Week 7)

```
6.1  ABossBase — phase system, health gates, invuln transitions
6.2  UBossDataAsset — phase definitions
6.3  ST_Boss_Tutorial — 2-phase behavior
6.4  Boss arena — encounter volume + camera lock + boundaries
6.5  WBP_BossHealthBar — phase indicators
6.6  L_R1_Level1 — tutorial level with encounters, checkpoints, boss
6.7  L_Hub — minimal spawn + portal to L_R1_Level1
6.8  Hub→Level→Hub flow via GameMode
6.9  Test: full playthrough Hub → Level → Boss → Return to Hub
```

### Phase 7: Polish Pass (Week 8)

```
7.1  Jump feel tuning — coyote, buffer, apex gravity, air control
7.2  Camera feel — look-ahead, smooth follow, boss zoom
7.3  Hit feedback — screen shake, hitstop, VFX
7.4  Enemy telegraph readability
7.5  UI polish — animations, transitions
7.6  Audio hooks — placeholder SFX
7.7  Full vertical slice playtest
```

---

## 16. Minimal Starter Scaffold

### Initial Class Creation List

| # | Class | Header | Parent | Key Components/Notes |
|---|---|---|---|---|
| 1 | `UBurningCOREGameInstance` | `Core/BurningCOREGameInstance.h` | `UGameInstance` | SaveGame management |
| 2 | `ABurningCOREGameMode` | `Core/BurningCOREGameMode.h` | `AGameModeBase` | Spawn, respawn, level flow |
| 3 | `ABurningCOREGameState` | `Core/BurningCOREGameState.h` | `AGameState` | Level runtime state |
| 4 | `ABurningCOREPlayerController` | `Core/BurningCOREPlayerController.h` | `APlayerController` | Input, camera, HUD |
| 5 | `ABurningCOREPlayerState` | `Core/BurningCOREPlayerState.h` | `APlayerState` | Session stats |
| 6 | `ADragonCharacter` | `Character/DragonCharacter.h` | `ACharacter` | ASC, FormComp, OverdriveComp, SideViewCMC |
| 7 | `USideViewMovementComponent` | `Character/SideViewMovementComponent.h` | `UCharacterMovementComponent` | Plane lock, jump feel |
| 8 | `UDragonFormComponent` | `Character/DragonFormComponent.h` | `UActorComponent` | Form registry, switching |
| 9 | `UDragonOverdriveComponent` | `Character/DragonOverdriveComponent.h` | `UActorComponent` | Overdrive resource |
| 10 | `UDragonAttributeSet` | `GAS/Attributes/DragonAttributeSet.h` | `UAttributeSet` | Health, Overdrive, Damage |
| 11 | `UEnemyAttributeSet` | `GAS/Attributes/EnemyAttributeSet.h` | `UAttributeSet` | Health, Damage |
| 12 | `UGA_DragonBaseShot` | `GAS/Abilities/GA_DragonBaseShot.h` | `UGameplayAbility` | Form-driven shot |
| 13 | `UGA_DragonChargeShot` | `GAS/Abilities/GA_DragonChargeShot.h` | `UGameplayAbility` | Hold/release charge |
| 14 | `UGA_FormSwitch` | `GAS/Abilities/GA_FormSwitch.h` | `UGameplayAbility` | Cycle forms |
| 15 | `UGA_OverdriveActivate` | `GAS/Abilities/GA_OverdriveActivate.h` | `UGameplayAbility` | Threshold check, activate |
| 16 | `UGA_Jump` | `GAS/Abilities/GA_Jump.h` | `UGameplayAbility` | Tag-tracked jump |
| 17 | `UGA_Dash` | `GAS/Abilities/GA_Dash.h` | `UGameplayAbility` | Cooldown dash |
| 18 | `UGA_HitReaction` | `GAS/Abilities/GA_HitReaction.h` | `UGameplayAbility` | Stagger handler |
| 19 | `ADragonProjectile` | `Combat/DragonProjectile.h` | `AActor` | Collision, movement, GE specs |
| 20 | `AEnemyProjectile` | `Combat/EnemyProjectile.h` | `AActor` | Simpler enemy variant |
| 21 | `AEnemyBase` | `AI/EnemyBase.h` | `ACharacter` | ASC, StateTree, perception |
| 22 | `AEnemyRanged` | `AI/EnemyRanged.h` | `AEnemyBase` | Ranged behavior |
| 23 | `AEnemyMelee` | `AI/EnemyMelee.h` | `AEnemyBase` | Melee behavior |
| 24 | `AEnemyFlying` | `AI/EnemyFlying.h` | `AEnemyBase` | Flying behavior |
| 25 | `ABossBase` | `AI/BossBase.h` | `AEnemyBase` | Phase system |
| 26 | `UDragonFormDataAsset` | `Data/DragonFormDataAsset.h` | `UPrimaryDataAsset` | Form definition |
| 27 | `UEnemyArchetypeAsset` | `Data/EnemyArchetypeAsset.h` | `UPrimaryDataAsset` | Enemy config |
| 28 | `UUpgradeDataAsset` | `Data/UpgradeDataAsset.h` | `UPrimaryDataAsset` | Upgrade definition |
| 29 | `UBurningCORESaveGame` | `Systems/BurningCORESaveGame.h` | `USaveGame` | Progression persistence |
| 30 | `UProgressionSubsystem` | `Systems/ProgressionSubsystem.h` | `UWorldSubsystem` | Upgrade/form/region state |
| 31 | `ACheckpointActor` | `Systems/CheckpointActor.h` | `AActor` | Checkpoint logic |
| 32 | `AEncounterVolume` | `Systems/EncounterVolume.h` | `AActor` | Wave spawning |
| 33 | `ACameraZoneVolume` | `Systems/CameraZoneVolume.h` | `ATriggerVolume` | Camera param zones |
| 34 | `IDamageable` | `Interfaces/Damageable.h` | `UInterface` | ApplyDamage, HandleDeath |
| 35 | `IInteractable` | `Interfaces/Interactable.h` | `UInterface` | Interact, CanInteract |

### Key Interfaces

```cpp
// IDamageable — universal damage contract
UINTERFACE(MinimalAPI, NotBlueprintable)
class UDamageable : public UInterface { GENERATED_BODY() };
class IDamageable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Damage")
    virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec,
        const FHitResult& HitResult) = 0;
    
    UFUNCTION(BlueprintCallable, Category="Damage")
    virtual bool IsAlive() const = 0;
};

// IInteractable — NPC/object interaction
UINTERFACE(MinimalAPI, BlueprintType)
class UInteractable : public UInterface { GENERATED_BODY() };
class IInteractable
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Interact")
    virtual void Interact(AActor* Interactor) = 0;
    
    UFUNCTION(BlueprintCallable, Category="Interact")
    virtual bool CanInteract(AActor* Interactor) const = 0;
    
    UFUNCTION(BlueprintCallable, Category="Interact")
    virtual FText GetInteractionPrompt() const = 0;
};
```

### Representative Header: DragonCharacter.h

```cpp
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "Damageable.h"
#include "DragonCharacter.generated.h"

class UAbilitySystemComponent;
class UDragonAttributeSet;
class USideViewMovementComponent;
class UDragonFormComponent;
class UDragonOverdriveComponent;
class UDragonAbilitySet;
class USpringArmComponent;
class UCameraComponent;

UCLASS(abstract)
class ADragonCharacter : public ACharacter, public IAbilitySystemInterface, public IDamageable
{
    GENERATED_BODY()
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities",
        meta=(AllowPrivateAccess="true"))
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    
    UPROPERTY()
    TObjectPtr<UDragonAttributeSet> AttributeSet;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
        meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDragonFormComponent> FormComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
        meta=(AllowPrivateAccess="true"))
    TObjectPtr<UDragonOverdriveComponent> OverdriveComponent;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
        meta=(AllowPrivateAccess="true"))
    TObjectPtr<USpringArmComponent> CameraBoom;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components",
        meta=(AllowPrivateAccess="true"))
    TObjectPtr<UCameraComponent> FollowCamera;
    
public:
    ADragonCharacter(const FObjectInitializer& ObjectInitializer);
    
    // IAbilitySystemInterface
    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
    
    // IDamageable
    virtual void ApplyDamage(const FGameplayEffectSpecHandle& DamageSpec,
        const FHitResult& HitResult) override;
    virtual bool IsAlive() const override;
    
    void InitializeAbilities(const UDragonAbilitySet* AbilitySet);
    
protected:
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
    
    UPROPERTY(EditDefaultsOnly, Category="Abilities")
    TObjectPtr<UDragonAbilitySet> DefaultAbilitySet;
    
    void OnHealthChanged(const FOnAttributeChangeData& Data);
    void HandleDeath();
    
public:
    FORCEINLINE UDragonFormComponent* GetFormComponent() const { return FormComponent; }
    FORCEINLINE UDragonOverdriveComponent* GetOverdriveComponent() const { return OverdriveComponent; }
};
```

---

## 17. Risks and Recommendations

| # | Risk | Impact | Mitigation |
|---|---|---|---|
| 1 | **Side-view 3D readability** — depth ambiguity making hits feel unfair | High | Strict Y-plane lock for player; all combat on same depth; clear visual layering; playtest with non-devs early |
| 2 | **GAS complexity creep** — over-engineering effects for simple interactions | High | Start with minimal AttributeSet (6 attrs max); resist adding attributes until proven needed; use tags for binary states, not attributes |
| 3 | **Form system overengineering** — building full 5-form system before proving 2 work | Medium | Ship vertical slice with Base + Fire only; validate data pipeline before adding forms; each new form = just a new DataAsset |
| 4 | **Projectile visual clutter** — too many particles on screen killing readability | High | Hard cap active player projectiles (6); enemy projectile cap per encounter; distinct color language (warm=player, cool/red=enemy); heavy playtesting |
| 5 | **Boss scripting complexity** — StateTree phases becoming unmanageable | Medium | Keep boss phases to 2-3 max; each phase = separate StateTree asset; avoid complex branching within phases; telegraph windows are sacred |
| 6 | **Progression/save system scope growth** — save data growing beyond maintainability | Medium | FPlayerProgressionData is the ONLY save struct; version the save format from day 1; never store runtime state in save |
| 7 | **Third Person Template residue** — template assumptions leaking into production code | Low | Phase 0 is explicit cleanup; delete all template BP content; replace GameMode/Controller with custom classes immediately |
| 8 | **CMC modification complexity** — custom CharacterMovementComponent bugs | Medium | Change minimally: plane lock + jump feel params; don't override physics simulation; use CMC's built-in `SetPlaneConstraint` when possible |
| 9 | **Animation state explosion** — too many states in ABP | Medium | Linked Anim Layers for upper/lower body separation; tag-driven transitions (not boolean spaghetti); Montage for one-shots |
| 10 | **Premature multi-hero architecture** — over-abstracting for heroes that don't exist | Medium | Design Dragon-first; keep base classes clean but don't add abstract layers "just in case"; refactor when second hero actually starts |

---

## Final Blocks

### A) Recommended Phase 1 Architecture Snapshot

```
ADragonCharacter (C++ abstract, BP derived)
├── USideViewMovementComponent (plane lock, jump feel)
├── UAbilitySystemComponent (GAS core)
├── UDragonAttributeSet (Health, Overdrive, BaseDamage, ChargeMultiplier)
├── UDragonFormComponent (Base + Fire forms via DataAssets)
├── UDragonOverdriveComponent (fill/drain/activate)
├── USpringArmComponent + UCameraComponent (side-view)
└── Input → GA_BaseShot, GA_ChargeShot, GA_Jump, GA_Dash, GA_FormSwitch, GA_Overdrive

AEnemyBase (C++ abstract, BP per archetype)
├── UAbilitySystemComponent + UEnemyAttributeSet
├── UStateTreeComponent (AI behavior)
├── UAIPerceptionComponent (Sight + Damage)
└── Derived: AEnemyRanged, AEnemyMelee, AEnemyFlying, ABossBase

ABurningCOREGameMode → spawn, respawn, level flow
ABurningCOREPlayerController → input context, camera, HUD
UBurningCOREGameInstance → save/load, global state
UProgressionSubsystem → upgrades, form unlocks
ACheckpointActor, AEncounterVolume, ACameraZoneVolume
```

### B) Minimal Folder Tree

```
Source/BurningCORE/
├── Core/          (GameInstance, GameMode, GameState, PlayerController, PlayerState)
├── Character/     (DragonCharacter, SideViewCMC, FormComponent, OverdriveComponent)
├── GAS/
│   ├── Abilities/ (GA_ classes)
│   └── Attributes/(DragonAttributeSet, EnemyAttributeSet)
├── Combat/        (DragonProjectile, EnemyProjectile)
├── AI/            (EnemyBase, EnemyRanged/Melee/Flying, BossBase, StateTreeTasks/)
├── Systems/       (SaveGame, ProgressionSubsystem, Checkpoint, Encounter, CameraZone)
├── Data/          (FormDataAsset, EnemyArchetype, UpgradeDataAsset)
├── UI/            (HUD, HealthBar, OverdriveGauge, FormIndicator, BossHealthBar)
└── Interfaces/    (Damageable, Interactable)

Content/
├── Dragon/        (Meshes, Anims, Materials, VFX, Forms, BP_DragonCharacter)
├── Enemies/       (per-type folders + Bosses/)
├── GAS/           (Effects/, Cues/, AbilitySets/)
├── Projectiles/   (Dragon/, Enemy/)
├── Levels/        (Hub/, Region1/)
├── UI/            (HUD/, Menus/, Shop/)
├── Systems/       (Input/, DataTables/)
├── Environment/   (Tiles/, Props/, Backgrounds/)
└── Dev/           (TestMaps/, DebugTools/)
```

### C) Minimal Gameplay Tag Seed List

```ini
# DefaultGameplayTags.ini or NativeGameplayTags registration

# State
+GameplayTagList=(Tag="State.Alive")
+GameplayTagList=(Tag="State.Dead")
+GameplayTagList=(Tag="State.Invulnerable")
+GameplayTagList=(Tag="State.Staggered")

# Movement
+GameplayTagList=(Tag="Movement.Grounded")
+GameplayTagList=(Tag="Movement.Airborne")

# Actions
+GameplayTagList=(Tag="Action.Shooting")
+GameplayTagList=(Tag="Action.Charging")
+GameplayTagList=(Tag="Action.Charged")
+GameplayTagList=(Tag="Action.OverdriveActive")
+GameplayTagList=(Tag="Action.FormSwitch")

# Abilities
+GameplayTagList=(Tag="Ability.Shoot.Base")
+GameplayTagList=(Tag="Ability.Shoot.Charge")
+GameplayTagList=(Tag="Ability.Movement.Jump")
+GameplayTagList=(Tag="Ability.Movement.Dash")
+GameplayTagList=(Tag="Ability.Overdrive.Activate")
+GameplayTagList=(Tag="Ability.Form.Switch")

# Forms
+GameplayTagList=(Tag="Form.Base")
+GameplayTagList=(Tag="Form.Fire")

# Status
+GameplayTagList=(Tag="Status.Burning")

# Cooldowns
+GameplayTagList=(Tag="Cooldown.Dash")
+GameplayTagList=(Tag="Cooldown.FormSwitch")

# Damage
+GameplayTagList=(Tag="Damage.Physical")
+GameplayTagList=(Tag="Damage.Fire")

# Events
+GameplayTagList=(Tag="Event.Hit.Dealt")
+GameplayTagList=(Tag="Event.Hit.Received")
+GameplayTagList=(Tag="Event.Kill")
+GameplayTagList=(Tag="Event.Death")
+GameplayTagList=(Tag="Event.Checkpoint.Reached")

# Enemy
+GameplayTagList=(Tag="Enemy.Type.Melee")
+GameplayTagList=(Tag="Enemy.Type.Ranged")
+GameplayTagList=(Tag="Enemy.Type.Flying")
+GameplayTagList=(Tag="Enemy.Type.Boss")

# Boss
+GameplayTagList=(Tag="Boss.Phase.1")
+GameplayTagList=(Tag="Boss.Phase.2")

# GameplayCues
+GameplayTagList=(Tag="GameplayCue.Dragon.Shoot")
+GameplayTagList=(Tag="GameplayCue.Dragon.ChargeRelease")
+GameplayTagList=(Tag="GameplayCue.Dragon.FormSwitch")
+GameplayTagList=(Tag="GameplayCue.Dragon.Overdrive")
+GameplayTagList=(Tag="GameplayCue.Hit.Impact")
+GameplayTagList=(Tag="GameplayCue.Status.Burning")
```

### D) Minimal Initial Class Creation List

**Phase 0 (Foundation) — 8 classes:**
1. `UBurningCOREGameInstance`
2. `ABurningCOREGameMode`
3. `ABurningCOREGameState`
4. `ABurningCOREPlayerController`
5. `ABurningCOREPlayerState`
6. `UBurningCORESaveGame`
7. `IDamageable` (interface)
8. `IInteractable` (interface)

**Phase 1 (Dragon Core) — 7 classes:**
9. `ADragonCharacter`
10. `USideViewMovementComponent`
11. `UDragonFormComponent`
12. `UDragonOverdriveComponent`
13. `UDragonAttributeSet`
14. `UGA_Jump`
15. `UGA_Dash`

**Phase 2 (Combat) — 6 classes:**
16. `ADragonProjectile`
17. `UDragonFormDataAsset`
18. `UGA_DragonBaseShot`
19. `UGA_DragonChargeShot`
20. `UGA_FormSwitch`
21. `UDragonAbilitySet`

**Phase 3 (Enemies) — 7 classes:**
22. `AEnemyBase`
23. `UEnemyAttributeSet`
24. `UEnemyArchetypeAsset`
25. `AEnemyRanged`
26. `AEnemyMelee`
27. `AEnemyFlying`
28. `AEnemyProjectile`

**Phase 4 (Forms + Overdrive) — 2 classes:**
29. `UGA_OverdriveActivate`
30. `UGA_HitReaction`

**Phase 5 (Systems) — 4 classes:**
31. `UProgressionSubsystem`
32. `ACheckpointActor`
33. `AEncounterVolume`
34. `ACameraZoneVolume`

**Phase 6 (Boss) — 3 classes:**
35. `ABossBase`
36. `UBossDataAsset`
37. `UUpgradeDataAsset`

**Total: 37 C++ classes for complete vertical slice.**
