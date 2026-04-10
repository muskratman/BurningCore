# Evaluation: DragonSlayer

## Pre-Response Checklist

Перед кожною відповіддю агент повинен перевірити своє рішення за цим списком:

### 1. Architectural Fit
- [ ] Рішення коректно розкладене між `Plugins/CookieBrosPlatformer` і `Source/DragonSlayer`?
- [ ] Перевірено збіг ролі (Architect/Gameplay/UI) з поточними файлами?
- [ ] Generic platformer mechanic не залишився випадково в project module?
- [ ] DragonSlayer-specific knowledge не протекло в reusable plugin?
- [ ] Взаємодія між шарами йде через interfaces, GAS, DataAssets або існуючі framework-механізми без дублювання?

### 2. Code Quality & Rules
- [ ] Input callbacks залишаються тонким glue-шаром між EnhancedInput і gameplay логікою?
- [ ] Всі числові UPROPERTY мають `meta = (ClampMin, ClampMax)`?
- [ ] Для reusable/project boundaries обрано правильний базовий клас або derivation path?
- [ ] Для подій використовуються Blueprint hooks, DataAssets або interfaces замість hardcoded presentation-логіки?
- [ ] Чи додано `FORCEINLINE` до getter методів?

### 3. Permissions Security
- [ ] Рішення намагається змінити `Build.cs` чи `.uproject`? → Потрібне підтвердження (⚠️)
- [ ] Рішення намагається змінити `Config/`? → Потрібне підтвердження (⚠️)
- [ ] Рішення видаляє існуючі `.uasset` файли? → Потрібне підтвердження (❌/⚠️)
- [ ] Змінюються reusable C++ base класи в `Plugins/CookieBrosPlatformer` або framework glue в `Source/DragonSlayer/Core`? → Тільки роль Architect (⚠️)

### 4. Overcomplexity Trigger
- [ ] Чи додає це рішення нову абстракцію, яка використовується тільки один раз?
- [ ] Чи можна вирішити це простіше в межах існуючого класу?
- [ ] Чи не створює це дубль між plugin foundation і project layer?
- [ ] Якщо ефективність < 70%, чи було зроблено крок назад до плану?

## Self-Correction Protocol

Якщо користувач вказує на помилку компіляції:
1. Не намагайся "вгадати" виправлення всліпу.
2. Проаналізуй помилку з урахуванням UE5 C++ специфіки (e.g. відсутній include, forward declaration, cyclic dependency).
3. Якщо помилка архітектурна — повідом про конфлікт з `.rules/` та запропонуй альтернативу.
