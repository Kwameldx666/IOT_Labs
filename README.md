# IOT Labs - Чистая Модульная Архитектура

## 📐 Философия: Максимальная переиспользуемость + Минимум файлов

Проект построен на трёх принципах:
1. **lib/** - переиспользуемые библиотеки (используются везде)
2. **src/lab*/** - один файл на лабораторную (вся логика внутри)
3. **include/config.h** - единая конфигурация (все константы)

## 🗂️ Структура проекта

```
IOT_Labs/
│
├── src/                                    # Код приложения
│   ├── main.cpp                            # Точка входа Arduino
│   ├── app_manager.hpp/cpp                 # Менеджер выбора лабораторных
│   │
│   ├── lab1/                               # Лабораторная 1
│   │   ├── lab1_1.hpp/cpp                  # Задание 1.1 (только логика)
│   │   └── lab1_2.hpp/cpp                  # Задание 1.2 (только логика)
│   │
│   ├── lab2/                               # Лабораторная 2
│   │   ├── lab2_1.hpp                      # Интерфейс
│   │   └── lab2_1.cpp                      # Планировщик задач (единый файл)
│   │
│   └── lab3/                               # Лабораторная 3
│       ├── lab3_1.hpp                      # Интерфейс
│       └── lab3_1.cpp                      # Датчики + FreeRTOS (единый файл)
│
├── lib/                                    # Переиспользуемые библиотеки
│   │
│   ├── task_scheduler/                     # ⭐ Планировщик задач
│   │   ├── task_scheduler.hpp              #    - Периодическое выполнение
│   │   └── task_scheduler.cpp              #    - Управление таймингом
│   │
│   ├── debouncer/                          # ⭐ Debouncing утилита
│   │   ├── debouncer.hpp                   #    - Подавление дребезга
│   │   └── debouncer.cpp                   #    - Детекция фронтов
│   │
│   ├── button_controller/                  # ⭐ Контроллер кнопок
│   │   ├── button_controller.hpp           #    - Чтение кнопок
│   │   └── button_controller.cpp           #    - Встроенный debouncing
│   │
│   ├── led_controller/                     # ⭐ Контроллер LED
│   │   ├── led_controller.hpp              #    - Управление LED
│   │   └── led_controller.cpp              #    - Отслеживание состояния
│   │
│   ├── status_indicator/                   # ⭐ НОВАЯ - Индикация статуса
│   │   ├── status_indicator.hpp            #    - Success/Failure LED
│   │   └── status_indicator.cpp            #    - С задержкой
│   │
│   ├── lcd_helper/                         # ⭐ НОВАЯ - LCD форматирование
│   │   ├── lcd_helper.hpp                  #    - Готовые макеты
│   │   └── lcd_helper.cpp                  #    - Статусы, инпуты
│   │
│   ├── analog_sensor/                      # ⭐ НОВАЯ - Аналоговый датчик
│   │   ├── analog_sensor.hpp               #    - Чтение с фильтрацией
│   │   └── analog_sensor.cpp               #    - Масштабирование, min/max
│   │
│   ├── threshold_manager/                  # ⭐ НОВАЯ - Мониторинг порогов
│   │   ├── threshold_manager.hpp           #    - NORMAL/WARNING/CRITICAL
│   │   └── threshold_manager.cpp           #    - С гистерезисом
│   │
│   ├── dd_button/                          # Устаревшая библиотека кнопок
│   │   ├── dd_button.hpp                   # (используется в старом коде)
│   │   └── dd_button.cpp
│   │
│   ├── dd_led/                             # Устаревшая библиотека LED
│   │   ├── dd_led.hpp                      # (используется в старом коде)
│   │   └── dd_led.cpp
│   │
│   ├── dd_lcd/                             # LCD I2C дисплей
│   │   └── src/
│   │       ├── dd_lcd.hpp
│   │       └── dd_lcd.cpp
│   │
│   ├── keypad/                             # Клавиатура 4x4
│   │   ├── dd_keypad.hpp
│   │   └── dd_keypad.cpp
│   │
│   └── Serial/                             # Serial с printf/scanf
│       ├── dd_serial.hpp
│       └── dd_serial.cpp
│
├── include/                                # Глобальные заголовки
│   └── config.h                            # Конфигурация пинов и констант
│
├── docs/                                   # Документация
│   └── lab2_1.md
│
├── ARCHITECTURE.md                         # Этот файл
├── README.md                               # Описание проекта
└── platformio.ini                          # Конфигурация PlatformIO
```

## 🎯 Новые переиспользуемые библиотеки

### 1. **task_scheduler** - Планировщик задач

**Назначение:** Выполнение периодических задач без blocking delay.

**Использование:**
```cpp
#include "task_scheduler.hpp"

// Определить задачи
void task1(unsigned long now) { /* ... */ }
void task2(unsigned long now) { /* ... */ }

Task tasks[] = {
  {task1, 100, 0},  // Каждые 100ms
  {task2, 200, 0},  // Каждые 200ms
};

TaskScheduler scheduler(tasks, 2);

void setup() {
  scheduler.init(millis());
}

void loop() {
  scheduler.runOnce();
}
```

**Особенности:**
- Автоматическое управление таймингом
- Обработка переполнения таймера
- Поддержка начальных offset'ов
- Легко добавлять/удалять задачи

---

### 2. **debouncer** - Подавление дребезга

**Назначение:** Устранение ложных срабатываний при нажатии кнопок.

**Использование:**
```cpp
#include "debouncer.hpp"

Debouncer btnDebouncer(35);  // 35ms debounce time

void loop() {
  unsigned long now = millis();
  bool pressed = digitalRead(BUTTON_PIN) == LOW;
  
  if (btnDebouncer.updateRisingEdge(pressed, now)) {
    // Кнопка нажата (с debounce)!
  }
}
```

**Особенности:**
- Детекция rising edge
- Настраиваемое время debounce
- Не использует delay()
- Легкий вес в памяти

---

### 3. **button_controller** - Контроллер кнопок

**Назначение:** Объединяет чтение кнопки и debouncing.

**Использование:**
```cpp
#include "button_controller.hpp"

ButtonController btn(7);  // Pin 7

void setup() {
  btn.begin();
}

void loop() {
  unsigned long now = millis();
  
  if (btn.wasPressed(now)) {
    // Кнопка нажата!
  }
  
  if (btn.read()) {
    // Кнопка удерживается
  }
}
```

**Особенности:**
- Автоматический debouncing
- Поддержка active-low и active-high
- INPUT_PULLUP для active-low
- Простой API

---

### 4. **led_controller** - Контроллер LED

**Назначение:** Управление LED с отслеживанием состояния.

**Использование:**
```cpp
#include "led_controller.hpp"

LedController led(13);

void setup() {
  led.begin();
}

void loop() {
  led.on();
  delay(1000);
  led.off();
  delay(1000);
  
  // Или
  led.toggle();
  
  // Проверка состояния
  if (led.isOn()) {
    // LED горит
  }
}
```

**Особенности:**
- Отслеживание состояния
- Toggle функция
- Чистый API
- Легко расширяется (fade, blink, etc.)

---

### 5. **analog_sensor** - Аналоговый датчик

**Назначение:** Чтение аналоговых датчиков с масштабированием и фильтрацией.

**Использование:**
```cpp
#include "analog_sensor.hpp"

// Датчик на A0, диапазон 0-1023, фильтр 5 точек
AnalogSensor sensor(A0, 0.0f, 1023.0f, 5);

void setup() {
  sensor.begin();
}

void loop() {
  float value = sensor.readFiltered();  // С фильтрацией
  float min = sensor.getMin();
  float max = sensor.getMax();
}
```

**Особенности:**
- Автоматическое масштабирование
- Скользящее среднее (moving average)
- Отслеживание min/max
- Счётчик образцов

---

### 6. **threshold_manager** - Менеджер пороговых значений

**Назначение:** Мониторинг значений с оповещениями о превышении порогов.

**Использование:**
```cpp
#include "threshold_manager.hpp"

// Warning=750, Critical=900, Hysteresis=10
ThresholdManager thresholds(750, 900, 10.0f);

void loop() {
  float value = readSensor();
  ThresholdLevel level = thresholds.update(value);
  
  if (level == ThresholdLevel::CRITICAL) {
    printf("ALERT!\n");
  }
  
  if (thresholds.hasLevelChanged()) {
    printf("Level changed to: %s\n", 
           ThresholdManager::levelToString(level));
  }
}
```

**Особенности:**
- Три уровня: NORMAL, WARNING, CRITICAL
- Гистерезис против осцилляции
- Детекция изменений уровня
- Строковое представление

---

### 7. **status_indicator** - Индикатор статуса

**Назначение:** Визуальная индикация статуса через LED.

**Использование:**
```cpp
#include "status_indicator.hpp"

StatusIndicator led(GREEN_LED_PIN, RED_LED_PIN);

void setup() {
  led.begin();
}

void loop() {
  led.show(StatusType::SUCCESS);   // Зелёный
  led.show(StatusType::FAILURE);   // Красный
  
  // С задержкой (yield для FreeRTOS)
  led.showAndHold(StatusType::WARNING, 2000, 10);
  
  led.clear();  // Выключить все
}
```

**Особенности:**
- Готовые статусы (SUCCESS, FAILURE, WARNING, IDLE)
- Задержка с yield для FreeRTOS
- Управление двумя LED
- Чистый API

---

## 🔄 Миграция старого кода

### Было (старый подход):
```cpp
// lab2_1_tasks.cpp
bool debounceRisingEdge(bool state, bool& last, ...) {
  // 10 строк кода
}

void taskButton() {
  static bool lastState = false;
  static unsigned long lastTime = 0;
  bool pressed = ButtonCheckStatePin(PIN);
  if (debounceRisingEdge(pressed, lastState, now, lastTime)) {
    // действие
  }
}
```

### Стало (новый подход):
```cpp
// lib/debouncer - переиспользуемая библиотека
// lab2_1_tasks.cpp
#include "debouncer.hpp"

Debouncer btnDebouncer;

void taskButton(unsigned long now) {
  bool pressed = ButtonCheckStatePin(PIN);
  if (btnDebouncer.updateRisingEdge(pressed, now)) {
    // действие
  }
}
```

## 📦 Что находится в lib vs src

### ✅ В lib/ (переиспользуемое):
- Классы и функции общего назначения
- Драйверы аппаратуры (кнопки, LED, LCD)
- Утилиты (debouncing, scheduler)
- Абстракции (Button, Led)
- Все, что может быть использовано в других проектах

### ✅ В src/lab*/ (специфичное):
- Конфигурация конкретной лабораторной
- Бизнес-логика задач
- Структуры состояния
- Setup/loop функции
- Все, что уникально для этой лабораторной

## 🚀 Как использовать библиотеки в новых лабораторных

### Пример: Lab 3 с новыми библиотеками

```cpp
// src/lab3/lab3_1.cpp
#include "task_scheduler.hpp"
#include "button_controller.hpp"
#include "led_controller.hpp"

// Определить компоненты
ButtonController startBtn(5);
ButtonController stopBtn(6);
LedController statusLed(10);

// Определить задачи
void taskCheckButtons(unsigned long now) {
  if (startBtn.wasPressed(now)) {
    statusLed.on();
  }
  if (stopBtn.wasPressed(now)) {
    statusLed.off();
  }
}

void taskBlink(unsigned long now) {
  statusLed.toggle();
}

// Настроить scheduler
Task tasks[] = {
  {taskCheckButtons, 25, 0},
  {taskBlink, 500, 0},
};
TaskScheduler scheduler(tasks, 2);

void setup_lab3_1() {
  startBtn.begin();
  stopBtn.begin();
  statusLed.begin();
  scheduler.init(millis());
}

void loop_lab3_1() {
  scheduler.runOnce();
}
```

**Результат:** Минимум кода, максимум переиспользования!

## 📊 Преимущества новой архитектуры

| Аспект | Старый подход | Новый подход |
|--------|---------------|--------------|
| **Переиспользование** | Копировать код | Подключить библиотеку |
| **Читаемость** | Много helper функций | Понятные классы |
| **Тестируемость** | Сложно | Каждая библиотека отдельно |
| **Размер lab кода** | 200+ строк | 50-100 строк |
| **Зависимости** | Неявные | Явные через #include |
| **Документация** | Комментарии | Doxygen в библиотеках |

## 🔧 Переключение между лабораторными

Откройте `src/app_manager.hpp`:

```cpp
// Изменить эту строку:
constexpr LabSelection ACTIVE_LAB = LabSelection::LAB2_1;

// Доступные:
// - LabSelection::LAB1_1
// - LabSelection::LAB1_2
// - LabSelection::LAB2_1
```

## 📝 Соглашения

1. **lib/** - только переиспользуемый код
2. **src/lab*/** - минимум кода, максимум использования lib
3. Каждая библиотека в своей папке
4. Заголовки и реализация вместе
5. Документация Doxygen-style
6. Примеры в комментариях

## 🎓 Для студентов

При создании новой лабораторной:

1. **Сначала проверьте lib/** - возможно нужная библиотека уже есть
2. **Используйте готовые классы** - ButtonController, LedController, TaskScheduler
3. **Пишите минимум кода** - только логику вашей задачи
4. **Если создаете что-то переиспользуемое** - сделайте библиотеку в lib/

---

**Автор:** IOT Labs Team  
**Обновлено:** 2 ноября 2025

