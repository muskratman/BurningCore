# Инструкция: настройка example-анимации Melee Enemy

Эта инструкция описывает ручную настройку базовой анимации для example-класса `APlatformerEnemyMelee` в плагине `CookieBrosPlatformer`.

## Цель

Настроить reusable example enemy так, чтобы:

- enemy использовал `PlatformerEnemyAnimInstance`;
- locomotion работал через `Idle` / `Move` / `Jump` / `Fall`;
- chase не был отдельным animation state, а оставался вариантом внутри `Move`;
- melee attack запускался через enemy montage;
- damage применялся через C++ notify `Platformer Enemy Attack Hit`.

## Используемые assets

```text
Plugins/CookieBrosPlatformer/Content/AI/BP_PlatformerEnemyMelee.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/ABP_PlatformerEnemy_Walking.uasset
Plugins/CookieBrosPlatformer/Content/AI/Anim/DA_PlatformerEnemyAnimExample_Melee.uasset
```

## Ownership

Эта настройка живёт в `Plugins/CookieBrosPlatformer`, потому что это reusable platformer foundation example.

Не добавляй сюда DragonSlayer-specific логику, UI-правила, progression или project-only combat rules. Если нужен конкретный DragonSlayer enemy с уникальным поведением, делай project-specific child в `Source/DragonSlayer` / project content.

## 1. Настроить Enemy Blueprint

Открой:

```text
BP_PlatformerEnemyMelee
```

Проверь `Class Settings`:

```text
Parent Class = PlatformerEnemyMelee
```

Выбери компонент `Mesh` и выставь:

```text
Animation Mode = Use Animation Blueprint
Anim Class = ABP_PlatformerEnemy_Walking
```

В defaults самого enemy найди секцию:

```text
Animation
```

И назначь:

```text
EnemyAnimDataAsset = DA_PlatformerEnemyAnimExample_Melee
```

Для быстрой проверки атаки можно использовать такие значения:

```text
Combat / Melee / MeleeTraceDistance = 140-200
Combat / Melee / MeleeTraceRadius = 45-80
Combat / Hit Reaction / MovementDelayOnHit = 0.2-0.5
Combat / Hit Reaction / OnHitTakenImpulse = 300-700
Animation / Attack / AttackHitFallbackDelay = 0.2
AI / Combat / Enable Player Chase = true
AI / Combat / Chase Agro Radius = 800-1200
```

`AttackHitFallbackDelay` — страховка на случай, если montage проигрывается, но notify ещё не поставлен. Для финальной настройки timing лучше задавать через notify.

`MovementDelayOnHit` отвечает за паузу AI movement input после non-fatal damage. Если значение `0`, enemy не останавливается при получении урона.

`OnHitTakenImpulse` отвечает за горизонтальный side-view knockback от атакующего. Если значение `0`, enemy не отбрасывается.

## 2. Настроить Enemy Anim Data Asset

Открой:

```text
DA_PlatformerEnemyAnimExample_Melee
```

В массиве:

```text
EnemyAnimations
```

добавь минимум одну запись:

```text
AnimTag = Anim.Enemy.Combat.MeleeAttack
Montage = твой melee attack AnimMontage
DefaultPlayRate = 1.0
```

Опционально можно добавить:

```text
Anim.Enemy.Combat.HitReaction -> hit reaction montage
Anim.Enemy.Combat.Death       -> death montage
```

Эти montage запускаются native C++ enemy flow:

```text
DamageAmount >= StaggerThreshold -> Anim.Enemy.Combat.HitReaction
Enemy death                      -> Anim.Enemy.Combat.Death
```

Fatal hit не проигрывает hit reaction отдельно. Если удар убил enemy, запускается death montage.

Важно: для enemy не используй player tags:

```text
Anim.Combat.MeleeHit
Anim.Combat.RangedShot
Anim.Combat.HitReaction
```

Для melee enemy attack нужен именно:

```text
Anim.Enemy.Combat.MeleeAttack
```

## 3. Настроить Melee Attack Montage

Открой montage, указанный в `DA_PlatformerEnemyAnimExample_Melee` для:

```text
Anim.Enemy.Combat.MeleeAttack
```

Проверь:

```text
Skeleton = тот же skeleton, что у Mesh enemy
Slot = тот slot, который используется в AnimBP
```

На кадр фактического удара добавь Anim Notify:

```text
Platformer Enemy Attack Hit
```

Этот notify вызывает C++:

```text
APlatformerEnemyBase::ApplyPendingAttackHit()
```

Damage, melee sweep и combat rules остаются в C++. Montage и notify только задают authored timing удара.

## 4. Настроить Animation Blueprint

Открой:

```text
ABP_PlatformerEnemy_Walking
```

В `Class Settings` проверь:

```text
Parent Class = PlatformerEnemyAnimInstance
```

`Event Graph` может быть минимальным. Основные переменные обновляет C++ в `UPlatformerEnemyAnimInstance`.

## 5. Настроить AnimGraph

Базовая схема:

```text
Locomotion State Machine
  -> Slot
  -> Output Pose
```

Минимальная locomotion state machine:

```text
Idle
Move
Jump
Fall
```

Переходы:

```text
Idle -> Move: bShouldMove
Move -> Idle: bShouldIdle
Idle/Move -> Jump: bShouldJump
Jump -> Fall: bShouldFall
Fall -> Idle/Move: bIsGrounded
Any -> Death: bShouldDie
Any Alive -> HitReact: bShouldHitReact
```

Для `Idle` используй idle sequence.

Для `Move` используй walk/run sequence или blendspace. Основной критерий перехода — `bShouldMove`.

## 6. Chase как вариант locomotion

Chase не является отдельной animation-механикой.

Enemy chase — это gameplay movement к combat target. В animation layer он должен быть только context flag для выбора визуального варианта движения.

Не делай отдельный state:

```text
Chase
```

Вместо этого внутри обычного `Move` state можно использовать:

```text
bIsChasing
```

Пример:

```text
bIsChasing = false -> обычная walk/run animation
bIsChasing = true  -> более агрессивная walk/run animation
```

Если отдельной chase-анимации пока нет, просто игнорируй `bIsChasing` и проигрывай обычный `Move`.

## 7. Slot для attack montage

После locomotion state machine добавь Slot node:

```text
Locomotion State Machine -> Slot "DefaultSlot" -> Output Pose
```

Slot name в AnimBP должен совпадать со slot, который используется в melee attack montage.

Если montage стоит в другом slot, например:

```text
UpperBody
```

то AnimGraph должен использовать соответствующий slot:

```text
Locomotion State Machine -> Slot "UpperBody" -> Output Pose
```

## 8. Проверка в PIE

Поставь `BP_PlatformerEnemyMelee` на тестовый уровень рядом с player / Dragon.

Проверь по шагам:

1. Enemy стоит на месте: играет `Idle`.
2. Enemy движется или патрулирует: играет `Move`.
3. Enemy видит player: `bHasCombatTarget = true`.
4. Enemy движется к player: `bIsChasing = true`, но используется обычный `Move` branch.
5. Enemy входит в attack range: запускается montage `Anim.Enemy.Combat.MeleeAttack`.
6. На notify `Platformer Enemy Attack Hit` применяется melee hit.
7. Если notify не поставлен, hit должен сработать через `AttackHitFallbackDelay`.
8. Enemy получает non-fatal hit: если `MovementDelayOnHit > 0`, enemy временно не добавляет patrol/chase movement input.
9. Enemy получает non-fatal hit: если `OnHitTakenImpulse > 0`, enemy отбрасывается по X от instigator.

## Частые ошибки

- `ABP_PlatformerEnemy_Walking` наследуется не от `PlatformerEnemyAnimInstance`.
- На `Mesh` enemy не назначен `ABP_PlatformerEnemy_Walking`.
- В `BP_PlatformerEnemyMelee` не назначен `EnemyAnimDataAsset`.
- В `DA_PlatformerEnemyAnimExample_Melee` нет `Anim.Enemy.Combat.MeleeAttack`.
- В DataAsset указан animation sequence вместо montage.
- Montage использует slot, которого нет в AnimGraph.
- В attack montage нет notify `Platformer Enemy Attack Hit`.
- В AnimBP сделан отдельный `Chase` state вместо variant внутри `Move`.
- Damage logic добавлена в AnimBP. Damage должен оставаться в C++.
- `MovementDelayOnHit = 0`, поэтому enemy не останавливается при hit.
- `OnHitTakenImpulse = 0`, поэтому enemy не получает knockback.
- У enemy слишком большое трение/braking в movement component, поэтому слабый knockback почти не виден.

## Что считается правильным результатом

Правильная схема:

```text
AI movement / chase logic
  -> enemy moves toward target
  -> AnimInstance sets bShouldMove and bIsChasing
  -> AnimBP plays Move or Move variant
  -> attack range reached
  -> C++ starts Anim.Enemy.Combat.MeleeAttack montage
  -> Platformer Enemy Attack Hit notify
  -> C++ applies melee sweep / damage
```

Такой flow сохраняет чистые границы:

- movement decision остаётся в enemy C++ / AI layer;
- animation selection остаётся в AnimBP;
- combat damage остаётся в C++;
- reusable setup остаётся в `CookieBrosPlatformer`.
