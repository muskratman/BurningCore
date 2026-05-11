# Инструкция: настройка Shot animation для playable platformer character

Документ описывает, как настроить базовую анимацию `Shot` в reusable foundation плагина `CookieBrosPlatformer`, чтобы она корректно проигрывалась и когда character стоит на месте, и когда character бежит.

Текущий production scope этого документа — plugin example flow, не DragonSlayer-specific content.

## Цель

Настроить `UGA_PlatformerBaseShot` так, чтобы:

- ability оставалась generic и data-driven;
- конкретная анимация выбиралась через `UPlatformerAnimDataAsset`;
- locomotion продолжал работать во время shot;
- shot montage перекрывал только верх тела, а ноги продолжали брать pose из idle/run locomotion;
- текущий slot contract оставался через `DefaultSlot`.

## Ownership

Эта настройка живет в plugin layer:

```text
Plugins/CookieBrosPlatformer/Content/Animation/ABP_PlatformerBase.uasset
Plugins/CookieBrosPlatformer/Content/Animation/ABP_PlatformerLayersExample.uasset
Plugins/CookieBrosPlatformer/Content/Animation/DA_PlatformerAnimData_Example.uasset
```

Причина: `Shot` в `UGA_PlatformerBaseShot` — generic projectile shot flow для platformer foundation. Project-specific персонажи должны переиспользовать этот contract через свой AnimDataAsset или child assets, а не добавлять hard reference на montage в ability.

## Runtime flow

Текущий C++ flow уже вызывает animation hook:

```text
UGA_PlatformerBaseShot::ActivateAbility
  -> SpawnConfiguredCombatProjectile(...)
  -> PlayAbilityAnimation(Anim.Combat.RangedShot)
```

`PlayAbilityAnimation` ищет montage так:

```text
Character AnimDataAsset
  -> AbilityAnimations[Anim.Combat.RangedShot]
  -> AnimMontage
  -> Character->PlayAnimMontage(...)
```

Поэтому для базовой привязки не нужно менять `UGA_PlatformerBaseShot.cpp`. Нужно правильно настроить animation assets.

## Главный принцип: locomotion + upper-body montage

Чтобы shot работал и на месте, и в беге, montage не должен полностью заменять всю позу персонажа.

Правильная схема:

```text
Idle/Run locomotion
  -> base full-body pose
  -> DefaultSlot montage pose
  -> Layered Blend Per Bone
  -> upper body берет shot montage
  -> legs остаются из locomotion
```

Результат:

```text
Стоит + Shot = idle legs + shot upper body
Бежит + Shot = run legs + shot upper body
```

Неправильная схема для run+shot:

```text
Locomotion / Traversal / Combat layers
  -> Layered Blend Per Bone
  -> DefaultSlot
  -> ControlRig
  -> Output Pose
```

Такой вариант делает montage full-body overlay, потому что `DefaultSlot` стоит после `Layered Blend Per Bone` и получает уже готовую итоговую позу. Сам `Slot` node не знает, что shot должен влиять только на верх тела. Во время бега character может визуально скользить, замирать ногами или терять run pose.

## Что должно быть в ABP_PlatformerBase

`ABP_PlatformerBase` должен быть host/composition AnimBP для reusable playable character animation.

Его ответственность:

- общий composition graph;
- подключение anim layers через `ALI_PlatformerLayers`;
- `DefaultSlot` для ability montage playback;
- layered blend, который решает, какие части тела перекрывает montage;
- отсутствие project-specific animation rules.

### Required setup

В `ABP_PlatformerBase` проверь:

```text
Class Settings
  Parent Class = PlatformerAnimInstance
```

Это важно, потому что C++ ability ищет anim instance через:

```text
Cast<UPlatformerAnimInstance>(Mesh->GetAnimInstance())
```

Если host AnimBP не наследуется от `PlatformerAnimInstance`, `AnimDataAsset` может не резолвиться для ability montage.

### AnimGraph structure

Текущая структура `ABP_PlatformerBase`:

```text
LocomotionLayer
TraversalOverlayLayer
CombatActionLayer
  -> Layered Blend Per Bone
  -> DefaultSlot
  -> ControlRig (isFalling)
  -> Output Pose
```

Для обычного full-body montage это допустимо. Для `Shot`, который должен работать во время бега, `DefaultSlot` нужно перенести до upper-body `Layered Blend Per Bone`, а не ставить `DefaultSlot` после финального layer blend.

Рекомендуемая структура host graph для shot overlay:

```text
Linked Anim Layer: LocomotionLayer
  -> LocomotionPose

Linked Anim Layer: TraversalOverlayLayer
  -> TraversalOverlayPose

Movement/Traversal Layered Blend Per Bone
  Base Pose = LocomotionPose
  Blend Pose 0 = TraversalOverlayPose
  Blend Weight 0 = traversal overlay weight/rule
  Branch Filter = traversal-specific bone chain
  -> BaseMovementPose

Slot: DefaultSlot
  Source Pose = BaseMovementPose
  -> CombatSlotPose

Upper-body Layered Blend Per Bone
  Base Pose = BaseMovementPose
  Blend Pose 0 = CombatSlotPose
  Blend Weight 0 = 1.0
  Branch Filter = upper body bone chain
  -> UpperBodyCombatPose

ControlRig (optional, e.g. isFalling)
  Source Pose = UpperBodyCombatPose
  -> Output Pose
```

Важно: в текущем `ALI_PlatformerLayers` `TraversalOverlayLayer` не имеет `Input Pose`, поэтому не нужно пытаться строить chain `Locomotion -> Traversal` внутри layer implementation. `ABP_PlatformerBase` должен получить `LocomotionPose` и `TraversalOverlayPose` как отдельные linked layer outputs и сам собрать из них `BaseMovementPose`.

Если traversal overlay сейчас не нужен или временно отключен, допустима упрощенная схема:

```text
LocomotionLayer
  -> LocomotionPose

DefaultSlot
  Source Pose = LocomotionPose
  -> CombatSlotPose

Layered Blend Per Bone
  Base Pose = LocomotionPose
  Blend Pose 0 = CombatSlotPose
  Branch Filter = upper body
  -> UpperBodyCombatPose

ControlRig (optional)
  Source Pose = UpperBodyCombatPose
  -> Output Pose
```

Ключевое правило: `Output Pose` должен идти из результата `Layered Blend Per Bone`, а не напрямую из `DefaultSlot`.

### DefaultSlot

На данный момент в плагине используется:

```text
DefaultSlot
```

Поэтому shot montage тоже должен использовать `DefaultSlot`.

Не добавляй новый `UpperBodySlot` только ради shot, пока нет реальной необходимости. Новый slot contract потребует синхронизации AnimBP, montages, docs и, возможно, будущих project assets.

### Layered Blend Per Bone

Для mannequin skeleton стартовая настройка:

```text
Blend Mode = Branch Filter
Branch Bone = spine_01
Blend Depth = 0
Mesh Space Rotation Blend = true
Curve Blend Option = Override
```

Не используй `Blend Depth = -1` для этого branch filter. В текущем UE 5.6 graph
это исключает ветку из blend: montage стартует и логирует duration, но верх тела
не получает pose из `DefaultSlot`.

Если shot слишком сильно крутит таз или ломает бег:

```text
Branch Bone = spine_02
```

Если руки почти не получают shot pose:

```text
Branch Bone = spine_01
Blend Depth = 0
```

Точная кость зависит от skeleton и authored animation. Для UE mannequin обычно начинают со `spine_01` или `spine_02`.

### Что не должно быть в ABP_PlatformerBase

Не добавляй сюда:

- конкретную Dragon shot animation;
- hard reference на project-specific montage;
- gameplay decision вроде "можно ли стрелять";
- spawn projectile logic;
- state machine state `Shot`, если shot уже идет через montage.

`ABP_PlatformerBase` должен только дать reusable место, куда montage может быть наложен поверх locomotion.

## Что должно быть в ABP_PlatformerLayersExample

`ABP_PlatformerLayersExample` должен быть example implementation для `ALI_PlatformerLayers`.

Его ответственность:

- locomotion state machine;
- example movement assets;
- traversal presentation example;
- lightweight implementation для combat layer, если он реально нужен;
- отсутствие ability-specific playback logic.

### Required setup

В `ABP_PlatformerLayersExample` проверь:

```text
Class Settings
  Parent Class = PlatformerAnimInstance

Implemented Interfaces
  ALI_PlatformerLayers
```

`ABP_PlatformerLayersExample` должен читать C++ variables из `UPlatformerAnimInstance`:

```text
bShouldIdle
bShouldMove
bShouldJump
bShouldFall
bShouldDashStart
bShouldDashLoop
bShouldDashEnd
bShouldLadderStart
bShouldLadderLoop
bShouldLadderEnd
bShouldLedgeGrabStart
bShouldLedgeGrabLoop
bShouldLedgeGrabEnd
bIsDead
GroundSpeed
VerticalVelocity
MovementDirectionX
```

### LocomotionLayer

`LocomotionLayer` отвечает за full-body базовую позу:

```text
Idle
Run / Walk
Crouch Idle
Crouch Move
Jump
Fall
Land
Ladder
Ledge
Dash
Death
```

Shot state сюда добавлять не нужно.

Во время shot character все равно должен оставаться в текущем locomotion state:

```text
Idle -> остается Idle
Run  -> остается Run
```

Shot поверх этого добавит `ABP_PlatformerBase` через `DefaultSlot`, если `DefaultSlot` участвует в upper-body ветке до финального `Layered Blend Per Bone`.

### CombatActionsLayer

Если `ABP_PlatformerBase` содержит `DefaultSlot` в upper-body ветке, то `CombatActionsLayer` в example layer не должен вручную проигрывать shot animation.

Так как текущий `CombatActionsLayer` не имеет `Input Pose`, он не может быть настоящим passthrough layer в стиле `Linked Input Pose -> Output Pose`.

Минимально правильный вариант для shot setup:

```text
ABP_PlatformerBase
  не использует CombatActionsLayer для Shot

ABP_PlatformerLayersExample / CombatActionsLayer
  остается пустым, neutral/reference pose, или используется только для других authored overlays
```

Иными словами, combat montage playback остается в host graph через `DefaultSlot`. `CombatActionsLayer` не нужен для ranged shot, пока layer interface не передает input pose или пока не появится отдельная authored overlay задача.

Добавлять отдельный state `RangedShot` в `CombatActionsLayer` стоит только если ты сознательно уходишь от montage-based ability animation. Для текущей архитектуры это не нужно.

### Что не должно быть в ABP_PlatformerLayersExample

Не добавляй сюда:

- `Play Montage` calls;
- projectile spawn;
- ability activation rules;
- hard reference на `GA_PlatformerBaseShot`;
- `Anim.Combat.RangedShot` lookup.

Layer example должен оставаться presentation layer, а не gameplay controller.

## Что должно быть в DA_PlatformerAnimData_Example

`DA_PlatformerAnimData_Example` — data-driven mapping:

```text
GameplayTag -> AnimMontage
```

Его ответственность:

- хранить example montage для ability animation tags;
- задавать default play rate;
- не содержать graph logic или gameplay rules.

### Required entry для Shot

Открой:

```text
Plugins/CookieBrosPlatformer/Content/Animation/DA_PlatformerAnimData_Example.uasset
```

В массиве:

```text
AbilityAnimations
```

должна быть запись:

```text
AnimTag = Anim.Combat.RangedShot
Montage = AM_Platformer_RangedShot
DefaultPlayRate = 1.0
```

Если отдельного shot montage пока нет, можно временно использовать placeholder montage для smoke-test. Но финальная plugin example настройка должна иметь отдельный readable asset, например:

```text
Plugins/CookieBrosPlatformer/Content/Animation/Combat/AM_Platformer_RangedShot.uasset
```

Не оставляй melee montage как финальную привязку для `Anim.Combat.RangedShot`: это запутает ownership и сломает читаемость example foundation.

### Дополнительные useful entries

Для playable-character contract можно держать рядом:

```text
Anim.Combat.MeleeHit          -> AM_Platformer_MeleeHit
Anim.Combat.MeleeChargeLoop   -> AM_Platformer_MeleeChargeLoop
Anim.Combat.RangedShot        -> AM_Platformer_RangedShot
Anim.Combat.RangedChargeLoop  -> AM_Platformer_RangedChargeLoop
Anim.Combat.HitReaction       -> AM_Platformer_HitReaction
Anim.Combat.Death             -> AM_Platformer_Death
```

Добавляй только те записи, для которых реально есть montage и gameplay flow.

## Настройка AM_Platformer_RangedShot

Создай montage в plugin content:

```text
Plugins/CookieBrosPlatformer/Content/Animation/Combat/AM_Platformer_RangedShot.uasset
```

Проверь:

```text
Skeleton = тот же skeleton, что у SK_PlatformerMannequin / example mesh
Slot = DefaultSlot
Enable Root Motion = false
Blend In Time = 0.05 - 0.12
Blend Out Time = 0.08 - 0.15
Play Rate = 1.0
```

Для текущего instant-shot flow Anim Notify не обязателен: projectile уже спавнится в `UGA_PlatformerBaseShot` сразу при activation.

Если позже понадобится точный timing выстрела по кадру анимации, это отдельное foundation изменение:

```text
Play montage
Wait for AnimNotify / GameplayEvent
Spawn projectile on event
End ability
```

Не смешивай это с текущей простой задачей настройки overlay shot.

## Character Blueprint wiring для проверки

На example playable character blueprint, который проверяет plugin animation, должны быть назначены:

```text
Mesh / Animation Mode = Use Animation Blueprint
Mesh / Anim Class = ABP_PlatformerBase

Animation / LinkedAnimLayerClass = ABP_PlatformerLayersExample
Animation / AnimDataAsset = DA_PlatformerAnimData_Example
```

Почему так:

- `ABP_PlatformerBase` владеет host composition graph и `DefaultSlot`;
- `ABP_PlatformerLayersExample` дает locomotion implementation;
- `DA_PlatformerAnimData_Example` резолвит `Anim.Combat.RangedShot` в montage.

Если назначить на Mesh только `ABP_PlatformerLayersExample`, host graph из `ABP_PlatformerBase` может не участвовать, и montage overlay через `DefaultSlot` не будет работать как задумано.

## Проверка в Editor

### 1. Проверить data lookup

В `DA_PlatformerAnimData_Example`:

```text
AbilityAnimations contains Anim.Combat.RangedShot
Montage is not None
```

### 2. Проверить montage slot

В `AM_Platformer_RangedShot`:

```text
Slot = DefaultSlot
```

Slot name должен совпадать с `Slot` node в `ABP_PlatformerBase`.

### 3. Проверить host graph

В `ABP_PlatformerBase`:

```text
DefaultSlot exists
DefaultSlot is before upper-body layered blend
Layered blend base pose is locomotion/traversal
Output pose comes from layered blend, not directly from full-body slot
ControlRig, if present, runs after the layered blend
```

### 4. Проверить locomotion layer

В `ABP_PlatformerLayersExample`:

```text
Idle/Run transitions depend on movement variables
Shot does not create a locomotion transition
CombatActionsLayer does not manually play the shot
```

### 5. PIE test cases

Проверить:

```text
Stand still -> press Shot
Expected: legs stay idle, upper body plays shot

Run -> press Shot
Expected: legs keep running, upper body plays shot

Spam Shot
Expected: RangeAttackDelay still controls fire rate

Dash / ledge hang / ledge climb -> press Shot
Expected: activation is blocked by ability tags
```

## Common issues

### Montage does not play

Проверь:

```text
Character AnimDataAsset assigned
DA contains Anim.Combat.RangedShot
Montage assigned
Mesh Anim Class inherits PlatformerAnimInstance
Montage skeleton matches mesh skeleton
```

### Montage plays, but run stops

Вероятно, `DefaultSlot` используется как full-body output. Это как раз происходит, если graph выглядит так:

```text
Layered Blend Per Bone
  -> DefaultSlot
  -> Output Pose
```

Исправление:

```text
DefaultSlot Source Pose = locomotion/traversal pose
Layered Blend Per Bone:
  Base Pose = locomotion/traversal pose
  Blend Pose = DefaultSlot output
  Branch Filter = upper body
Output Pose = layered blend result, optionally after ControlRig
```

### Arms do not animate

Проверь branch filter:

```text
Branch Bone too high/low
Blend Depth is not `0` for the current `ABP_PlatformerBase` setup
Blend Depth is `-1`, which excludes the branch from the montage blend
Montage authored with little/no upper-body motion
Skeleton mismatch
```

### Legs receive shot pose

Проверь:

```text
Branch Bone should start at spine_01/spine_02, not pelvis/root
Root motion disabled
Montage animation does not key pelvis/root aggressively
```

## Architectural tradeoff

Текущий подход оставляет `Shot` instant:

```text
Projectile spawns immediately
Animation is visual feedback
```

Это простой и правильный путь для базового plugin foundation.

Более точный authored timing через notify/event будет лучше для polished combat, но потребует отдельного изменения в `UGA_PlatformerBaseShot` и GAS ability lifetime. Делать это стоит только когда реально нужен frame-accurate projectile release.
