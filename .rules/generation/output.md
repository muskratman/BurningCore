# Output Contract: DragonSlayer

## Response Format

### Code Changes

1. Покажи **де** (файл, клас, функція) і **що** змінюється
2. Якщо зміна перетинає межу `Plugins/CookieBrosPlatformer` ↔ `Source/DragonSlayer`, коротко поясни ЧОМУ код живе саме там
3. Snippet ДО → ПІСЛЯ (або повний файл для нових)
4. Якщо зміна торкається `Build.cs`, `.uproject` або reusable base у плагіні → позначити явно

### Нові класи

Подай повні .h + .cpp файли. Структура:

```
// .h: copyright → #pragma once → includes → forward decl → class
// .cpp: matching .h → Engine includes → project includes → implementation
```

### Пояснення

- Коротко: що, чому, як це впливає на архітектуру
- Не пояснюй очевидне (що таке UPROPERTY, як працює Cast)
- ЧОМУ за кожним неочевидним рішенням
- Якщо правиться документація, вкажи новий source of truth

## Language

- Код: англійська (коментарі, naming, log messages)
- Пояснення: мова користувача (UA/RU/EN — адаптується)

## Build Verification

- Після значних змін: «Build → перевір компіляцію»
- Для gameplay/UI змін без тест-фреймворку нагадуй про PIE/manual verification
- Не запускай build автоматично без підтвердження

## Error Handling

- Помилка компіляції → покажи помилку + виправлення
- Architectural concern → ескалюй (⚠️), не виправляй мовчки
