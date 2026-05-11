# Инструкция: настройка ABP_PlatformerLayersExample (Locomotion Layers)

Документ описывает создание `ABP_PlatformerLayersExample` — linked layer AnimBP персонажа Dragon,
который содержит полный locomotion state machine.

Предварительно прочти [PlatformerBaseAnimBlueprint_Setup.md](PlatformerBaseAnimBlueprint_Setup.md) —
там описана роль `ABP_PlatformerBase` (host graph) и почему `ABP_PlatformerLayersExample` является отдельным файлом.

---

## Архитектура в двух словах

```
ABP_PlatformerBase  (host, на Mesh)
    вызывает LocomotionLayer
         ↓
ABP_PlatformerLayersExample  (linked layer, подключается через LinkedAnimLayerClass)
    содержит state machine со всеми состояниями и переходами
    читает sequences из DA_Dragon_LocomotionAnimSet
    читает переменные из UPlatformerAnimInstance (C++)
```

`ABP_PlatformerLayersExample` **не назначается на Mesh напрямую**. Он подключается через `LinkedAnimLayerClass`
на персонаже и вызывается из `ABP_PlatformerBase` через Linked Anim Graph ноду.

---

## Предварительные требования

### Скомпилировать проект

C++ класс `UPlatformerLocomotionAnimSet` добавлен в плагин. Чтобы он появился в UE Editor,
проект должен быть скомпилирован. В UE Editor нажми:

```
Tools → Compile
```

или используй Live Coding: `Ctrl + Alt + F11`.

После успешной компиляции при создании DataAsset в поиске появится тип
`PlatformerLocomotionAnimSet`.

---

## Часть 1: Создать DA_Dragon_LocomotionAnimSet

`DA_Dragon_LocomotionAnimSet` — DataAsset, который хранит все animation sequences и
blendspaces для скелета Dragon. AnimBP читает его один раз при инициализации.

### Шаг 1.1: Создать DataAsset

В Content Browser перейди в:

```
Content/Characters/Dragon/Animation
```

Кликни правой кнопкой → **Miscellaneous → Data Asset**.

В диалоге выбора класса введи в поиске:

```
PlatformerLocomotionAnimSet
```

Выбери `PlatformerLocomotionAnimSet`, нажми **Select**.

Назови файл:

```
DA_Dragon_LocomotionAnimSet
```

### Шаг 1.2: Заполнить поля

Открой `DA_Dragon_LocomotionAnimSet`. Поля сгруппированы по категориям.

#### Ground

| Поле | Что назначить | Если null |
|---|---|---|
| `Locomotion Blend Space` | BlendSpace 1D: X = GroundSpeed (0–600 cm/s), позы Idle/Walk/Run | T-pose на земле |
| `Crouch Blend Space` | BlendSpace 1D: X = GroundSpeed, позы CrouchIdle/CrouchWalk | статичная crouch поза |

#### Air

| Поле | Что назначить | Если null |
|---|---|---|
| `Jump Start Sequence` | поза во время прыжка вверх | T-pose в воздухе |
| `Fall Loop Sequence` | looped поза падения | T-pose при падении |
| `Land Sequence` | короткая landing поза (0.15–0.2s, no loop) | нет Land state, прямо в Locomotion |

> `Land Sequence` — feature toggle. Если null, переход AirFall → Locomotion происходит
> сразу без промежуточного состояния.

#### Traversal | Dash

Если Dash анимация не нужна — оставь все три null.

| Поле | Что назначить |
|---|---|
| `Dash Start Sequence` | начало dash (короткая, 0.1–0.15s) |
| `Dash Loop Sequence` | looped основная фаза dash |
| `Dash End Sequence` | выход из dash |

#### Traversal | Wall

| Поле | Что назначить |
|---|---|
| `Wall Slide Sequence` | looped скольжение по стене |
| `Wall Jump Sequence` | прыжок от стены |

#### Traversal | Ladder

| Поле | Что назначить |
|---|---|
| `Ladder Start Sequence` | вход на лестницу |
| `Ladder Loop Blend Space` | BlendSpace 1D: X = LadderSpeed (0..MaxClimbSpeed), позы GrabIdle/Climb |
| `Ladder End Sequence` | выход с лестницы |

> `Ladder Loop Blend Space` — BlendSpace по **фактической скорости** (`LadderSpeed`),
> а не по raw input. Аналогия с Idle↔Run: там ось — `GroundSpeed`, не input axis.
> При `LadderSpeed = 0` (персонаж держится без движения) — статичная поза.
> При `LadderSpeed > 0` — анимация подъёма. Это корректно даже если игрок держит
> input, но персонаж достиг края лестницы и остановился.

#### Traversal | LedgeGrab

| Поле | Что назначить |
|---|---|
| `Ledge Grab Start Sequence` | захват уступа |
| `Ledge Grab Loop Sequence` | looped вис |
| `Ledge Climb Sequence` | подъём через уступ |

#### Look

| Поле | Что назначить | Если null |
|---|---|---|
| `Look Aim Offset` | AimOffset BlendSpace: Yaw (-90..90), Pitch (-90..90) | нет Look системы |

---

## Часть 2: Назначить на персонажа

Открой Blueprint персонажа (`BP_PlayableDragonCharacter` или аналог).

```
Mesh компонент:
  Anim Class = ABP_PlatformerBase

Character Defaults:
  Linked Anim Layer Class  = ABP_PlatformerLayersExample                  ← создадим в Части 3
  Anim Data Asset          = DA_Dragon_AnimData
  Locomotion Anim Set      = DA_Dragon_LocomotionAnimSet
```

---

## Часть 3: Создать ABP_PlatformerLayersExample

### Шаг 3.1: Создать Animation Blueprint

В Content Browser в папке `Content/Characters/Dragon/Animation`:

```
Right Click → Animation → Animation Blueprint
```

- **Target Skeleton** = скелет Dragon (`SK_Dragon` или аналог)
- **Parent Class** = `PlatformerAnimInstance`

Назови: `ABP_PlatformerLayersExample`.

### Шаг 3.2: Class Settings

Открой `ABP_PlatformerLayersExample` → нажми **Class Settings**.

Проверь:
```
Parent Class = PlatformerAnimInstance
```

В **Interfaces → Add** добавь:
```
ALI_PlatformerLayers
```

После добавления в **My Blueprint → Graphs** появится вкладка `LocomotionLayer`.

Нажми **Compile**.

---

## Часть 4: Переменные AnimBP

Все cached sequences и feature toggles объявлены в C++ классе `UPlatformerAnimInstance`
и заполняются автоматически при инициализации из `LocomotionAnimSet`.

**В `ABP_PlatformerLayersExample` не нужно создавать никаких переменных вручную и не нужен Event Graph.**

Просто используй переменные напрямую в state machine — они унаследованы от
`PlatformerAnimInstance`:

**Sequences (Anim Sequence / Blend Space):**
```
CachedLocomotionBS          CachedCrouchBS
CachedJumpStartSequence     CachedFallLoopSequence    CachedLandSequence
CachedDashStartSequence     CachedDashLoopSequence    CachedDashEndSequence
CachedWallSlideSequence     CachedWallJumpSequence
CachedLadderStartSequence   CachedLadderLoopBS        CachedLadderEndSequence
CachedLedgeGrabStartSequence  CachedLedgeGrabLoopSequence  CachedLedgeClimbSequence
CachedLookAimOffset
```

**Feature toggles (Boolean):**
```
bHasDash    bHasWall    bHasLadder    bHasLedgeGrab    bHasLand    bHasLook
```

Если `DA_Dragon_LocomotionAnimSet` назначен на персонажа и проект скомпилирован —
все эти переменные будут заполнены до первого кадра анимации.

---

## Часть 5: LocomotionLayer AnimGraph

Перейди во вкладку **LocomotionLayer** (My Blueprint → Graphs).

### Шаг 5.1: Добавить State Machine

```
Right Click → Add New State Machine
```

Назови: `LocomotionSM`.

### Шаг 5.2: Подключить к Output Pose

**Без AimOffset:**
```
[LocomotionSM] ──→ [Output Pose]
```

**С AimOffset (условное применение):**

Добавь **Blend Poses by bool**:
```
Active Value = bHasLook

True Pose  = [LocomotionSM] → [Aim Offset Player]
                                   Aim Offset = CachedLookAimOffset
                                   Yaw   = LookYaw
                                   Pitch = LookPitch

False Pose = [LocomotionSM] (напрямую)

[Blend Poses by bool] ──→ [Output Pose]
```

`LookYaw` и `LookPitch` — переменные от `PlatformerAnimInstance`, уже вычислены в C++.

---

## Как использовать Cached-переменные в нодах State Machine

> Переменная `CachedLocomotionBS` — это asset reference, а не Pose. Напрямую
> к Output Pose подключить нельзя. Pose выдаёт только нода-плеер (`Blend Space Player`
> или `Sequence Player`). Чтобы такая нода принимала ПЕРЕМЕННУЮ вместо фиксированного
> asset — нужно expose pin.

### Как expose pin для Blend Space Player

1. Добавь ноду `Blend Space Player` в граф состояния (Right Click → Blend Space Player)
2. Кликни на ноду — в **Details** слева появятся её свойства
3. Найди свойство **Blend Space**
4. Справа от поля есть маленькая иконка (`⊙` или стрелка) — кликни её → выбери **Expose as Pin**
5. На ноде появится входной пин типа BlendSpace
6. Добавь **Get CachedLocomotionBS** (переменная от родительского класса) и подключи к этому пину

### Как expose pin для Sequence Player

То же самое: в Details → **Sequence** → Expose as Pin → подключи соответствующую
`Cached*` переменную.

---

## Часть 6: State Machine — все состояния

Открой `LocomotionSM` двойным кликом.

### Шаг 6.1: Создать состояния

Кликни правой кнопкой → **Add State**. Создай:

```
Locomotion     ← Idle + Run + Crouch — всё внутри одного состояния
AirJump
AirFall
Land
DashStart
DashLoop
DashEnd
LadderStart
LadderLoop
LadderEnd
LedgeGrabStart
LedgeGrabLoop
LedgeClimb
WallSlide
WallJump
Death
```

`CrouchIdle` и `CrouchMove` — **отдельных состояний нет**. Crouch обрабатывается
внутри `Locomotion` через `Blend Poses by bool`.

Потяни провод от **Entry** к `Locomotion` — это начальное состояние.

### Шаг 6.2: Настроить анимацию в каждом состоянии

Открой состояние двойным кликом. Добавь ноды анимации → подключи к **Output Pose**.

---

#### Locomotion

Внутри этого состояния нужно учесть два режима: обычное движение и приседание.
Используй **Blend Poses by bool** по `bIsCrouching`.

**Структура внутри состояния:**

```
[Get CachedLocomotionBS] → [Blend Space Player] (BS exposed as pin)
                                X = GroundSpeed
                                    ↓ (False Pose)
                          [Blend Poses by bool]
                             Active = bIsCrouching
                                    ↓
[Get CachedCrouchBS] →    [Blend Space Player] (BS exposed as pin)
                                X = GroundSpeed
                                    ↓ (True Pose)
                                    ↓
                              [Output Pose]
```

**Пошагово:**

1. Добавь **Blend Poses by bool** (Right Click → Blend Poses by bool)
2. В поле **Active Value** подключи переменную `bIsCrouching`
3. Добавь первый **Blend Space Player** для обычного движения:
   - Expose as Pin → **Blend Space** → подключи **Get CachedLocomotionBS**
   - **X** → подключи **Get GroundSpeed**
   - Выход → **False Pose** пин Blend Poses by bool
4. Добавь второй **Blend Space Player** для приседания:
   - Expose as Pin → **Blend Space** → подключи **Get CachedCrouchBS**
   - **X** → подключи **Get GroundSpeed**
   - Выход → **True Pose** пин Blend Poses by bool

> Если `CachedCrouchBS` null — True Pose будет T-pose. Чтобы избежать этого,
> оберни crouch ветку в **Is Valid** → при null используй Regular BlendSpace
> как fallback для обоих пинов.

5. Выход **Blend Poses by bool** → **Output Pose**

---

#### AirJump

1. Добавь **Sequence Player** (Right Click → Sequence Player)
2. В Details → **Sequence** → Expose as Pin
3. Подключи **Get CachedJumpStartSequence** к пину
4. **Loop Animation = false**
5. Выход → **Output Pose**

---

#### AirFall

Аналогично AirJump:
- Sequence → **Get CachedFallLoopSequence**
- **Loop Animation = true**

---

#### Land

- Sequence → **Get CachedLandSequence**
- **Loop Animation = false**

`Loop = false` — состояние доигрывает и выходит через transition `!bShouldLand`.

---

#### DashStart

Используй **Sequence Evaluator** (не Player) — позиция в анимации должна
точно соответствовать прогрессу dash из C++:

1. Добавь **Sequence Evaluator** (Right Click → Sequence Evaluator)
2. В Details → **Sequence** → Expose as Pin → подключи **Get CachedDashStartSequence**
3. Для пина **Explicit Time**:
   - Добавь **Multiply (float)**
   - A = **Get DashProgressAlpha**
   - B = **Get Sequence Length** → подключи **Get CachedDashStartSequence**
4. Выход → **Output Pose**

> При cancel dash на 30% — анимация стоит на 30%, не доигрывает.

---

#### DashLoop

- `Sequence Player`, Sequence → **Get CachedDashLoopSequence**, Loop = true

---

#### DashEnd

- `Sequence Player`, Sequence → **Get CachedDashEndSequence**, Loop = false

---

#### LadderStart

- `Sequence Player`, Sequence → **Get CachedLadderStartSequence**, Loop = false

---

#### LadderLoop

1. Добавь **Blend Space Player**
2. Expose as Pin → **Blend Space** → подключи **Get CachedLadderLoopBS**
3. **X** → добавь **Abs (float)** → вход = **Get LadderClimbInput** → выход → X
4. Loop = true
5. Выход → **Output Pose**

> Abs нужен: BlendSpace ось 0..MaxSpeed, а LadderClimbInput может быть отрицательным.
> LadderSpeed (фактическая скорость из C++) лучше, если лестница имеет момент инерции.

---

#### LadderEnd

- `Sequence Player`, Sequence → **Get CachedLadderEndSequence**, Loop = false

---

#### LedgeGrabStart

- `Sequence Player`, Sequence → **Get CachedLedgeGrabStartSequence**, Loop = false

---

#### LedgeGrabLoop

- `Sequence Player`, Sequence → **Get CachedLedgeGrabLoopSequence**, Loop = true

---

#### LedgeClimb

`Sequence Evaluator` — прогресс из C++:
- Sequence → **Get CachedLedgeClimbSequence**
- Explicit Time = **Get LedgeGrabEndTimeAlpha** × **Get Sequence Length(CachedLedgeClimbSequence)**

---

#### WallSlide

- `Sequence Player`, Sequence → **Get CachedWallSlideSequence**, Loop = true

---

#### WallJump

- `Sequence Player`, Sequence → **Get CachedWallJumpSequence**, Loop = false

---

#### Death

**Reference Pose** → **Output Pose**.

Death анимация идёт через Montage → DefaultSlot в `ABP_PlatformerBase`.
Runtime flow запускается в `APlatformerCharacterBase`: сначала выключается player input, затем проигрывается `Anim.Combat.Death`, затем выдерживается `DeathPostAnimationDelay = 0.3s`, и только после этого `APlatformerPlayerControllerBase::HandleControlledCharacterDeath()` передает управление project-specific меню или reload flow.

## Часть 7: State Machine — все переходы

### Как добавить переход

Наведи курсор на состояние-источник → появится кнопка `+` по краям → потяни к
состоянию-цели. Кликни на стрелку → откроется граф условия. Построй условие
из переменных `PlatformerAnimInstance` → подключи к **Result**.

В Details каждого перехода:
```
Automatic Rule Based on Sequence Player Remaining Time = false
```

### О приоритете переходов

Приоритет перехода определяется его **порядком в списке** в Details панели исходного
состояния: `1` = наивысший, проверяется первым. Если условие `1` истинно — переходы
`2`, `3`... не проверяются.

В таблицах ниже колонка `#` — это позиция в списке переходов состояния в редакторе.

---

### State Alias — как создать

**State Alias** позволяет задать один переход сразу из нескольких состояний вместо
того чтобы дублировать его для каждого.

Как создать:
1. В State Machine кликни правой кнопкой → **Add State Alias**
2. Назови (например, `SA_DashAny`)
3. Кликни на alias → в **Details → Aliased States** поставь галочки у нужных состояний
4. Потяни переход FROM alias TO целевого состояния как обычно

**State Aliases в этом проекте:**

| Alias | Состояния |
|---|---|
| `SA_AllStates` | все состояния кроме Death |
| `SA_LocomotionGroup` | Locomotion, AirJump, AirFall, Land |
| `SA_DashAny` | DashStart, DashLoop, DashEnd |
| `SA_LadderActive` | LadderStart, LadderLoop |
| `SA_LedgeHanging` | LedgeGrabStart, LedgeGrabLoop |

---

### SA_AllStates → Death

Из любого состояния. Этот alias создаётся первым и имеет наивысший приоритет
в каждом состоянии — должен стоять **первым** в списке переходов.

| # | От | К | Условие | Duration |
|---|---|---|---|---|
| 1 | SA_AllStates | Death | `bIsDead` | 0.15s |

---

### SA_LocomotionGroup → Traversal

Traversal-переходы одинаковы для Locomotion, AirJump, AirFall и Land.
Создаются один раз через alias. Стоят сразу после Death.

| # | К | Условие | Duration |
|---|---|---|---|
| 2 | DashStart | `bShouldDashStart && bHasDash` | 0.05s |
| 3 | DashLoop | `bShouldDashLoop && bHasDash` | 0.05s |
| 4 | LadderStart | `bShouldLadderStart && bHasLadder` | 0.1s |
| 5 | LedgeGrabStart | `bShouldLedgeGrabStart && bHasLedgeGrab` | 0.05s |
| 6 | WallSlide | `bShouldWallSlide && bHasWall` | 0.05s |

> `!bIsDead` можно убрать из условий — `SA_AllStates → Death` с приоритетом 1 уже
> гарантирует что при смерти мы уйдём в Death раньше любого traversal перехода.

---

### Locomotion — индивидуальные переходы

(После приоритетов 1–6 от SA_AllStates и SA_LocomotionGroup)

| # | К | Условие | Duration |
|---|---|---|---|
| 7 | AirJump | `bShouldJump` | 0.05s |
| 8 | AirFall | `bShouldFall` | 0.1s |

---

### SA_DashAny — cancel переход

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | AirJump | `bShouldJump` | 0.05s |

> Приоритет 2 выше внутренних dash-переходов (DashStart→DashLoop и т.д.),
> поэтому прыжок мгновенно прерывает dash на любой фазе.

---

### DashStart — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | DashLoop | `bShouldDashLoop` | 0.0s |

---

### DashLoop — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | DashEnd | `bShouldDashEnd` | 0.05s |

---

### DashEnd — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | AirFall | `!bShouldDashEnd && bIsInAir` | 0.1s |
| 4 | Locomotion | `!bShouldDashEnd && !bIsInAir` | 0.1s |

---

### SA_LadderActive — общий выход при прыжке с лестницы

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | AirFall | `!bShouldLadderLoop && !bShouldLadderEnd && bIsInAir` | 0.05s |

---

### LadderStart — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | LadderLoop | `bShouldLadderLoop` | 0.05s |

---

### LadderLoop — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | LadderEnd | `bShouldLadderEnd` | 0.1s |

---

### LadderEnd — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | Locomotion | `!bShouldLadderEnd && !bIsInAir` | 0.1s |
| 3 | AirFall | `!bShouldLadderEnd && bIsInAir` | 0.05s |

---

### SA_LedgeHanging — выход при срыве с уступа

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | AirFall | `!bShouldLedgeGrabStart && !bShouldLedgeGrabLoop && !bShouldLedgeGrabEnd` | 0.05s |

---

### LedgeGrabStart — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | LedgeGrabLoop | `bShouldLedgeGrabLoop` | 0.0s |

---

### LedgeGrabLoop — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 3 | LedgeClimb | `bShouldLedgeGrabEnd` | 0.05s |

---

### LedgeClimb — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | Locomotion | `!bShouldLedgeGrabEnd && !bIsInAir` | 0.1s |
| 3 | AirFall | `!bShouldLedgeGrabEnd && bIsInAir` | 0.05s |

---

### WallSlide — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | WallJump | `bShouldWallJump` | 0.0s |
| 3 | AirFall | `!bShouldWallSlide && !bShouldWallJump && bIsInAir` | 0.05s |

---

### WallJump — индивидуальные переходы

| # | К | Условие | Duration |
|---|---|---|---|
| 1 | Death | `bIsDead` | 0.15s |
| 2 | AirJump | `!bShouldWallJump && bShouldJump` | 0.05s |
| 3 | AirFall | `!bShouldWallJump && bShouldFall` | 0.05s |

---

### AirJump — индивидуальные переходы

(После приоритетов 1–6 от SA_AllStates и SA_LocomotionGroup)

| # | К | Условие | Duration |
|---|---|---|---|
| 7 | AirFall | `bShouldFall` | 0.1s |
| 8 | Locomotion | `bShouldIdle \|\| bShouldMove` | 0.1s |

---

### AirFall — индивидуальные переходы

(После приоритетов 1–6 от SA_AllStates и SA_LocomotionGroup)

| # | К | Условие | Duration |
|---|---|---|---|
| 7 | Land | `bShouldLand && bHasLand` | 0.05s |
| 8 | Locomotion | `(bShouldIdle \|\| bShouldMove) && !bHasLand` | 0.1s |

---

### Land — индивидуальные переходы

(После приоритетов 1–6 от SA_AllStates и SA_LocomotionGroup)

| # | К | Условие | Duration |
|---|---|---|---|
| 7 | AirJump | `bShouldJump` | 0.05s |
| 8 | Locomotion | `!bShouldLand` | 0.05s |

> Приоритет 7: `AirJump` выше `Locomotion` — double-jump cancel во время landing recovery.

---

### Итоговый список переходов в редакторе (пример для Locomotion)

Для наглядности — как выглядит список переходов Locomotion в Details UE Editor:

```
Transition List (Locomotion):
  1. SA_AllStates      → Death          [bIsDead]
  2. SA_LocomotionGroup → DashStart     [bShouldDashStart && bHasDash]
  3. SA_LocomotionGroup → DashLoop      [bShouldDashLoop && bHasDash]
  4. SA_LocomotionGroup → LadderStart   [bShouldLadderStart && bHasLadder]
  5. SA_LocomotionGroup → LedgeGrabStart [bShouldLedgeGrabStart && bHasLedgeGrab]
  6. SA_LocomotionGroup → WallSlide     [bShouldWallSlide && bHasWall]
  7. Locomotion        → AirJump        [bShouldJump]
  8. Locomotion        → AirFall        [bShouldFall]
```

## Часть 8: Назначить ABP_PlatformerLayersExample на персонажа

```
Mesh:
  Anim Class = ABP_PlatformerBase

Character Defaults:
  Linked Anim Layer Class = ABP_PlatformerLayersExample
  Locomotion Anim Set     = DA_Dragon_LocomotionAnimSet
```

---

## Часть 9: Проверка в PIE

| Действие | Ожидание |
|---|---|
| Стоять | Locomotion BlendSpace при X=0 (Idle) |
| Бежать | Locomotion BlendSpace при X≈600 (Run) |
| Прыгнуть | AirJump → AirFall при падении |
| Приземлиться | Land → Locomotion (если `bHasLand`) |
| Приседать | CrouchIdle / CrouchMove |
| Dash | DashStart → DashLoop → DashEnd |
| Dash + прыжок | DashLoop → AirJump немедленно (cancel) |
| Лестница без input | LadderLoop BlendSpace X=0, статичная поза |
| Лестница с input | LadderLoop BlendSpace X=1, анимация подъёма |
| Уступ | LedgeGrabStart → LedgeGrabLoop → LedgeClimb → Locomotion |

---

## Частые ошибки

### PlatformerLocomotionAnimSet не появляется в поиске DataAsset

Проект не скомпилирован после добавления C++ файлов. Нажми **Tools → Compile**.

### Персонаж в T-pose

- `Linked Anim Layer Class = ABP_PlatformerLayersExample` не назначен
- `ABP_PlatformerLayersExample → Class Settings → Interfaces` не содержит `ALI_PlatformerLayers`
- `LocomotionLayer` граф не подключён к Output Pose

### bHasDash = false хотя sequences назначены

- `Locomotion Anim Set` не назначен на персонажа в поле `Locomotion Anim Set`
- Проект не перекомпилирован после последних C++ изменений → нажми **Tools → Compile**

### LadderLoop играет всё время, не реагирует на input

- Используется Sequence Player вместо Blend Space Player
- Убедись что `CachedLadderLoopBS` — это именно BlendSpace, а не Sequence

### Land поза не играет

- `CachedLandSequence` null — назначь `Land Sequence` в DataAsset
- Transition AirFall → Land: условие `bShouldLand && bHasLand`
