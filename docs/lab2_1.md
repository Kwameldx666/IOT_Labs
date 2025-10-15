# Lab 2.1 ? Sequential Operating System Demo

## Overview
The project implements a sequential (non-preemptive) task system for Arduino Mega. Three provider tasks share a common `SharedState` structure and an IDLE consumer prints periodic reports with `printf`. Each task has its own fixed period and offset to demonstrate time-triggered scheduling.

## Task Set
| Task | Period (ms) | Offset (ms) | Responsibility | Produced fields |
|------|-------------|-------------|----------------|-----------------|
| Button+LED | 25 | 0 | Reads button on pin 7, toggles main LED on pin 12 | `mainLedOn`, `mainToggleCount`, `blinkSuppressed` |
| Blink | 40 | 10 | Drives blinking LED on pin 10 when main LED is off | `blinkLedOn`, `blinkCycleCount`, `blinkOnAccumMs` |
| State | 35 | 20 | Adjusts blink-on window via buttons on pins 6/5 | `blinkWindowUnits`, `blinkOnTargetMs`, `incAdjustCount`, `decAdjustCount` |
| IDLE | 1000 | ? | Prints aggregated state via stdout/Serial | ? |

The scheduler walks the table sequentially inside `lab2SchedulerRunOnce()`, executing each task once per iteration when its release time arrives. No task performs blocking delays.

## Provider / Consumer Flow
- Providers (Button+LED, Blink, State) update selected fields in `SharedState` with minimal processing time.
- Consumer (IDLE) reads the same structure every 1000 ms and reports the current snapshot: `printf("[Lab2.1] ...")`.
- The shared structure is single-writer per field, so no mutexes are required in this sequential context.

## Synchronisation Details
- Software debounce (`BUTTON_DEBOUNCE_MS`) prevents multiple toggles per press.
- Blink task owns the green LED; Button+LED task enforces suppression when the main LED is active, guaranteeing consistent light states.
- State task recalculates `blinkOnTargetMs` after each adjustment to keep timings coherent.

## Hardware / Wokwi Mapping
- `LED_PIN` (Mega pin 12, blue LED via 220 ? resistor) ? main light, toggled by Task 1.
- `GREEN_LED_PIN` (pin 10, green LED via 220 ? resistor) ? blink light controlled by Task 2.
- `RED_LED_PIN` (pin 4) ? kept for Lab 1.2 compatibility; unused here.
- `BUTTON_TOGGLE_PIN` (pin 7) ? toggle main LED.
- `BUTTON_INC_PIN` (pin 6) ? increase blink-on window.
- `BUTTON_DEC_PIN` (pin 5) ? decrease blink-on window.
All buttons connect to ground with the internal pull-up enabled.

## Execution Order
```
loop()
  ?? taskButtonAndLed()
  ?? taskBlinkController()
  ?? taskStateVariable()
  ?? taskIdleReport()
```

## Usage
1. Ensure `#define LAB2_1` is active in `src/main.cpp`.
2. Wire the circuit as described (matches the supplied Wokwi JSON).
3. Build and upload with PlatformIO (`pio run -t upload`).
4. Open the Serial Monitor at 9600 baud to observe IDLE reports every ~1 s.

Example output:
```
[Lab2.1] main:OFF blink:ON win:4 on:480ms off:220ms toggles:2 cycles:5 inc:1 dec:0 on_acc:960ms
```

## Verification Steps
- Start: main LED is OFF, blink LED begins flashing after its first off interval.
- Press pin 7 button: main LED turns ON, blink LED halts; press again to resume blinking.
- Press pin 6: blink ON time increases (up to 10 units, 120 ms each).
- Press pin 5: blink ON time decreases (down to 1 unit).
- Observe Serial logs updating counters and durations accordingly.

## Possible Extensions
- Mirror the IDLE summary on the LCD using the existing `own_stdio` module.
- Allow runtime reconfiguration of task periods.
- Stream JSON telemetry for external logging tools.
