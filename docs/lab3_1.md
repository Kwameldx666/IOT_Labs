# Lab 3.1: Sensor Data Acquisition with FreeRTOS

## 📋 Обзор

Модульное приложение для сбора данных с аналогового датчика с использованием FreeRTOS для управления задачами.

## 🎯 Цели

- Периодический сбор данных с датчика (500 мс)
- Мониторинг пороговых значений с индикацией
- Отображение структурированных отчётов через STDIO
- Использование FreeRTOS для планирования задач

## 🏗️ Архитектура

### Переиспользуемые библиотеки

#### 1. **AnalogSensor** (`lib/analog_sensor/`)
- Чтение аналоговых датчиков
- Масштабирование в физические единицы
- Скользящее среднее (фильтрация)
- Отслеживание min/max значений

```cpp
AnalogSensor sensor(A0, 0.0f, 1023.0f, 5); // 5-точечный фильтр
sensor.begin();
float value = sensor.readFiltered();
```

#### 2. **ThresholdManager** (`lib/threshold_manager/`)
- Мониторинг пороговых значений
- Уровни: NORMAL, WARNING, CRITICAL
- Гистерезис для предотвращения осцилляции

```cpp
ThresholdManager thresholds(750, 900, 10.0f);
ThresholdLevel level = thresholds.update(value);
```

#### 3. **StatusIndicator** (`lib/status_indicator/`)
- Визуальная индикация через LED
- Поддержка уровней статуса
- Задержка с yield для FreeRTOS

```cpp
StatusIndicator led(GREEN_LED_PIN, RED_LED_PIN);
led.show(StatusType::WARNING);
```

### Структура Lab 3.1

```
src/lab3/
├── lab3_1.hpp    - Интерфейс (setup/loop)
└── lab3_1.cpp    - Полная реализация (один файл)
    ├── Lab3State           - Разделяемое состояние
    ├── taskSensorRead      - FreeRTOS Task 1 (чтение датчика)
    ├── taskStatusReport    - FreeRTOS Task 2 (отчёты)
    └── setup_lab3_1        - Инициализация и запуск
```

## ⚙️ Конфигурация

Все константы в `include/config.h`:

```cpp
// Датчик
#define LAB3_SENSOR_PIN A0

// Периоды задач
#define LAB3_SENSOR_READ_PERIOD_MS 500
#define LAB3_REPORT_PERIOD_MS 500

// Смещения задач
#define LAB3_SENSOR_TASK_OFFSET_MS 0
#define LAB3_REPORT_TASK_OFFSET_MS 100

// Пороги
#define LAB3_THRESHOLD_WARNING 750
#define LAB3_THRESHOLD_CRITICAL 900

// Приоритеты FreeRTOS
#define LAB3_SENSOR_TASK_PRIORITY 2
#define LAB3_REPORT_TASK_PRIORITY 1
```

## 🔄 FreeRTOS Задачи

### Task 1: Sensor Reading
- **Период:** 500 мс
- **Смещение:** 0 мс
- **Приоритет:** 2 (высокий)
- **Функции:**
  - Чтение датчика с фильтрацией
  - Проверка пороговых значений
  - Обновление разделяемого состояния (mutex)
  - Управление визуальной индикацией

```cpp
vTaskDelayUntil(&xLastWakeTime, xPeriod); // Периодичность
```

### Task 2: Status Report
- **Период:** 500 мс
- **Смещение:** 100 мс
- **Приоритет:** 1 (низкий)
- **Функции:**
  - Чтение разделяемого состояния (mutex)
  - Форматированный вывод через printf
  - Отчёты о состоянии системы
  - Алерты при превышении порогов

```cpp
vTaskDelayUntil(&xLastWakeTime, xPeriod); // Периодичность
```

## 🔐 Синхронизация

**Mutex** для защиты разделяемого состояния:

```cpp
// Запись (Task 1)
if (xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
  gState.currentValue = value;
  // ...
  xSemaphoreGive(gStateMutex);
}

// Чтение (Task 2)
if (xSemaphoreTake(gStateMutex, portMAX_DELAY) == pdTRUE) {
  localState = gState;
  xSemaphoreGive(gStateMutex);
}
```

## 📊 Пример вывода

```
========== Lab 3.1: Sensor Status Report ==========
Time:        12500 ms
---------------------------------------------------
Sensor Value:  456.23 (ADC units)
Min Value:     120.50
Max Value:     890.75
Sample Count:  25
---------------------------------------------------
Alert Level:   NORMAL
Thresholds:    WARNING=750, CRITICAL=900
===================================================
```

При превышении критического порога:
```
*** ALERT: CRITICAL THRESHOLD EXCEEDED! ***
*** Action Required: Check sensor immediately! ***
```

## 🎨 Визуальная индикация

| Уровень | LED статус |
|---------|------------|
| NORMAL | Все LED выключены |
| WARNING | Желтый/мигающий |
| CRITICAL | Красный постоянно |

## 📈 Диаграмма времени

```
Time:    0ms    100ms   500ms   600ms   1000ms  1100ms
         |       |       |       |       |       |
Task1:   [Read]          [Read]          [Read]
Task2:           [Report]        [Report]        [Report]
```

**Смещение 100 мс** между задачами предотвращает конфликты.

## 🧪 Тестирование

1. **Подключите потенциометр** к пину A0
2. **Загрузите код** через PlatformIO
3. **Откройте Serial Monitor** (9600 baud)
4. **Вращайте потенциометр** и наблюдайте:
   - Изменение значений в реальном времени
   - Срабатывание порогов
   - LED индикацию
   - Структурированные отчёты каждые 500 мс

## 🔧 Модификации

### Изменить датчик
1. Подключите другой аналоговый датчик к A0
2. Обновите масштабирование в конструкторе:
```cpp
AnalogSensor sensor(LAB3_SENSOR_PIN, minPhysical, maxPhysical, filterSize);
```

### Изменить периоды
Отредактируйте `config.h`:
```cpp
#define LAB3_SENSOR_READ_PERIOD_MS 250  // Быстрее
#define LAB3_REPORT_PERIOD_MS 1000      // Реже отчёты
```

### Добавить третью задачу
```cpp
void taskCustom(void* pvParameters) {
  while (true) {
    // Ваш код
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

// В setup:
xTaskCreate(taskCustom, "Custom", 256, nullptr, 1, nullptr);
```

## 📦 Зависимости

- `Arduino_FreeRTOS.h` - Планировщик задач
- `analog_sensor` - Чтение датчика
- `threshold_manager` - Мониторинг порогов
- `status_indicator` - Визуальная обратная связь

## ✅ Соответствие требованиям

| Требование | Реализация |
|------------|------------|
| ✅ Съём сигнала | AnalogSensor library |
| ✅ Периодичность | vTaskDelayUntil() |
| ✅ Смещения | vTaskDelay() в начале |
| ✅ STDIO отчёты | printf() каждые 500 мс |
| ✅ Модульность | Библиотеки в lib/ |
| ✅ FreeRTOS | 2 задачи с mutex |
| ✅ Импровизация | Алерт при критическом уровне |

## 🎓 Оценивание

- **50%** - Работа сбора и отображения ✅
- **10%** - STDIO отчёты ✅
- **10%** - Объяснение hardware-software ✅
- **10%** - Схемы (см. diagram.json) ✅
- **10%** - Физическое функционирование ✅
- **10%** - Дополнительное поведение (алерты) ✅

---

**Автор:** IOT Labs Team  
**Дата:** 2 ноября 2025  
**Версия:** 1.0

