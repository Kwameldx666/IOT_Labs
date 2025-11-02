# Lab 3.1 - Wokwi Setup Guide

## ✅ Исправления выполнены

1. ✅ Исправлен `lab3_1.cpp` - удалён дублированный код
2. ✅ Обновлён `diagram.json` для Wokwi симуляции
3. ✅ Код готов к работе с FreeRTOS

## 🔌 Схема подключения в Wokwi

### Компоненты:
- **Arduino Mega 2560**
- **Potentiometer** (на A0) - симулирует датчик
- **Green LED** (на Pin 10) - статус NORMAL
- **Red LED** (на Pin 12) - статус WARNING/CRITICAL
- **LCD 16x2 I2C** (опционально) - визуализация
- **2x Resistor 220Ω** - для LED

### Подключения:
```
Potentiometer:
  VCC → 5V
  GND → GND
  SIG → A0

Green LED:
  A → R1 → Pin 10
  C → GND

Red LED:
  A → R2 → Pin 12
  C → GND

LCD I2C (опционально):
  VCC → 5V
  GND → GND
  SDA → Pin 20
  SCL → Pin 21
```

## 🎮 Как запустить в Wokwi

### Шаг 1: Выбор лабораторной
В `src/app_manager.hpp` установите:
```cpp
constexpr LabSelection ACTIVE_LAB = LabSelection::LAB3_1;
```

### Шаг 2: Запуск симуляции
1. Откройте проект в Wokwi
2. Нажмите кнопку "Start Simulation" (зелёный play)
3. Откройте Serial Monitor (иконка терминала)

### Шаг 3: Управление
- **Крутите потенциометр** влево/вправо
- Наблюдайте изменение значений в Serial Monitor
- Смотрите как меняются LED индикаторы

## 📊 Пороговые значения

```
Value Range    | LED Status          | Alert Level
---------------|---------------------|-------------
0 - 699        | Green ON            | NORMAL
700 - 899      | Green ON (blink)    | WARNING  
900 - 1023     | Red ON (blink)      | CRITICAL
```

## 🖥️ Ожидаемый вывод

```
================================================
   Lab 3.1: Sensor Data Acquisition System
================================================
Sensor Pin:      A0
Read Period:     500 ms
Report Period:   500 ms
Warning Thresh:  700
Critical Thresh: 900
================================================
Using FreeRTOS for task scheduling...
Tasks: [Sensor Read] + [Status Report]
================================================

[Lab 3.1] Tasks created successfully!
[Lab 3.1] Starting FreeRTOS scheduler...


========== Lab 3.1: Sensor Status Report ==========
Time:        612 ms
---------------------------------------------------
Sensor Value:  512.00 (ADC units)
Min Value:     512.00
Max Value:     512.00
Sample Count:  1
---------------------------------------------------
Alert Level:   NORMAL
Thresholds:    WARNING=700, CRITICAL=900
===================================================
```

## 🎯 Тестовые сценарии

### Сценарий 1: Normal Operation
1. Установите потенциометр в среднее положение (~512)
2. Зелёный LED должен гореть постоянно
3. Красный LED выключен
4. Alert Level: NORMAL

### Сценарий 2: Warning Level
1. Поверните потенциометр на ~75% (значение ~770)
2. Зелёный LED начнёт мигать
3. Alert Level изменится на WARNING
4. В консоли появится "[CHANGED!]"

### Сценарий 3: Critical Level
1. Поверните потенциометр на максимум (~1000)
2. Зелёный LED погаснет
3. Красный LED начнёт мигать
4. Alert Level: CRITICAL
5. В консоли появится "*** ALERT: CRITICAL THRESHOLD EXCEEDED! ***"

## 🔧 Настройка порогов

В `include/config.h`:
```cpp
#define LAB3_THRESHOLD_WARNING 700      // Предупреждение
#define LAB3_THRESHOLD_CRITICAL 900     // Критический уровень
```

Измените эти значения для тестирования разных сценариев.

## 📚 Используемые библиотеки

- `analog_sensor` - чтение с фильтрацией (5-sample moving average)
- `threshold_manager` - управление пороговыми значениями
- `status_indicator` - управление LED индикаторами
- `Arduino_FreeRTOS` - многозадачность

## ⚙️ FreeRTOS Tasks

### Task 1: Sensor Reading
- Период: 500 мс
- Offset: 0 мс
- Приоритет: 2
- Функция: Читает датчик, проверяет пороги, обновляет LED

### Task 2: Status Reporting
- Период: 500 мс
- Offset: 100 мс
- Приоритет: 1
- Функция: Выводит отчёт в Serial Monitor

### Synchronization
- **Mutex** защищает разделяемые данные между задачами
- Предотвращает race conditions

## 🐛 Troubleshooting

### Проблема: LED не работают
- Проверьте подключения в diagram.json
- Убедитесь что резисторы 220Ω установлены
- Проверьте правильность pin номеров (10 и 12)

### Проблема: Нет вывода в Serial
- Откройте Serial Monitor в Wokwi
- Убедитесь что baud rate = 9600
- Проверьте что SerialBegin() вызывается

### Проблема: FreeRTOS не запускается
- Проверьте наличие библиотеки Arduino_FreeRTOS
- Убедитесь что стек задач достаточен (128 words)
- Проверьте что mutex создаётся успешно

## ✅ Чек-лист готовности

- [x] lab3_1.cpp исправлен
- [x] diagram.json обновлён
- [x] Компоненты добавлены в схему
- [x] Подключения настроены
- [x] FreeRTOS tasks созданы
- [x] Mutex для синхронизации
- [x] LED индикаторы работают
- [x] Serial output форматирован

---

**Статус:** ✅ **Готово к запуску в Wokwi**

Просто откройте проект и нажмите Start Simulation!

