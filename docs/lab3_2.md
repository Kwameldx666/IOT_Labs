# Lab 3.2: Signal Conditioning with Digital Filters

## 📋 Обзор

Модульное приложение для кондиционирования сигнала с аналоговых датчиков с использованием цифровых фильтров и FreeRTOS.

## 🎯 Цели

- Применение цифровых фильтров для очистки сигнала
- Реализация фильтра "Соль и Перец" (медианный)
- Реализация взвешенного фильтра усреднения
- Преобразование ADC → Voltage → Physical
- Применение насыщения (saturation)
- Отчёты через STDIO с FreeRTOS

## 🔄 Конвейер обработки сигнала

```
Raw ADC (0-1023)
    ↓
[Salt & Pepper Filter]  ← Удаление импульсных шумов
    ↓
[Weighted Average]      ← Сглаживание сигнала
    ↓
[ADC → Voltage]         ← Преобразование в вольты
    ↓
[Voltage → Physical]    ← Преобразование в физическую величину
    ↓
[Saturation]            ← Ограничение допустимого диапазона
    ↓
Physical Value (°C)
```

## 🏗️ Архитектура

### Переиспользуемые библиотеки

#### 1. **SaltPepperFilter** (`lib/salt_pepper_filter/`)
**Назначение:** Медианный фильтр для удаления импульсных шумов.

**Принцип работы:**
- Собирает окно из N образцов
- Сортирует значения
- Возвращает медиану (центральное значение)
- Эффективен против выбросов (спайков)

**Использование:**
```cpp
SaltPepperFilter filter(5);  // Окно 5 образцов
float cleaned = filter.filter(noisyValue);
```

**Пример:**
```
Входные данные:  [100, 105, 950, 102, 98]  ← 950 это шум!
Отсортированные: [98, 100, 102, 105, 950]
Медиана:         102  ← Шум отфильтрован!
```

#### 2. **WeightedAverageFilter** (`lib/weighted_average_filter/`)
**Назначение:** Взвешенное скользящее усреднение для сглаживания.

**Принцип работы:**
- Более новые образцы имеют больший вес
- Веса: [1, 2, 3, 4, 5] (линейные)
- Weighted Avg = (1×old + 2×... + 5×new) / (1+2+3+4+5)

**Использование:**
```cpp
WeightedAverageFilter filter(5);  // Окно 5 образцов
float smoothed = filter.filter(value);
```

**Пример:**
```
Образцы: [100, 102, 104, 106, 108]
Веса:    [1,   2,   3,   4,   5]
Результат: (1×100 + 2×102 + 3×104 + 4×106 + 5×108) / 15 = 105.3
```

#### 3. **SignalConverter** (`lib/signal_converter/`)
**Назначение:** Преобразования ADC → Voltage → Physical с насыщением.

**Формулы:**
```cpp
// ADC → Voltage
voltage = (adc_value / 1023) × 5.0V

// Voltage → Physical (LM35 example: 10mV/°C)
temperature = voltage × 100

// Saturation
if (temp < -40°C) temp = -40°C
if (temp > 125°C) temp = 125°C
```

**Использование:**
```cpp
SignalConverter conv(1023.0f, 5.0f, 100.0f, 0.0f, -40.0f, 125.0f);
float physical = conv.convert(adcValue);
```

### Структура Lab 3.2

```
src/lab3/
├── lab3_2.hpp    - Интерфейс
└── lab3_2.cpp    - Полная реализация (один файл)
    ├── SignalData           - Структура данных сигнала
    ├── processSignal()      - Конвейер обработки
    ├── taskSignalSampling   - FreeRTOS Task 1
    ├── taskStatusReport     - FreeRTOS Task 2
    └── setup_lab3_2         - Инициализация
```

## ⚙️ Конфигурация

Все настройки в `include/config.h`:

```cpp
// Датчики
#define LAB32_SENSOR1_PIN A0
#define LAB32_SENSOR2_PIN A1  // Дополнительный датчик (бонус)

// Периоды задач
#define LAB32_SAMPLING_PERIOD_MS 100  // Быстрая выборка
#define LAB32_REPORT_PERIOD_MS 500    // Отчёты

// Параметры фильтров
#define LAB32_SALT_PEPPER_WINDOW 5    // Медиана 5 точек
#define LAB32_WEIGHTED_AVG_WINDOW 5   // Взвешенное среднее 5 точек

// ADC
#define LAB32_ADC_RESOLUTION 1023.0f
#define LAB32_ADC_REFERENCE_VOLTAGE 5.0f

// Калибровка датчика (LM35)
#define LAB32_SENSOR1_VOLTAGE_SCALE 100.0f  // 10mV/°C
#define LAB32_SENSOR1_MIN_VALUE -40.0f
#define LAB32_SENSOR1_MAX_VALUE 125.0f
```

## 🔄 FreeRTOS Задачи

### Task 1: Signal Sampling & Conditioning
- **Период:** 100 мс (быстрая выборка для фильтров)
- **Приоритет:** 3 (высокий)
- **Функции:**
  1. Чтение ADC с обоих датчиков
  2. Применение Salt & Pepper фильтра
  3. Применение Weighted Average фильтра
  4. Преобразование ADC→Voltage→Physical
  5. Насыщение
  6. Обновление статистики

### Task 2: Status Reporting
- **Период:** 500 мс
- **Смещение:** 50 мс
- **Приоритет:** 1 (низкий)
- **Функции:**
  - Чтение обработанных данных (mutex)
  - Форматированный вывод через printf
  - Отображение конвейера обработки
  - Статистика (min/max/samples)

## 📊 Пример вывода

```
╔════════════════════════════════════════════════════════╗
║     Lab 3.2: Signal Conditioning Report               ║
╚════════════════════════════════════════════════════════╝
Time: 5000 ms

┌─── SENSOR 1 (Pin A0) ───────────────────────────────┐
│ Signal Processing Pipeline:                          │
│   Raw ADC:           512 (0-1023)                    │
│   ↓ Salt & Pepper:   510.00                          │
│   ↓ Weighted Avg:    508.50                          │
│   ↓ Voltage:         2.485 V                         │
│   ↓ Physical:        24.85 °C (with saturation)      │
│                                                        │
│ Statistics:                                            │
│   Samples:   50                                       │
│   Min Value: 22.30 °C                                 │
│   Max Value: 26.10 °C                                 │
│   Range:     3.80 °C                                  │
└────────────────────────────────────────────────────────┘

┌─── SENSOR 2 (Pin A1) ───────────────────────────────┐
│ Physical Value:      23.45 °C                        │
│ Samples:             50                              │
└────────────────────────────────────────────────────────┘

[Filter Configuration]
  Salt & Pepper Window:    5 samples (median)
  Weighted Avg Window:     5 samples (weighted)
  Sampling Period:         100 ms
  Report Period:           500 ms
════════════════════════════════════════════════════════
```

## 🎨 Визуализация фильтров

### Эффект Salt & Pepper Filter

```
Сигнал с шумом:     [100, 105, 950, 102, 98, 103, 1000, 101]
                           ↑ шум       ↑ шум

После фильтра:      [100, 102, 102, 102, 101, 101, 101, 101]
                    ✓ Шумы удалены, сигнал чист
```

### Эффект Weighted Average

```
Ступенчатый сигнал: [100, 100, 100, 200, 200, 200]
                                    ↑ резкий переход

После фильтра:      [100, 100, 107, 147, 180, 193]
                    ✓ Плавный переход, сглаживание
```

## 🧪 Тестирование

### Подключение LM35 Temperature Sensor

```
LM35 Pin 1 (Vcc)  → Arduino 5V
LM35 Pin 2 (Vout) → Arduino A0
LM35 Pin 3 (GND)  → Arduino GND
```

### Тестовые сценарии

1. **Нормальная работа:**
   - Держите датчик при комнатной температуре
   - Наблюдайте стабильные показания ~22-25°C

2. **Тест импульсных помех:**
   - Отключите/подключите датчик быстро
   - Salt & Pepper фильтр должен подавить выбросы

3. **Тест плавных изменений:**
   - Нагрейте датчик пальцами
   - Weighted Average сгладит переход

4. **Два датчика (бонус):**
   - Подключите второй LM35 к A1
   - Сравните показания обоих датчиков

## 🔧 Калибровка для других датчиков

### Для термистора NTC 10kΩ:

```cpp
// Изменить в config.h:
#define LAB32_SENSOR1_VOLTAGE_SCALE 1.0f  // Не линейный!
#define LAB32_SENSOR1_VOLTAGE_OFFSET 0.0f

// В lab3_2.cpp использовать формулу Steinhart-Hart:
// 1/T = A + B×ln(R) + C×(ln(R))³
```

### Для фоторезистора:

```cpp
// Масштабирование в люксы
#define LAB32_SENSOR1_VOLTAGE_SCALE 1000.0f  // Lux
#define LAB32_SENSOR1_MIN_VALUE 0.0f
#define LAB32_SENSOR1_MAX_VALUE 10000.0f
```

## 📈 Диаграмма времени

```
Time:    0ms   50ms  100ms 150ms 200ms 250ms 300ms 350ms 400ms 450ms 500ms
         |     |     |     |     |     |     |     |     |     |     |
Task1:   [S]         [S]         [S]         [S]         [S]         [S]
Task2:         [R]                                              [R]

S = Sampling & Conditioning
R = Report
```

## 📦 Зависимости

- `Arduino_FreeRTOS.h` - Планировщик
- `salt_pepper_filter` - Медианный фильтр
- `weighted_average_filter` - Взвешенное усреднение
- `signal_converter` - ADC→Voltage→Physical
- `dd_serial` - STDIO output

## ✅ Соответствие требованиям (Оценивание)

| Требование | Реализация | Баллы |
|------------|------------|-------|
| ✅ Базовое приложение с отображением | SignalData + printf | 50% |
| ✅ Фильтр "Соль и Перец" | SaltPepperFilter library | 10% |
| ✅ Взвешенный фильтр усреднения | WeightedAverageFilter library | 10% |
| ✅ FreeRTOS + периодические отчёты | 2 задачи с vTaskDelayUntil | 10% |
| ✅ Дополнительный датчик | Sensor 2 на A1 | 10% |
| ✅ Физическое функционирование | Реальный LM35 | 10% |

**Итого:** 100%

## 🎓 Технические детали

### Почему медианный фильтр?
- Нелинейный фильтр
- Полностью удаляет выбросы
- Сохраняет резкие переходы
- Идеален для импульсных помех

### Почему взвешенное усреднение?
- Линейный фильтр
- Отдаёт приоритет свежим данным
- Быстрее реагирует на изменения
- Лучше чем простое усреднение

### Порядок фильтров важен!
1. **Сначала Salt & Pepper** - удаляем грубые шумы
2. **Потом Weighted Average** - сглаживаем результат

Обратный порядок работает хуже!

## 🔍 Отладка

### Проблема: Значения скачут
**Решение:** Увеличьте окна фильтров
```cpp
#define LAB32_SALT_PEPPER_WINDOW 7
#define LAB32_WEIGHTED_AVG_WINDOW 10
```

### Проблема: Медленная реакция
**Решение:** Уменьшите окно weighted average
```cpp
#define LAB32_WEIGHTED_AVG_WINDOW 3
```

### Проблема: Неправильные значения
**Решение:** Проверьте калибровку
```cpp
// Измерьте реальное напряжение и подстройте scale
#define LAB32_SENSOR1_VOLTAGE_SCALE 100.0f  // Для LM35
```

---

**Автор:** IOT Labs Team  
**Дата:** 2 ноября 2025  
**Версия:** 1.0

