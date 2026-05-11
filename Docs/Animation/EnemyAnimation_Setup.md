# Инструкция: настройка анимации Enemy

Эта инструкция описывает, как подключить базовую анимацию врагов через reusable-классы плагина `CookieBrosPlatformer`.

Документ написан для ручной настройки в Unreal Editor. Файлы в `Docs/ProjectSystems` используются как контекст для AI-агентов, поэтому эта инструкция лежит отдельно.

## Что уже есть в плагине

Для Enemy теперь есть отдельная animation foundation:

- `UPlatformerEnemyAnimInstance`
- `UPlatformerEnemyAnimDataAsset`
- enemy-specific animation tags
- поле `EnemyAnimDataAsset` в `APlatformerEnemyBase`

Enemy-анимация намеренно отделена от player-анимации. В платформерах враги часто проще, более scripted и сильнее завязаны на archetype/AI-поведение, поэтому отдельный anim contract чище, чем общий универсальный класс для игрока и врагов.

## Основные файлы

Класс AnimInstance:

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/Animation/PlatformerEnemyAnimInstance.h
```

Data Asset:

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/Data/PlatformerEnemyAnimDataAsset.h
```

Gameplay Tags:

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/Animation/PlatformerEnemyAnimGameplayTags.h
```

Enemy base:

```text
Plugins/CookieBrosPlatformer/Source/CookieBrosPlatformer/Public/AI/PlatformerEnemyBase.h
```

## Общая схема настройки

Нужно сделать три вещи:

1. Создать `EnemyAnimDataAsset`.
2. Создать Enemy Animation Blueprint с parent class `PlatformerEnemyAnimInstance`.
3. Назначить AnimBP и EnemyAnimDataAsset на Enemy Blueprint.

## Шаг 1. Создать EnemyAnimDataAsset

В Content Browser создай новый Data Asset типа:

```text
PlatformerEnemyAnimDataAsset
```

Например:

```text
DA_EnemyAnim_Goblin
DA_EnemyAnim_FlyingEye
DA_EnemyAnim_Skeleton
```

Внутри asset заполни массив:

```text
EnemyAnimations
```

Каждый элемент содержит:

- `AnimTag`
- `Montage`
- `DefaultPlayRate`

Пример:

```text
Anim.Enemy.Combat.MeleeAttack  -> AM_Goblin_Attack
Anim.Enemy.Combat.HitReaction  -> AM_Goblin_HitReact
Anim.Enemy.Combat.Death        -> AM_Goblin_Death
```

Для ranged enemy:

```text
Anim.Enemy.Combat.RangedAttack -> AM_Archer_Shot
```

Для flying enemy:

```text
Anim.Enemy.Movement.Fly        -> AM_FlyingEye_Fly
Anim.Enemy.Combat.MeleeAttack  -> AM_FlyingEye_Attack
```

## Шаг 2. Использовать enemy-specific tags

Для врагов используй только enemy tags:

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

Не используй для Enemy старые player tags:

```text
Anim.Combat.MeleeHit
Anim.Combat.RangedShot
Anim.Combat.HitReaction
```

Они относятся к playable-character animation contract.

## Шаг 3. Создать Enemy Animation Blueprint

Создай Animation Blueprint для скелета врага.

В настройках AnimBP установи parent class:

```text
PlatformerEnemyAnimInstance
```

После этого в AnimBP будут доступны переменные из C++.

## Шаг 4. Назначить AnimBP на Enemy Blueprint

Открой Blueprint врага, например:

```text
BP_PlatformerEnemyMelee
BP_PlatformerEnemyRanged
BP_PlatformerEnemyFlying
BP_PlatformerEnemyElite
```

На `Mesh` назначь:

```text
Animation Mode = Use Animation Blueprint
Anim Class = твой Enemy AnimBP
```

## Шаг 5. Назначить EnemyAnimDataAsset

В defaults врага найди секцию:

```text
Animation
```

И назначь:

```text
EnemyAnimDataAsset = твой DA_EnemyAnim_...
```

Этот asset нужен для поиска montage по gameplay tag.

## Переменные для ground enemy

Для обычных walking enemies используй:

```text
bShouldIdle
bShouldMove
bShouldJump
bShouldFall
GroundSpeed
VerticalVelocity
MovementDirectionX
```

Простая логика state machine:

```text
Idle -> Move: bShouldMove
Move -> Idle: bShouldIdle
Idle/Move -> Jump: bShouldJump
Jump -> Fall: bShouldFall
Any -> Death: bShouldDie
Any Alive -> HitReact: bShouldHitReact
```

`bShouldMove` становится true, когда enemy жив, стоит на земле, не летит, не проигрывает hit reaction/attack state и имеет скорость выше `MovementStateSpeedThreshold`.

## Переменные для flying enemy

Для flying enemies используй отдельные flight-переменные:

```text
bIsFlying
bIsFlyingMoving
bShouldFlyIdle
bShouldFlyMove
bShouldFlyAttack
AirSpeed
FlightDirectionZ
```

Для flying enemy лучше сделать отдельную ветку state machine:

```text
FlyIdle
FlyMove
FlyAttack
HitReact
Death
```

Простые переходы:

```text
FlyIdle -> FlyMove: bShouldFlyMove
FlyMove -> FlyIdle: bShouldFlyIdle
Any Fly -> FlyAttack: bShouldFlyAttack
Any -> Death: bShouldDie
Any Alive -> HitReact: bShouldHitReact
```

Не привязывай flying enemy к `bShouldIdle` / `bShouldMove`. Эти переменные предназначены для ground locomotion.

## Переменные для chase

Доступные переменные:

```text
bHasCombatTarget
bIsChasing
CombatTargetDistance
AttackRange
bIsInAttackRange
```

Сейчас chase определяется просто:

```text
bIsChasing = HasCombatTarget && GroundSpeed > MovementStateSpeedThreshold
```

Chase не является отдельной animation-механикой и не должен подменять locomotion state.
Это контекстный animation helper: если нужен визуально другой бег за игроком, делай variant внутри обычного `Move` / `FlyMove` state через `bIsChasing`, blend pose или blendspace parameter.

Для ground enemy базовая логика остаётся:

```text
Move: bShouldMove
Chase variant внутри Move: bShouldMove && bIsChasing
```

Для flying enemy:

```text
FlyMove: bShouldFlyMove
Chase variant внутри FlyMove: bShouldFlyMove && bIsChasing
```

Не заводи отдельный `Chase` / `FlyChase` state, если он не меняет gameplay posture, collision, movement mode или combat contract.

## Переменные для attack / hit / death

Доступные переменные:

```text
bIsMeleeAttacking
bIsRangedAttacking
bIsSpecialAttacking
bIsHitReacting
bIsDeathAnimating
bIsPlayingEnemyMontage
bShouldAttack
bShouldHitReact
bShouldDie
```

Важно: enemy combat-анимации сейчас montage-driven.

Это значит, что `bIsMeleeAttacking`, `bIsRangedAttacking`, `bIsHitReacting` или `bIsDeathAnimating` станут true только если был запущен соответствующий enemy montage через `PlayEnemyMontage` или native C++ enemy flow.

## Как запускать montage

В `PlatformerEnemyAnimInstance` есть API:

```text
PlayEnemyMontage(AnimTag, PlayRate)
StopEnemyMontage(AnimTag, BlendOutTime)
ResolveEnemyMontage(AnimTag)
```

Пример:

```text
PlayEnemyMontage(Anim.Enemy.Combat.MeleeAttack)
PlayEnemyMontage(Anim.Enemy.Combat.RangedAttack)
PlayEnemyMontage(Anim.Enemy.Combat.HitReaction)
PlayEnemyMontage(Anim.Enemy.Combat.Death)
```

Запускать montage можно из:

- Enemy Blueprint
- StateTree Task
- GAS ability
- native C++ enemy combat flow

## Native enemy hit / death flow

`APlatformerEnemyBase` автоматически запускает enemy montage из `EnemyAnimDataAsset`:

```text
OnCombatDamageReceived() + DamageAmount >= StaggerThreshold
  -> Anim.Enemy.Combat.HitReaction

OnCombatDeath()
  -> Anim.Enemy.Combat.Death
```

Death montage задерживает native `Destroy()` на длительность проигрывания montage. Если death montage не найден или не может быть проигран, enemy уничтожается сразу как fallback.

Hit reaction не проигрывается для fatal hit: если damage уже убил enemy, запускается только death flow.

## Native enemy hit movement reaction

`APlatformerEnemyBase` поддерживает две reusable настройки реакции на non-fatal hit:

```text
MovementDelayOnHit
OnHitTakenImpulse
```

`MovementDelayOnHit` задаёт паузу AI movement input после получения урона. Если значение `0`, enemy не останавливается и продолжает обычный patrol/chase flow.

`OnHitTakenImpulse` задаёт горизонтальный side-view knockback от damage instigator. Если значение `0`, enemy не получает knockback.

Эти настройки не должны жить в AnimBP. AnimBP только отображает результат через locomotion / hit montage, а movement pause и impulse остаются в C++ enemy foundation.

## Native enemy attack flow

Native enemy attack разделён на две стадии:

```text
TryAttackTarget()
StartAttackAnimation()
ApplyPendingAttackHit()
ApplyAttackHit()
```

`TryAttackTarget()` больше не наносит damage напрямую. Он валидирует target, запускает attack montage через `StartAttackAnimation()` и фиксирует pending target для текущей атаки.

Конкретный hit payload живёт в `ApplyAttackHit()`:

```text
APlatformerEnemyMelee  -> melee sweep / damage
APlatformerEnemyRanged -> projectile spawn
APlatformerEnemyElite  -> melee или ranged payload по attack tag, выбранному при старте
```

Attack montage выбирается через enemy animation tag:

```text
APlatformerEnemyMelee  -> Anim.Enemy.Combat.MeleeAttack
APlatformerEnemyRanged -> Anim.Enemy.Combat.RangedAttack
APlatformerEnemyElite  -> Anim.Enemy.Combat.MeleeAttack или Anim.Enemy.Combat.RangedAttack
```

## Attack hit notify

Для production timing поставь notify на кадр удара в attack montage:

```text
Platformer Enemy Attack Hit
```

Этот notify вызывает C++:

```text
APlatformerEnemyBase::ApplyPendingAttackHit()
```

Damage, projectile spawning и combat rules остаются в C++; AnimBP и notify только задают authored timing.

Если montage не назначен или не может быть проигран, native flow применит hit сразу как fallback. Если montage проигрывается, но notify пока не поставлен, сработает fallback timer `AttackHitFallbackDelay` из enemy defaults. Для финальной настройки montage лучше поставить notify и при необходимости уменьшить/обнулить fallback delay.

## Рекомендуемый минимальный AnimGraph

Для melee enemy:

```text
Locomotion State Machine
  Idle
  Move
  Jump
  Fall

Slot для attack/hit/death montage
Output Pose
```

Для flying enemy:

```text
Flight State Machine
  FlyIdle
  FlyMove
  FlyAttack

Slot для hit/death montage
Output Pose
```

Если attack сделан montage, лучше держать attack в montage slot.

Если attack должен быть полноценным loop/state, можно использовать `bShouldAttack` или `bShouldFlyAttack` в state machine.

## Частые ошибки

- Enemy AnimBP унаследован от `PlatformerAnimInstance`, а не от `PlatformerEnemyAnimInstance`.
- На враге не назначен `EnemyAnimDataAsset`.
- В `EnemyAnimDataAsset` используются player tags.
- Montage находится в неправильном slot.
- В attack montage нет notify `Platformer Enemy Attack Hit`, поэтому hit timing уходит в fallback delay.
- Flying enemy использует ground locomotion переменные.
- Damage logic добавлена в AnimBP.

## Что проверять после настройки

1. Enemy стоит на месте: работает idle.
2. Enemy патрулирует: работает move или fly move.
3. Enemy видит target и движется к нему: работает chase/fly chase.
4. Enemy атакует: запускается нужный attack montage.
5. Enemy получает hit reaction: запускается hit montage.
6. Enemy умирает: death state или death montage срабатывает.
7. Flying enemy не проваливается в ground locomotion branch.

## Когда расширять C++

Расширять `PlatformerEnemyAnimInstance` стоит, если новое состояние универсально для всех enemy или большинства enemy archetypes.

Примеры reusable-состояний:

```text
bIsStunned
bIsSpawning
bIsReturningToPatrol
bIsPreparingAttack
bIsRecoveringFromAttack
```

Если состояние относится только к конкретному врагу, лучше держать его в project-specific AnimBP или отдельном project-specific subclass.
