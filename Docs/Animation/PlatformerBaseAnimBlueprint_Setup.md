# Инструкция: полная настройка ABP_PlatformerBase

Документ описывает полную настройку `ABP_PlatformerBase` — host Animation Blueprint
плагина `CookieBrosPlatformer`. Написан с расчётом на читателя с минимальным опытом
работы с AnimGraph.

Фокус документа — reusable plugin layer, не project-specific DragonSlayer animation.

---

## Что такое ABP_PlatformerBase и зачем он нужен

В Unreal Engine Animation Blueprint (AnimBP) — это файл, который описывает как
вычисляется итоговая поза персонажа каждый кадр. Он состоит из двух частей:

- **AnimGraph** — граф, который собирает итоговую pose из нод
- **Event Graph** — Blueprint-логика (в нашем случае почти пустой, всё делает C++)

`ABP_PlatformerBase` — это **host** AnimBP. Он:
- назначается напрямую на Skeletal Mesh персонажа
- собирает финальную pose из locomotion layer и ability montage
- содержит `DefaultSlot` — точку куда GAS играет ability animations (выстрел, удар, хит)
- **не** содержит state machine с анимациями — это задача `ABP_Dragon` (linked layer)

`ABP_Dragon` (или `ABP_PlatformerLayersExample`) — это **linked layer** AnimBP. Он:
- содержит state machine со всеми состояниями и переходами
- читает переменные из `UPlatformerAnimInstance` (C++)
- подключается к персонажу через `LinkedAnimLayerClass`
- **не** назначается напрямую на Mesh

Итоговая runtime схема:

```
Skeletal Mesh
  Anim Class        = ABP_PlatformerBase    ← host, назначается на Mesh
  LinkedAnimLayerClass = ABP_Dragon         ← state machine, подключается через C++
  AnimDataAsset     = DA_Dragon_AnimData    ← GameplayTag → Montage mapping
  LocomotionAnimSet = DA_Dragon_LocomotionAnimSet ← sequences и blendspaces
```

---

## Часть 1: Создание ALI_PlatformerLayers

`ALI_PlatformerLayers` — это **Linked Anim Layer Interface**. Он описывает контракт:
какие именно layer graphs должен реализовать `ABP_Dragon`. Без него host не знает
какие функции вызывать.

### Шаг 1.1: Создать Interface asset

Открой Content Browser. Перейди в папку:

```
Plugins/CookieBrosPlatformer/Content/Animation
```

Кликни правой кнопкой в пустом месте. В меню выбери:

```
Animation → Animation Layer Interface
```

Назови файл:

```
ALI_PlatformerLayers
```

Открой его двойным кликом.

### Шаг 1.2: Добавить layers

Ты увидишь пустой интерфейс с кнопкой `Add` в левой панели.

Нажми `Add` и добавь один layer:

```
Name = LocomotionLayer
```

Этот layer будет отвечать за всю locomotion — idle, run, jump, dash, ladder и т.д.

Нажми **Compile** и **Save**.

> Если в будущем понадобится отдельный layer для traversal overlay или combat —
> добавишь сюда. Пока достаточно одного `LocomotionLayer`.

---

## Часть 2: Создание ABP_PlatformerBase

### Шаг 2.1: Создать Animation Blueprint

Открой Content Browser. Перейди в:

```
Plugins/CookieBrosPlatformer/Content/Animation
```

Кликни правой кнопкой. В меню выбери:

```
Animation → Animation Blueprint
```

Откроется диалог с двумя полями:

**Target Skeleton** — выбери скелет персонажа, например:
```
SK_Mannequin   (или SK_Dragon, или любой скелет твоего персонажа)
```

**Parent Class** — нажми на выпадающий список, введи в поиске:
```
PlatformerAnimInstance
```
Выбери `PlatformerAnimInstance`.

Нажми **Create**. Назови файл:

```
ABP_PlatformerBase
```

### Шаг 2.2: Проверить Parent Class

Открой `ABP_PlatformerBase` двойным кликом.

В верхней панели нажми **Class Settings**.

В правой панели Details найди раздел **Class**:

```
Parent Class = PlatformerAnimInstance   ← должно быть так
```

Если там стоит `AnimInstance` — смени вручную на `PlatformerAnimInstance`.

### Шаг 2.3: Добавить Interface

В том же **Class Settings** в правой панели найди раздел **Interfaces**.

Нажми кнопку **Add** рядом с "Implemented Interfaces".

В поиске введи:
```
ALI_PlatformerLayers
```

Выбери и добавь.

После добавления в левой панели (My Blueprint) появятся tabs с названиями layers.
В нашем случае появится tab `LocomotionLayer`. Это fallback graph — он нужен чтобы
AnimBP компилировался без подключённого `ABP_Dragon`. Настроим его позже.

Нажми **Compile**. Ошибок быть не должно.

---

## Часть 3: Настройка AnimGraph

AnimGraph — это главная вкладка где строится граф pose.

Перейди в вкладку **AnimGraph** (по умолчанию открыта).

Ты увидишь одну ноду — `Output Pose` с красным кружком. Это финальная точка графа —
всё что подключено к ней определяет позу персонажа.

### Шаг 3.1: Добавить Linked Anim Graph (LocomotionLayer)

Кликни правой кнопкой в пустом месте AnimGraph. Откроется меню поиска.

Введи в поиске:
```
Linked Anim Graph
```

В списке найди **Linked Anim Graph** (не Layer, а именно Graph). Добавь его.

В Details этой ноды найди поле **Layer** и выбери:
```
LocomotionLayer
```

Эта нода каждый кадр вызывает `LocomotionLayer` graph из `ABP_Dragon`
(или любого другого подключённого linked AnimBP) и возвращает его pose.

Выглядит нода так:

```
[ Linked Anim Graph ]
[ Layer: LocomotionLayer ]
        ↓
    (output pin)
```

### Шаг 3.2: Добавить Save Cached Pose — BaseMovementPose

Кликни правой кнопкой. Введи в поиске:
```
Save Cached Pose
```

Добавь ноду. В Details или прямо на ноде измени имя:
```
BaseMovementPose
```

Соедини выход `Linked Anim Graph` со входом `Save Cached Pose`:

```
[Linked Anim Graph: LocomotionLayer] ──→ [Save Cached Pose: BaseMovementPose]
```

Чтобы соединить — потяни мышью от выходного пина одной ноды к входному пину другой.

> `Save Cached Pose` запоминает pose в памяти. Это нужно потому что
> одну и ту же locomotion pose нам понадобится использовать в двух местах одновременно.

### Шаг 3.3: Добавить Use Cached Pose — первое использование

Кликни правой кнопкой. Введи:
```
Use Cached Pose
```

Добавь ноду. В выпадающем списке выбери:
```
BaseMovementPose
```

Эта нода читает сохранённую pose. Расположи её правее `Save Cached Pose`.

### Шаг 3.4: Добавить Slot (DefaultSlot)

Кликни правой кнопкой. Введи:
```
Slot
```

Выбери **Slot (Animation)**. Добавь.

В Details найди поле **Slot Name** и введи:
```
DefaultSlot
```

> `DefaultSlot` — это точка куда GAS проигрывает ability montages.
> Когда montage не играет — нода пропускает входную pose насквозь без изменений.
> Когда montage играет — нода возвращает pose из montage.

Соедини:
```
[Use Cached Pose: BaseMovementPose] ──→ [Slot: DefaultSlot]
```

### Шаг 3.5: Добавить Save Cached Pose — AbilitySlotPose

Кликни правой кнопкой. Добавь ещё одну `Save Cached Pose`. Назови:
```
AbilitySlotPose
```

Соедини:
```
[Slot: DefaultSlot] ──→ [Save Cached Pose: AbilitySlotPose]
```

Теперь у тебя есть две сохранённые pose:
- `BaseMovementPose` — чистая locomotion без montage
- `AbilitySlotPose` — locomotion с наложенным montage (или та же поза если montage не играет)

### Шаг 3.6: Добавить два Use Cached Pose для Layered Blend

Добавь два `Use Cached Pose`:
- Первый → выбери `BaseMovementPose`
- Второй → выбери `AbilitySlotPose`

Расположи их рядом, они понадобятся для следующего шага.

### Шаг 3.7: Добавить Layered Blend Per Bone

Кликни правой кнопкой. Введи:
```
Layered Blend Per Bone
```

Добавь ноду.

Эта нода смешивает две pose по скелету: нижняя часть тела берётся из одной pose,
верхняя — из другой.

**Подключи пины:**

```
Base Pose  ←── [Use Cached Pose: BaseMovementPose]   (ноги/таз от locomotion)
Blend Pose ←── [Use Cached Pose: AbilitySlotPose]    (верхнее тело от montage)
```

У ноды может быть только один `Blend Pose` pin по умолчанию.
Если нужно добавить — в Details нажми `+` рядом с `Layer Setup`.

**Настрой Branch Filter:**

В Details найди раздел:
```
Layer Setup → [0] → Branch Filters → [+]
```

Нажми `+` чтобы добавить фильтр. Заполни:

```
Bone Name   = spine_01
Blend Depth = 0
```

`spine_01` — кость в основании позвоночника. Всё что выше неё (грудь, руки, голова)
получит pose из `AbilitySlotPose` (montage). Всё ниже (таз, ноги) останется от
`BaseMovementPose` (locomotion).

`Blend Depth = 0` — рабочий contract для текущего `ABP_PlatformerBase` / UE 5.6:
применить branch blend от `spine_01` на верх тела. Не ставь `-1`: для branch filter
это исключает ветку из blend, из-за чего montage может успешно запускаться
(`duration > 0` в логе), но визуально не попадать в финальный pose.

Также в Details включи:
```
Mesh Space Rotation Blend = true
```

Это улучшает качество blend при поворотах.

### Шаг 3.8: Подключить к Output Pose

Соедини выход `Layered Blend Per Bone` с нодой `Output Pose`:

```
[Layered Blend Per Bone] ──→ [Output Pose]
```

### Итоговая схема AnimGraph

После всех шагов граф должен выглядеть так (слева направо):

```
[Linked Anim Graph: LocomotionLayer]
              ↓
[Save Cached Pose: BaseMovementPose]
       ↙                    ↘
[Use Cached Pose]      [Use Cached Pose]
  BaseMovementPose       BaseMovementPose
       ↓                      ↓
[Slot: DefaultSlot]    (пока в стороне)
       ↓
[Save Cached Pose: AbilitySlotPose]
       ↓
[Use Cached Pose: AbilitySlotPose]
       ↓
                    ┌──────────────────────────┐
[Use Cached Pose:   │  Layered Blend Per Bone  │──→ [Output Pose]
 BaseMovementPose] →│  Base Pose               │
[Use Cached Pose:  →│  Blend Pose [0]          │
 AbilitySlotPose]   │  Filter: spine_01, 0     │
                    └──────────────────────────┘
```

Нажми **Compile**. Если есть ошибки — смотри раздел «Частые ошибки» в конце.

---

## Часть 4: Настройка fallback LocomotionLayer

В левой панели (My Blueprint) в разделе **Graphs** ты увидишь вкладку `LocomotionLayer`.
Это fallback — он используется когда `LinkedAnimLayerClass` не назначен.

Перейди в неё (двойной клик).

Ты увидишь пустой граф с `Output Pose`.

Добавь ноду `Reference Pose`:

```
Right Click → Search "Reference Pose" → добавь
```

Соедини:
```
[Reference Pose] ──→ [Output Pose]
```

Это минимальный fallback — персонаж встанет в T-pose если linked layer не подключён.
Этого достаточно для компиляции и дебага.

Нажми **Compile** и **Save**.

---

## Часть 5: Настройка ABP_Dragon (linked layer)

`ABP_Dragon` — это AnimBP персонажа, который содержит state machine.

### Шаг 5.1: Создать Animation Blueprint

В Content Browser перейди в:
```
Content/Characters/Dragon/Animation
```

Кликни правой кнопкой:
```
Animation → Animation Blueprint
```

Target Skeleton — скелет Dragon. Parent Class:
```
PlatformerAnimInstance
```

Назови:
```
ABP_Dragon
```

### Шаг 5.2: Настроить Class Settings

Открой `ABP_Dragon`. Нажми **Class Settings**.

**Parent Class:**
```
PlatformerAnimInstance
```

**Implemented Interfaces → Add:**
```
ALI_PlatformerLayers
```

После добавления интерфейса в левой панели появится вкладка `LocomotionLayer`.

### Шаг 5.3: Настроить LocomotionLayer graph

Перейди во вкладку `LocomotionLayer` (в My Blueprint → Graphs).

Здесь будет полный state machine. Подробная инструкция по его настройке —
в документе [AnimBP_LocomotionLayers_Setup.md](AnimBP_LocomotionLayers_Setup.md).

Для минимальной проверки можно сначала вставить один `Sequence Player`
с любой idle анимацией и подключить к `Output Pose`.

### Шаг 5.4: Event Graph

В `ABP_Dragon` **Event Graph должен быть пустым**.

Все переменные (скорость, флаги прыжка, dash и т.д.) уже обновляет C++
(`UPlatformerAnimInstance`) каждый тик. Не нужно:

```
❌ Try Get Pawn Owner
❌ Cast to Character
❌ Вычислять GroundSpeed вручную
❌ Читать теги из ASC
❌ Запускать montage
```

В `Event Blueprint Initialize Animation` — только кэширование sequences из
`LocomotionAnimSet`. Подробнее в [AnimBP_LocomotionLayers_Setup.md](AnimBP_LocomotionLayers_Setup.md).

---

## Часть 6: Назначить на персонажа

Открой Blueprint персонажа (например `BP_PlayableDragonCharacter`).

### На Skeletal Mesh компоненте:

Выбери компонент `Mesh` в Components панели. В Details:

```
Animation
  Animation Mode = Use Animation Blueprint
  Anim Class     = ABP_PlatformerBase       ← host AnimBP
```

### В Defaults персонажа (или Character C++ defaults):

```
Animation
  LinkedAnimLayerClass = ABP_Dragon          ← state machine AnimBP
  AnimDataAsset        = DA_Dragon_AnimData  ← GameplayTag → Montage
  LocomotionAnimSet    = DA_Dragon_LocomotionAnimSet ← sequences и blendspaces
```

> Не назначай `ABP_Dragon` напрямую на Mesh вместо `ABP_PlatformerBase`.
> Тогда host graph с `DefaultSlot` и upper-body blend не будет использоваться
> и ability montages (выстрелы, удары) не будут работать корректно.

---

## Часть 7: Проверка в PIE

Запусти Play In Editor.

### Проверка locomotion

Открой Animation Debug: во вкладке `Window → Animation → Animation Insights`
или просто наблюдай персонажа визуально.

| Действие | Ожидаемый результат |
|---|---|
| Стоять | Idle анимация |
| Бежать | Run анимация |
| Прыгнуть | Jump → Fall |
| Приземлиться | Landing (если назначен `LandSequence`) → Idle |

### Проверка ability montage

Активируй ability выстрела через input.

| Ситуация | Ожидаемый результат |
|---|---|
| Выстрел стоя | верхнее тело играет montage, ноги — idle |
| Выстрел в беге | верхнее тело играет montage, ноги продолжают бежать |

Если ноги останавливаются при выстреле — `DefaultSlot` стоит после
`Layered Blend Per Bone`, а должен быть до. Перепроверь схему в Шаге 3.4–3.7.

---

## Частые ошибки

### Ошибка компиляции: "Linked anim layer node can only be used once"

Причина: одна и та же `Linked Anim Graph` нода подключена в двух местах финального
графа одновременно.

Решение: вызывай каждый linked layer один раз, сохраняй результат в
`Save Cached Pose`, далее используй `Use Cached Pose`.

### Ability montage не играет

Проверь по очереди:

```
1. Mesh → Anim Class = ABP_PlatformerBase   (не ABP_Dragon)
2. Character → AnimDataAsset назначен
3. В DA_Dragon_AnimData есть запись: AnimTag = Anim.Combat.RangedShot, Montage = AM_...
4. Montage → Slot Name = DefaultSlot
5. Skeleton montage совпадает со skeleton персонажа
```

### Персонаж в T-pose

Причина: `LinkedAnimLayerClass` не назначен или `ABP_Dragon` не реализует
`ALI_PlatformerLayers`.

Проверь:
```
1. Character → LinkedAnimLayerClass = ABP_Dragon
2. ABP_Dragon → Class Settings → Implemented Interfaces = ALI_PlatformerLayers
3. ABP_Dragon → LocomotionLayer graph подключён к Output Pose
```

### Locomotion не реагирует на движение

Причина: в `ABP_Dragon → LocomotionLayer` state machine не читает переменные
`UPlatformerAnimInstance`.

Проверь:
```
1. ABP_Dragon → Parent Class = PlatformerAnimInstance
2. В state machine используются переменные bShouldIdle, bShouldMove и т.д.
   (не самостоятельно вычисленные)
```

---

## Архитектурные правила

### ABP_PlatformerBase содержит

```
✓ Linked Anim Graph ноды (вызовы layer interface)
✓ Save / Use Cached Pose ноды
✓ Slot: DefaultSlot
✓ Layered Blend Per Bone (upper-body montage blend)
✓ Output Pose
```

### ABP_PlatformerBase НЕ содержит

```
✗ State machine с анимациями
✗ Animation sequences или blendspaces
✗ GameplayTag проверки
✗ Blueprint логику расчёта движения
✗ Dragon-specific или project-specific код
```

### ABP_Dragon содержит

```
✓ LocomotionLayer graph со state machine
✓ Sequence Player / Evaluator ноды
✓ Blendspace Player ноды
✓ AimOffset (если используется Look система)
✓ Transition rules основанные на переменных UPlatformerAnimInstance
```

### ABP_Dragon НЕ содержит

```
✗ Slot ноды (это зона ABP_PlatformerBase)
✗ Layered Blend Per Bone (это зона ABP_PlatformerBase)
✗ Spawn projectile / Activate Ability / Play Montage вручную
```
