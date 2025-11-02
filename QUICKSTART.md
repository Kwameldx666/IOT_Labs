# Быстрый старт - Новая архитектура

## 🎯 Основная идея

**ДО:** Весь код в одной папке, дублирование, сложно переиспользовать.

**ПОСЛЕ:** Переиспользуемые компоненты в `lib/`, лабораторные используют готовые блоки.

---

## 📚 Доступные библиотеки (lib/)

### 1. TaskScheduler - Планировщик задач
```cpp
#include "task_scheduler.hpp"

void myTask(unsigned long now) {
  // Ваш код
}

Task tasks[] = {{myTask, 100, 0}};  // Каждые 100ms
TaskScheduler scheduler(tasks, 1);

void setup() {
  scheduler.init(millis());
}

void loop() {
  scheduler.runOnce();
}
```

### 2. Debouncer - Подавление дребезга
```cpp
#include "debouncer.hpp"

Debouncer btn(35);  // 35ms debounce

void loop() {
  bool pressed = digitalRead(PIN) == LOW;
  if (btn.updateRisingEdge(pressed, millis())) {
    // Нажата!
  }
}
```

### 3. ButtonController - Кнопка с debouncing
```cpp
#include "button_controller.hpp"

ButtonController btn(7);  // Pin 7

void setup() {
  btn.begin();
}

void loop() {
  if (btn.wasPressed(millis())) {
    // Нажата!
  }
}
```

### 4. LedController - Управление LED
```cpp
#include "led_controller.hpp"

LedController led(13);

void setup() {
  led.begin();
}

void loop() {
  led.toggle();
  delay(1000);
}
```

---

## ✨ Пример: Создание Lab 3

### Шаг 1: Создайте файлы
```
src/lab3/
  ├── lab3_1.hpp
  ├── lab3_1.cpp
  └── lab3_1_config.hpp
```

### Шаг 2: lab3_1_config.hpp
```cpp
#ifndef LAB3_1_CONFIG_HPP
#define LAB3_1_CONFIG_HPP

constexpr uint32_t kTaskPeriodMs = 100;

#endif
```

### Шаг 3: lab3_1.hpp
```cpp
#ifndef LAB3_1_HPP
#define LAB3_1_HPP

void setup_lab3_1();
void loop_lab3_1();

#endif
```

### Шаг 4: lab3_1.cpp (ИСПОЛЬЗУЙТЕ БИБЛИОТЕКИ!)
```cpp
#include "lab3_1.hpp"
#include "task_scheduler.hpp"
#include "button_controller.hpp"
#include "led_controller.hpp"
#include "config.h"
#include <stdio.h>

// Компоненты
ButtonController toggleBtn(BUTTON_TOGGLE_PIN);
LedController mainLed(LED_PIN);

// Задача
void taskControl(unsigned long now) {
  if (toggleBtn.wasPressed(now)) {
    mainLed.toggle();
    printf("LED toggled!\n");
  }
}

// Scheduler
Task tasks[] = {{taskControl, 25, 0}};
TaskScheduler scheduler(tasks, 1);

void setup_lab3_1() {
  toggleBtn.begin();
  mainLed.begin();
  scheduler.init(millis());
  printf("[Lab 3.1] Ready\n");
}

void loop_lab3_1() {
  scheduler.runOnce();
}
```

### Шаг 5: Добавьте в app_manager.hpp
```cpp
enum class LabSelection {
  // ...existing...
  LAB3_1,  // Добавьте
};

constexpr LabSelection ACTIVE_LAB = LabSelection::LAB3_1;
```

### Шаг 6: Обновите app_manager.cpp
```cpp
#include "lab3/lab3_1.hpp"  // Добавьте

void appManagerSetup() {
  // ...existing cases...
  case LabSelection::LAB3_1:
    setup_lab3_1();
    break;
}

void appManagerLoop() {
  // ...existing cases...
  case LabSelection::LAB3_1:
    loop_lab3_1();
    break;
}
```

### Готово! ✅

Всего **~30 строк кода** в lab3_1.cpp благодаря библиотекам!

---

## 🔍 Когда создавать новую библиотеку?

Создайте библиотеку в `lib/` если:

✅ Код будет использоваться в нескольких лабораторных  
✅ Это общая функциональность (sensor, actuator, algorithm)  
✅ Хотите протестировать отдельно  
✅ Можно использовать в других проектах  

Оставьте в `src/lab*/` если:

✅ Специфичная логика только для этой лабораторной  
✅ Конфигурация/константы  
✅ Структуры состояния  
✅ Setup/loop функции  

---

## 📋 Чеклист для новой лабораторной

- [ ] Проверил доступные библиотеки в `lib/`
- [ ] Использовал `TaskScheduler` для периодических задач
- [ ] Использовал `ButtonController` для кнопок
- [ ] Использовал `LedController` для LED
- [ ] Создал только необходимые файлы в `src/lab*/`
- [ ] Добавил в `app_manager`
- [ ] Проверил компиляцию
- [ ] Написал комментарии

---

## 🆘 FAQ

**Q: Где хранить пины?**  
A: В `include/config.h` (глобальные) или `lab*_config.hpp` (специфичные)

**Q: Как добавить свою библиотеку?**  
A: Создайте папку в `lib/`, добавьте `.hpp` и `.cpp`

**Q: Старые библиотеки (dd_button, dd_led) удалять?**  
A: Пока оставьте для совместимости со старым кодом

**Q: Как переключиться на Lab 1?**  
A: Измените `ACTIVE_LAB` в `app_manager.hpp`

---

**Удачи в разработке! 🚀**

