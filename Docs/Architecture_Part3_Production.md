# DragonSlayer — Game Architecture (Part 3: Production)

Part 3 описує production rules для щоденної роботи в поточному проекті.

---

## 1. Ownership In Practice

### Architect owns

- reusable base у `Plugins/CookieBrosPlatformer`
- cross-cutting framework glue у `Source/DragonSlayer/Core`
- `Source/DragonSlayer/Platformer/Base`
- docs, `.rules/`, `Build.cs`, `.uproject`
- рішення про перенесення логіки між plugin і source

### Gameplay owns

- `Source/DragonSlayer/Character`
- `Source/DragonSlayer/AI`
- `Source/DragonSlayer/GAS`
- `Source/DragonSlayer/Data`
- `Source/DragonSlayer/Systems`
- `Source/DragonSlayer/Projectiles`
- project-specific gameplay glue у `Source/DragonSlayer/Platformer`

### UI owns

- `Source/DragonSlayer/UI`
- `Source/DragonSlayer/Core/UI`
- project-specific UMG/menu/HUD widgets

---

## 2. Production Checklist For Any New Feature

1. Визнач, чи фіча reusable для інших платформерів, чи специфічна для DragonSlayer.
2. Обери базовий клас або shell, який уже існує.
3. Додай тільки мінімально потрібне розширення.
4. Якщо потрібен tuning, віддай його в DataAsset / settings / BP defaults.
5. Якщо потрібен UI feedback, підключай його через project UI layer, а не через gameplay hard refs.
6. Перевір вручну в PIE.

---

## 3. Where New Code Should Go

| Якщо задача про... | Класти сюди |
|---|---|
| generic camera / checkpoint / traversal / environment / reusable widget | `Plugins/CookieBrosPlatformer` |
| Dragon forms / Overdrive / Dragon abilities / project progression | `Source/DragonSlayer` |
| current level flow glue / playable character bindings | `Source/DragonSlayer/Platformer` |
| main menu / HUD / pause UX | `Source/DragonSlayer/Core/UI` або `Source/DragonSlayer/UI` |
| editor-only import tooling | `CookieBrosLevelEditor` |

---

## 4. Production Priorities

### Dragon Gameplay

Поточний production path:

1. movement / traversal feel
2. jump / dash / crouch / glide / fly glue
3. base shot / charge shot
4. hit reaction / damage readability
5. forms
6. Overdrive
7. enemy encounters and boss flow

### UI

1. health readability
2. pause/settings usability
3. main menu flow
4. developer tuning widgets when they help gameplay iteration

### Persistence

1. correct save type and load validation
2. checkpoint and level progression
3. forms/upgrades/secrets

---

## 5. Integration Rules

- Не дублюй reusable plugin class у `Source/DragonSlayer`.
- Не змішуй HUD/menu logic із core gameplay rules.
- Не роби save system залежним від widgets.
- Не прив'язуй AI до конкретного HUD або menu flow.
- Не додавай Dragon-specific knowledge в reusable plugin without reuse value.
- Не додавай новий manager, якщо вистачає існуючого GameInstance / GameMode / Component / GAS / Interface / DataAsset.

---

## 6. Camera Production Rule

`APlatformerCameraManager` залишається reusable side-view camera shell.

Поточна очікувана поведінка:

- smooth follow
- horizontal/vertical look-ahead
- жодного hard lock через `Camera XMin Bounds` / `Camera XMax Bounds`

Якщо майбутня camera feature generic і може знадобитися іншим platformer-проектам, вона розглядається як plugin work.

---

## 7. Verification

- Формального test framework зараз немає.
- Основна перевірка gameplay/UI змін — ручний запуск через Play In Editor.
- Build verification: Build task в IDE.
- Якщо щось не перевірялось, це потрібно явно сказати в close-out.

---

## 8. Current Production Truth

- Назва проекту: `DragonSlayer`
- Версія рушія: `UE 5.6`
- База платформерних механік: `Plugins/CookieBrosPlatformer`
- Проектно-специфічна логіка: `Source/DragonSlayer`

Цей документ замінює старі припущення про legacy single-module layout і variant-based структуру.
