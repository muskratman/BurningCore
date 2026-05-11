# Context: Base Enemy System

Этот документ описывает reusable Enemy-систему в плагине `CookieBrosPlatformer` и нужен как быстрый context pack для AI-агента в новом чате.

DragonSlayer использует двухслойную модель:

- `Plugins/CookieBrosPlatformer` - reusable platformer foundation;
- `Source/DragonSlayer` - project-specific gameplay, Dragon content, rewards, progression and presentation.

Enemy foundation сейчас должна поддерживать production path для базовых врагов без преждевременного расширения под гипотетические будущие кампании.

## Read First

Для работы с Enemy сначала читать эти файлы:

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/PlatformerEnemyBase.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/AI/PlatformerEnemyBase.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/PlatformerEnemyMelee.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/AI/PlatformerEnemyMelee.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/PlatformerEnemyRanged.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/AI/PlatformerEnemyRanged.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/PlatformerEnemyElite.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/AI/PlatformerEnemyElite.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/Animation/PlatformerEnemyAnimInstance.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/Animation/PlatformerEnemyAnimInstance.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/Data/PlatformerEnemyAnimDataAsset.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/Data/PlatformerEnemyArchetypeAsset.h
```

Если задача касается editor quick settings, дополнительно читать:

```text
Docs/ProjectSystems/Context_PlatformerSettings.md
Plugins/CookieBrosPlatformer/Source/CookieBrosLevelEditor/Private/PlatformerSettings/SPlatformerSettingsWidget.cpp
Plugins/CookieBrosPlatformer/Source/CookieBrosLevelEditor/Private/PlatformerSettings/PlatformerSettingsObjects.h
Plugins/CookieBrosPlatformer/Source/CookieBrosLevelEditor/Private/PlatformerSettings/PlatformerSettingsObjects.cpp
```

Если задача касается настройки Animation Blueprint или montage:

```text
Docs/EnemyAnimation_Setup.md
Docs/MeleeEnemyAnimation_Example_Setup.md
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/Animation/PlatformerEnemyAnimGameplayTags.h
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Private/Animation/AnimNotifies/AnimNotify_PlatformerEnemyAttackHit.cpp
```

## Ownership

Generic enemy behavior lives in `Plugins/CookieBrosPlatformer`.

Plugin owns:

- base enemy lifecycle;
- reusable melee/ranged/elite/flying shells;
- GAS-aware damage plumbing;
- AI perception and combat target plumbing;
- native patrol/chase movement;
- side-view enemy facing;
- enemy animation C++ contract;
- enemy montage tag mapping;
- enemy attack-hit anim notify;
- reusable enemy archetype data;
- reusable editor quick settings.

Project layer `Source/DragonSlayer` owns only project-specific additions:

- DragonSlayer-specific enemy wrappers, if they bind project-specific attribute sets or content;
- reward/progression data;
- campaign-specific enemy logic;
- DragonSlayer-only UI and save/progression integration.

Do not duplicate plugin shell classes in `Source/DragonSlayer` if inheritance is enough. Existing project classes like `AEnemyMelee` are thin wrappers over plugin classes and currently bind `UEnemyAttributeSet`; keep them thin unless the behavior is truly DragonSlayer-specific.

## Runtime Class Map

Core reusable runtime classes:

```text
APlatformerCombatCharacterBase
  -> APlatformerEnemyBase
       -> APlatformerEnemyMelee
       -> APlatformerEnemyRanged
       -> APlatformerEnemyFlying
       -> APlatformerEnemyElite
```

`APlatformerCombatCharacterBase` is the shared GAS combat shell. It owns:

- `UAbilitySystemComponent`;
- `UPlatformerCharacterAttributeSet`;
- `UWidgetComponent` for health UI;
- `IDamageable` implementation;
- damage effect spec creation;
- health/death synchronization;
- combat death tag/event plumbing.

`APlatformerEnemyBase` adds reusable enemy behavior:

- `UStateTreeComponent`;
- `UAIPerceptionComponent`;
- sight and damage senses;
- `UPlatformerPathComponent` for native patrol;
- combat target tracking;
- enemy stat setters/getters;
- patrol/chase movement;
- attack lifecycle;
- hit reaction and death montage triggers;
- enemy collision ignore with other enemies.

Subclasses own attack payloads:

- `APlatformerEnemyMelee` - melee sweep and `Anim.Enemy.Combat.MeleeAttack`;
- `APlatformerEnemyRanged` - projectile spawn and `Anim.Enemy.Combat.RangedAttack`;
- `APlatformerEnemyElite` - reusable stronger enemy with `Melee`, `Ranged`, or `Hybrid` combat profile;
- `APlatformerEnemyFlying` - movement-mode setup for flying enemies; attack behavior still comes from its inherited class unless extended.

## Runtime Assets

Reusable plugin content currently includes example assets:

```text
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerEnemyMelee.uasset
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerEnemyRanged.uasset
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerEnemyFlying.uasset
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerEnemyElite.uasset
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerBossBase.uasset
Plugins/CookieBrosPlatformer/Content/AI/Projectiles/BP_EnemyProjectile.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/ABP_PlatformerEnemy_Walking.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/ABP_PlatformerEnemy_Flying.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/DA_PlatformerEnemyAnimExample_Melee.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/DA_PlatformerEnemyAnimExample_Ranged.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/DA_PlatformerEnemyAnimExample_Flying.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/DA_PlatformerEnemyAnimExample_Elite.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/Montage/AM_Platformer_Attack_Melee.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/Montage/AM_Platformer_Death.uasset
```

## Enemy Base Lifecycle

Constructor of `APlatformerEnemyBase`:

- enables tick;
- sets `AutoPossessAI = PlacedInWorldOrSpawned`;
- uses `AAIController`;
- initializes GAS attributes from local enemy defaults;
- creates `StateTreeComponent`;
- creates `PerceptionComponent`;
- creates `PatrolPathComponent`;
- configures sight and damage perception;
- configures side-view movement orientation;
- initializes mesh facing and health widget placement.

`BeginPlay()`:

- captures `PatrolOriginLocation`;
- resets patrol runtime state;
- spawns default controller if needed;
- ignores movement collision with other `APlatformerEnemyBase` actors;
- applies local runtime settings;
- initializes from `DefaultArchetype` if assigned.

`InitializeFromArchetype()`:

- treats `UPlatformerEnemyArchetypeAsset` as a template;
- applies health, damage, hit delay, movement speed, engage/lose/attack ranges and damage effect class;
- preserves instance-edited values when they differ from class defaults;
- updates perception ranges;
- applies subclass-specific archetype data;
- starts StateTree logic if archetype has `BehaviorTree`;
- grants gameplay abilities from archetype on authority.

Note: `BehaviorTree` in `UPlatformerEnemyArchetypeAsset` is a `UStateTree*` field. It is named historically; the runtime path is StateTree.

## Runtime Tick Flow

`APlatformerEnemyBase::Tick()` runs this high-level flow:

```text
if movement is delayed by hit:
    return

if no combat target:
    patrol
    return

if target dead or beyond lose range:
    clear target
    patrol
    return

if target in attack range:
    stop movement
    face target
    try attack

if chase enabled and target is outside comfort range:
    chase target
else:
    patrol
```

Chase is movement toward the combat target. It is not an independent animation mechanic. Animation can use `bIsChasing` only as a locomotion variant flag inside normal `Move` or `FlyMove`.

## Perception And Targeting

`APlatformerEnemyBase` uses `UAIPerceptionComponent` with:

- `UAISenseConfig_Sight`;
- `UAISenseConfig_Damage`.

`HandleTargetPerceptionUpdated()` casts sensed actors to `APlatformerCombatCharacterBase`.

Target is accepted when:

- actor is a combat character;
- actor is not the enemy itself;
- actor is within `CombatEngageRange`.

Target is cleared when:

- current target is no longer sensed and is beyond `CombatLoseTargetRange`;
- target dies;
- target moves beyond `CombatLoseTargetRange`.

`SetCombatTarget()` updates `CurrentCombatTarget` and calls:

```text
OnCombatTargetChanged()
BP_OnCombatTargetChanged()
```

## Movement

Enemy movement is side-view constrained:

- movement/chase deltas ignore Y;
- walking enemies move along X;
- flying enemies can move along X/Z;
- mesh facing rotates based on X direction.

Native patrol uses `UPlatformerPathComponent`:

```text
FPlatformerPathPoint
  PointLocation - local offset from patrol origin
  PointDelay    - delay after reaching this point
  SpeedScale    - speed multiplier for segment starting at this point
```

Runtime patrol world position is:

```text
PatrolOriginLocation + PathPoints[Index].PointLocation
```

`bRepeatPath` controls whether the path loops from the last point back to the first. If disabled, patrol performs one pass and stops at the final point.

`MovementDelayOnHit` and `OnHitTakenImpulse`:

- live on `APlatformerEnemyBase`;
- are exposed through PlatformerSettings;
- apply only after non-fatal damage;
- `MovementDelayOnHit = 0` disables movement input delay;
- `OnHitTakenImpulse = 0` disables knockback;
- knockback is horizontal side-view `LaunchCharacter()` away from the damage instigator, with hit normal and facing fallbacks.

During hit movement delay the tick returns before patrol/chase input is added. The code does not repeatedly zero velocity during the whole delay, so knockback can still move the enemy.

## Attack Lifecycle

The attack lifecycle is split into two production-facing steps:

```text
TryAttackTarget()
  -> CanAttackTarget()
  -> StartAttackAnimation()
       stores PendingAttackTarget
       stores PendingAttackAnimationTag
       plays montage by tag
       schedules fallback hit timer
  -> LastAttackWorldTime updated

AnimNotify_PlatformerEnemyAttackHit
  -> ApplyPendingAttackHit()
       -> ApplyAttackHit()
```

`StartAttackAnimation()` owns authored timing setup. `ApplyAttackHit()` owns actual gameplay payload and is overridden by subclasses.

If no montage is found, `StartAttackAnimation()` immediately calls `ApplyPendingAttackHit()`. If montage plays but no notify fires, `AttackHitFallbackDelay` can still apply the pending hit. Final production timing should use `UAnimNotify_PlatformerEnemyAttackHit` on the impact frame.

Attack blocking rules:

- enemy must be alive;
- target must be alive;
- target must be within attack range;
- no pending attack is already in progress;
- attack montage for the relevant tag must not already be playing;
- cooldown from attributes must have elapsed.

## Melee Attack

`APlatformerEnemyMelee` overrides:

- `GetAttackRange()`;
- `GetAttackCooldown()`;
- `GetAttackDamageAmount()`;
- `ApplyAttackHit()`;
- `GetAttackAnimationTagForTarget()`.

Melee damage path:

```text
Anim.Enemy.Combat.MeleeAttack montage
  -> Platformer Enemy Attack Hit notify
  -> APlatformerEnemyBase::ApplyPendingAttackHit()
  -> APlatformerEnemyMelee::ApplyAttackHit()
  -> sphere sweep on ECC_Pawn
  -> ApplyCombatDamageToActor()
```

If the sweep does not find the target but the target is still valid, melee uses a fallback hit result at the target location and applies damage. This keeps prototype melee reliable while animation/collision timing is being refined.

## Ranged Attack

`APlatformerEnemyRanged` overrides:

- `GetAttackRange()`;
- `GetAttackCooldown()`;
- `GetAttackDamageAmount()`;
- `ApplyAttackHit()`;
- `GetAttackAnimationTagForTarget()`;
- `ApplyArchetypeCombatData()`.

Ranged damage path:

```text
Anim.Enemy.Combat.RangedAttack montage
  -> Platformer Enemy Attack Hit notify
  -> APlatformerEnemyRanged::ApplyAttackHit()
  -> spawn AEnemyProjectile
  -> initialize projectile with outgoing GAS damage spec
```

Projectile spawn uses `ProjectileSpawnSocketName` when a valid mesh socket exists. Otherwise it uses actor location plus forward/up offsets. Projectile max distance is applied after spawn.

## Elite Enemy

`APlatformerEnemyElite` is a reusable strengthened non-boss shell with:

```text
EPlatformerEnemyCombatProfile::Melee
EPlatformerEnemyCombatProfile::Ranged
EPlatformerEnemyCombatProfile::Hybrid
```

Hybrid resolves mode by distance:

- target within melee range -> melee;
- otherwise -> ranged.

Elite stores the pending attack tag before hit application so the hit payload matches the montage that started, even if distance changes before the notify frame.

Elite also applies default larger capsule and mesh scale/location.

## Flying Enemy

`APlatformerEnemyFlying` sets:

- gravity scale to `0`;
- default land movement mode to `MOVE_Flying`;
- runtime movement mode to `MOVE_Flying` on `BeginPlay()`.

Flying is currently a movement foundation. It does not introduce a separate attack payload by itself. If a flying enemy needs melee/ranged payloads, prefer a minimal subclass that reuses the existing attack lifecycle instead of duplicating base flow.

## Damage And Death

Damage starts in `APlatformerCombatCharacterBase`:

```text
ApplyCombatDamageToActor()
  -> target IDamageable::ApplyDamage()
  -> AbilitySystemComponent applies GameplayEffectSpec
  -> Health change detected
  -> OnCombatDamageReceived()
  -> SyncCombatLifeStateFromAttributes()
```

`APlatformerEnemyBase::OnCombatDamageReceived()` adds enemy-specific behavior:

- ignores follow-up hit reaction work if enemy is already dead;
- applies movement delay and knockback;
- sets damage instigator as combat target when it is a combat character;
- if damage is at least `StaggerThreshold`, clears pending attack, plays `Anim.Enemy.Combat.HitReaction`, and sends GAS event `Event.Combat.HitReceived`.

Death path:

```text
Health <= 0
  -> APlatformerCombatCharacterBase::OnCombatDeath()
  -> adds State.Combat.Dead loose tag
  -> sends Event.Combat.Death
  -> disables movement
  -> APlatformerEnemyBase::OnCombatDeath()
       clears pending attack
       clears combat target
       stops StateTree
       disables perception tick
       plays Anim.Enemy.Combat.Death montage
       delays Destroy() until montage duration if montage played
```

Fatal hit does not play hit reaction separately. It goes to death.

## Animation System

Enemy animation is intentionally separate from playable character animation.

Main C++ classes:

```text
UPlatformerEnemyAnimInstance
UPlatformerEnemyAnimDataAsset
PlatformerEnemyAnimGameplayTags
UAnimNotify_PlatformerEnemyAttackHit
```

`UPlatformerEnemyAnimDataAsset` maps enemy animation tags to montages:

```text
AnimTag
Montage
DefaultPlayRate
```

Current enemy tags:

```text
Anim.Enemy.Movement.Jump
Anim.Enemy.Movement.Fall
Anim.Enemy.Movement.Land
Anim.Enemy.Movement.Fly
Anim.Enemy.Combat.MeleeAttack
Anim.Enemy.Combat.RangedAttack
Anim.Enemy.Combat.SpecialAttack
Anim.Enemy.Combat.HitReaction
Anim.Enemy.Combat.Death
```

Do not use player animation tags such as `Anim.Combat.MeleeHit` for enemies.

`UPlatformerEnemyAnimInstance` caches:

- owning `APlatformerEnemyBase`;
- ASC;
- movement component;
- enemy anim data from the enemy actor.

It exposes movement, AI, combat and derived helper variables to AnimBP:

```text
GroundSpeed
AirSpeed
VerticalVelocity
MovementDirectionX
FlightDirectionZ
bIsGrounded
bIsInAir
bIsFlying
bHasCombatTarget
bIsChasing
bIsInAttackRange
CombatTargetDistance
AttackRange
bIsMeleeAttacking
bIsRangedAttacking
bIsSpecialAttacking
bIsHitReacting
bIsDeathAnimating
bShouldIdle
bShouldMove
bShouldJump
bShouldFall
bShouldAttack
bShouldHitReact
bShouldDie
bShouldFlyIdle
bShouldFlyMove
bShouldFlyAttack
```

Recommended AnimGraph shape:

```text
Locomotion State Machine
  -> Slot used by enemy montages
  -> Output Pose
```

Attack, hit reaction and death are montage-driven. State machine hit/death booleans exist as helpers, but the current agreed direction is montage-driven for hit reaction and death.

## Editor PlatformerSettings

`PlatformerSettings` is editor-only tooling in `CookieBrosLevelEditor`. It edits selected actors through transient settings objects.

Enemy settings classes:

```text
UPlatformerEnemySettingsObject
UPlatformerRangedEnemySettingsObject
UPlatformerEliteEnemySettingsObject
```

Enemy creation order in the settings widget matters:

```text
APlatformerEnemyElite
APlatformerEnemyRanged
APlatformerEnemyBase
```

`UPlatformerEnemySettingsObject` exposes common enemy values:

```text
Health
MovementSpeed
Damage
HitDelay
MovementDelayOnHit
OnHitTakenImpulse
EnablePlayerChase
ChaseAgroRadius
bRepeatPath
PathPoints
```

PlatformerSettings must remain a facade. It should call typed actor APIs and must not contain gameplay rules.

## Project Layer Notes

Current DragonSlayer project wrappers include:

```text
Source/DragonSlayer/AI/EnemyMelee.h
Source/DragonSlayer/AI/EnemyRanged.h
Source/DragonSlayer/AI/EnemyFlying.h
Source/DragonSlayer/AI/BossBase.h
Source/DragonSlayer/Data/EnemyArchetypeAsset.h
```

`UEnemyArchetypeAsset` extends `UPlatformerEnemyArchetypeAsset` with reward/progression fields:

```text
CurrencyDrop
OverdriveEnergyDrop
Immunities
```

Keep these in project layer. Do not move DragonSlayer reward logic into `CookieBrosPlatformer`.

## Common Extension Tasks

Add a reusable enemy stat:

1. Add property and typed getter/setter to `APlatformerEnemyBase`.
2. Apply/clamp it in `ApplyEnemyRuntimeSettings()` if it affects runtime state.
3. Add PlatformerSettings field only if designers need editor quick tuning.
4. Update this document and `Context_PlatformerSettings.md`.

Add a new reusable attack type:

1. Add enemy animation tag in `PlatformerEnemyAnimGameplayTags`.
2. Add payload override or subclass method in the relevant enemy class.
3. Keep `StartAttackAnimation()` / `ApplyAttackHit()` separation.
4. Add montage entry to `UPlatformerEnemyAnimDataAsset`.
5. Add `Platformer Enemy Attack Hit` notify to the montage.

Add a project-specific enemy:

1. Prefer inheriting from plugin enemy classes.
2. Put DragonSlayer-only behavior in `Source/DragonSlayer`.
3. Do not copy plugin base movement/combat logic.
4. Use project data assets only for project-specific data like rewards.

Add a new enemy AnimBP:

1. Parent class must be `PlatformerEnemyAnimInstance`.
2. Assign AnimBP to enemy mesh.
3. Assign `EnemyAnimDataAsset` on the enemy actor.
4. Use enemy tags only.
5. Put attack notify on impact frames.

## Architectural Pitfalls

- Do not put gameplay rules in widgets or editor settings objects.
- Do not make Chase a separate animation mechanic. Chase is movement; animation may treat it as a Move/FlyMove variant.
- Do not use player animation tags for enemy montages.
- Do not bypass GAS damage path for combat damage.
- Do not duplicate melee/ranged payload logic in Blueprint unless the goal is presentation only.
- Do not add ad-hoc managers for enemy state; use actor, component, GAS, StateTree, DataAsset or interface-friendly hooks.
- Do not move DragonSlayer rewards/progression into the plugin.
- Do not remove attack notify fallback unless all production montages are guaranteed to have notify timing.

## Verification Checklist

After enemy changes, verify:

- project builds as `DragonSlayerEditor`;
- placed melee enemy can patrol;
- enemy acquires player target through perception or damage;
- chase moves toward target only when enabled and in chase radius;
- attack montage starts from `EnemyAnimDataAsset`;
- `Platformer Enemy Attack Hit` notify applies damage at the authored frame;
- fallback hit still works when montage has no notify;
- hit reaction montage plays on non-fatal stagger damage;
- death montage plays and actor is destroyed after montage duration;
- `MovementDelayOnHit = 0` does not pause movement;
- `MovementDelayOnHit > 0` suppresses patrol/chase input briefly;
- `OnHitTakenImpulse = 0` disables knockback;
- `OnHitTakenImpulse > 0` knocks enemy horizontally away from instigator;
- PlatformerSettings pulls and pushes common enemy values correctly;
- DragonSlayer project wrappers still compile if plugin API changes.

## Related Docs

```text
Docs/EnemyAnimation_Setup.md
Docs/MeleeEnemyAnimation_Example_Setup.md
Docs/ProjectSystems/Context_PlatformerSettings.md
Docs/ProjectSystems/Context_DeveloperSettings.md
Docs/ProjectSystems/Context_PaperTileMap.md
```

