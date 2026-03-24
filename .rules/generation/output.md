# Output Contract: BurningCORE

## Response Format

### Code Changes

1. Покажи **де** (файл, клас, функція) і **що** змінюється
2. Snippet ДО → ПІСЛЯ (або повний файл для нових)
3. Якщо зміна торкається Build.cs або .uproject → позначити явно

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

## Language

- Код: англійська (коментарі, naming, log messages)
- Пояснення: мова користувача (UA/RU/EN — адаптується)

## Build Verification

- Після значних змін: «Build → перевір компіляцію»
- Не запускай build автоматично без підтвердження

## Error Handling

- Помилка компіляції → покажи помилку + виправлення
- Architectural concern → ескалюй (⚠️), не виправляй мовчки
