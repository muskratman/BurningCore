# Evaluation: BurningCORE

## Pre-Response Checklist

Перед кожною відповіддю агент повинен перевірити своє рішення за цим списком:

### 1. Architectural Fit
- [ ] Рішення використовує `Variant_*` структуру замість редагування Base класів?
- [ ] Перевірено збіг ролі (Architect/Gameplay/UI) з поточними файлами?
- [ ] Взаємодія між модулями йде через Interfaces (напр. `ICombatDamageable`)?

### 2. Code Quality & Rules
- [ ] Використано EnhancedInput `Do*()` патерн для нових Input Actions?
- [ ] Всі числові UPROPERTY мають `meta = (ClampMin, ClampMax)`?
- [ ] Для подій використовуються `BlueprintImplementableEvent` замість hardcoded C++ логіки?
- [ ] Чи додано `FORCEINLINE` до getter методів?

### 3. Permissions Security
- [ ] Рішення намагається змінити `Build.cs` чи `.uproject`? → Потрібне підтвердження (⚠️)
- [ ] Рішення видаляє існуючі `.uasset` файли? → Потрібне підтвердження (❌/⚠️)
- [ ] Змінюються існуючі C++ Base класи (`BurningCORE*.h/.cpp`)? → Тільки роль Architect (⚠️)

### 4. Overcomplexity Trigger
- [ ] Чи додає це рішення нову абстракцію, яка використовується тільки один раз?
- [ ] Чи можна вирішити це простіше в межах існуючого класу?
- [ ] Якщо ефективність < 70%, чи було зроблено крок назад до плану?

## Self-Correction Protocol

Якщо користувач вказує на помилку компіляції:
1. Не намагайся "вгадати" виправлення всліпу.
2. Проаналізуй помилку з урахуванням UE5 C++ специфіки (e.g. відсутній include, forward declaration, cyclic dependency).
3. Якщо помилка архітектурна — повідом про конфлікт з `.rules/` та запропонуй альтернативу.
