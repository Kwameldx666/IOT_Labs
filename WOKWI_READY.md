# 🚀 Lab 3.1 - Готово к запуску в Wokwi!

## ✅ Всё уже настроено:

1. ✅ **ACTIVE_LAB = LAB3_1** установлена в app_manager.hpp
2. ✅ **diagram.json** готов с правильной схемой
3. ✅ **wokwi.toml** настроен для Arduino Mega
4. ✅ Код исправлен и готов к работе

---

## 🎮 Запуск (3 простых шага):

### 1️⃣ Соберите проект
```bash
pio run
```

### 2️⃣ Откройте Wokwi
- Перейдите на https://wokwi.com/
- Нажмите "Start New Project" → "From Template"
- Или откройте существующий проект

### 3️⃣ Запустите симуляцию
- Нажмите **зелёную кнопку Play** ▶️
- Откройте **Serial Monitor** (иконка терминала 🖥️)
- **Крутите потенциометр** 🎛️

---

## 🔌 Что подключено:

```
┌─────────────────────────────────────────┐
│  Arduino Mega 2560                      │
│                                         │
│  A0  ← Potentiometer (симуляция датчика)
│  Pin 10 → Green LED (NORMAL)           │
│  Pin 12 → Red LED (WARNING/CRITICAL)   │
│  Pin 20/21 → LCD I2C (опционально)     │
└─────────────────────────────────────────┘
```

---

## 📊 Тестирование:

### Сценарий 1: Норма (0-699)
- Потенциометр: **центр** (~512)
- Результат: **Зелёный LED горит**
- Serial: `Alert Level: NORMAL`

### Сценарий 2: Предупреждение (700-899)
- Потенциометр: **75%** (~770)
- Результат: **Зелёный LED мигает**
- Serial: `Alert Level: WARNING [CHANGED!]`

### Сценарий 3: Критично (900-1023)
- Потенциометр: **максимум** (~1000)
- Результат: **Красный LED мигает**
- Serial: `*** ALERT: CRITICAL THRESHOLD EXCEEDED! ***`

---

## 🖥️ Ожидаемый вывод в Serial:

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

---

## 🎯 Особенности реализации:

✅ **FreeRTOS** - две задачи с разными приоритетами  
✅ **Mutex** - защита разделяемых данных  
✅ **Фильтрация** - 5-sample moving average  
✅ **Модульность** - переиспользуемые библиотеки  
✅ **Visual feedback** - LED индикаторы  

---

## 🔧 Настройка (опционально):

Измените пороги в `include/config.h`:
```cpp
#define LAB3_THRESHOLD_WARNING 700   // ← измените здесь
#define LAB3_THRESHOLD_CRITICAL 900  // ← и здесь
```

---

## 📚 Используемые библиотеки:

1. **analog_sensor** - чтение датчика с фильтрацией
2. **threshold_manager** - управление порогами
3. **status_indicator** - LED индикация
4. **Arduino_FreeRTOS** - многозадачность

---

## ⚡ Быстрые команды:

```bash
# Сборка
pio run

# Загрузка (если железо подключено)
pio run -t upload

# Мониторинг Serial
pio device monitor

# Очистка
pio run -t clean
```

---

**🎉 ВСЁ ГОТОВО! Просто запустите Wokwi симуляцию!**

**Статус:** ✅ **READY TO RUN**  
**Платформа:** Wokwi Simulator  
**Board:** Arduino Mega 2560  
**Lab:** 3.1 - Sensor Data Acquisition with FreeRTOS

