#ifndef CONFIG_H
#define CONFIG_H

/**
 * @file config.h
 * @brief Global hardware and timing configuration for all laboratory exercises
 * 
 * This file contains pin assignments, timing constants, and hardware parameters
 * used across different labs. Modify these values to match your hardware setup.
 */

// ============================================================================
// Buffer and Command Configuration
// ============================================================================
#define SETUP_INIT_SIZE 64          ///< Size of command input buffer
#define COMMAND_TOKEN_BUFFER 16     ///< Token buffer size for parsing

// ============================================================================
// Serial Communication Configuration
// ============================================================================
#define SERIAL_BAUD_RATE 9600       ///< Serial communication baud rate
#define SERIAL_INIT_DELAY_MS 500    ///< Delay after serial initialization (increased for stability)

// ============================================================================
// Timing Configuration
// ============================================================================
#define KEYPAD_POLL_DELAY_MS 10     ///< Keypad polling interval
#define LAB1_RESULT_HOLD_MS 2000    ///< Display duration for Lab 1 results
#define LAB1_RESULT_TICK_MS 10      ///< Tick interval for Lab 1
#define BUTTON_DEBOUNCE_MS 35       ///< Button debounce time

// ============================================================================
// Keypad Configuration (4x4 Matrix - Arduino Mega)
// ============================================================================
#define KEYPAD_ROW0_PIN 39
#define KEYPAD_ROW1_PIN 41
#define KEYPAD_ROW2_PIN 43
#define KEYPAD_ROW3_PIN 45

#define KEYPAD_COL0_PIN 47
#define KEYPAD_COL1_PIN 49
#define KEYPAD_COL2_PIN 51
#define KEYPAD_COL3_PIN 53

#define KEYPAD_ROWS 4
#define KEYPAD_COLS 4

// ============================================================================
// Access Control Configuration
// ============================================================================
#define ACCESS_CODE "1234"
#define ACCESS_CODE_LENGTH 4
#define ACCESS_CODE_BUFFER_SIZE (ACCESS_CODE_LENGTH + 1)

// ============================================================================
// LCD Display Configuration (I2C)
// ============================================================================
#define LCD_I2C_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2

// ============================================================================
// LED Pin Assignments
// ============================================================================
#define LED_PIN 13
#define GREEN_LED_PIN 10
#define RED_LED_PIN 12

// ============================================================================
// Lab Selection - Config Only (No Menu)
// ============================================================================
// Set LAB_SELECTED_CODE to the lab you want to run at boot.
// Options: 0, 11, 12, 21, 31, 32, 41, 42, 51, 52, 61, 62
// No interactive menu - change this value and rebuild to switch labs.
#ifndef LAB_SELECTED_CODE
#define LAB_SELECTED_CODE 62
#endif

// ============================================================================
// Button Pin Assignments
// ============================================================================
#define BUTTON_PIN 11
#define BUTTON_TOGGLE_PIN 7
#define BUTTON_INC_PIN 6
#define BUTTON_DEC_PIN 5

// Lab 3.1 pins
#define LAB31_BUTTON_PIN 13
#define LAB31_LED_RED_PIN 3
#define LAB31_LED_GREEN_PIN 2
#define LAB31_PIN_TRIG 10
#define LAB31_PIN_ECHO 8
#define LAB31_MQ2_PIN A0


// ============================================================================
// Lab 2.1 Configuration - Task Scheduler
// ============================================================================
#define LAB2_TASK_BUTTON_PERIOD_MS 25
#define LAB2_TASK_BLINK_PERIOD_MS 40
#define LAB2_TASK_STATE_PERIOD_MS 35

#define LAB2_TASK_BUTTON_OFFSET_MS 0
#define LAB2_TASK_BLINK_OFFSET_MS 10
#define LAB2_TASK_STATE_OFFSET_MS 20

#define LAB2_BLINK_WINDOW_MIN 1
#define LAB2_BLINK_WINDOW_MAX 10
#define LAB2_BLINK_WINDOW_DEFAULT 4

#define LAB2_BLINK_BASE_UNIT_MS 120
#define LAB2_BLINK_OFF_DURATION_MS 220

#define LAB2_IDLE_REPORT_PERIOD_MS 1000

// ============================================================================
// Lab 3.1 Configuration - Sensor Data Acquisition with FreeRTOS
// ============================================================================
// Sensor pin
#define LAB3_SENSOR_PIN A0              ///< Analog sensor pin (or digital)

// Task timing (FreeRTOS)
#define LAB3_SENSOR_READ_PERIOD_MS 500  ///< Sensor reading period
#define LAB3_REPORT_PERIOD_MS 500       ///< Status report period
#define LAB3_SENSOR_TASK_OFFSET_MS 0    ///< Sensor task offset
#define LAB3_REPORT_TASK_OFFSET_MS 100  ///< Report task offset

// Sensor thresholds
#define LAB3_THRESHOLD_WARNING 750      ///< Warning threshold value
#define LAB3_THRESHOLD_CRITICAL 900     ///< Critical threshold value

// Task priorities (FreeRTOS)
#define LAB3_SENSOR_TASK_PRIORITY 2     ///< Sensor read task priority
#define LAB3_REPORT_TASK_PRIORITY 1     ///< Report task priority

// Stack sizes (FreeRTOS) - Use configMINIMAL_STACK_SIZE as base
#define LAB3_SENSOR_TASK_STACK (configMINIMAL_STACK_SIZE)      ///< Sensor task
#define LAB3_REPORT_TASK_STACK (configMINIMAL_STACK_SIZE + 50) ///< Report (needs more for Serial)

// ============================================================================
// Lab 3.2 Configuration - Signal Conditioning with Digital Filters
// ============================================================================
// Sensor pins
#define LAB32_SENSOR1_PIN A0            ///< Primary analog sensor pin (potentiometer)
#define LAB32_ULTRASONIC_TRIG_PIN 8     ///< HC-SR04 TRIG pin
#define LAB32_ULTRASONIC_ECHO_PIN 9     ///< HC-SR04 ECHO pin

// Task timing (FreeRTOS)
#define LAB32_SAMPLING_PERIOD_MS 100    ///< Sampling period (fast for filtering)
#define LAB32_REPORT_PERIOD_MS 500      ///< Report period
#define LAB32_SAMPLING_TASK_OFFSET_MS 0 ///< Sampling task offset
#define LAB32_REPORT_TASK_OFFSET_MS 50  ///< Report task offset

// Task priorities
#define LAB32_SAMPLING_TASK_PRIORITY 3  ///< High priority for sampling
#define LAB32_REPORT_TASK_PRIORITY 1    ///< Low priority for reporting

// Stack sizes
#define LAB32_SAMPLING_TASK_STACK 512   ///< Sampling task stack
#define LAB32_REPORT_TASK_STACK 1024    ///< Report task stack (printf needs more)

// ADC configuration
#define LAB32_ADC_RESOLUTION 1023.0f    ///< 10-bit ADC resolution
#define LAB32_ADC_REFERENCE_VOLTAGE 5.0f ///< Reference voltage (V)

// Sensor calibration (example: LM35 temperature sensor)
#define LAB32_SENSOR1_VOLTAGE_SCALE 100.0f  ///< Scale factor (e.g., 10mV/°C)
#define LAB32_SENSOR1_VOLTAGE_OFFSET 0.0f   ///< Offset voltage
#define LAB32_SENSOR1_MIN_VALUE -40.0f      ///< Min physical value
#define LAB32_SENSOR1_MAX_VALUE 125.0f      ///< Max physical value

// Filter parameters
#define LAB32_SALT_PEPPER_WINDOW 5      ///< Salt & Pepper filter window size
#define LAB32_MOVING_AVG_WINDOW 10      ///< Moving average window size
#define LAB32_WEIGHTED_AVG_WINDOW 5     ///< Weighted average window size

// LED indicator pins
#define LAB32_LED_GREEN 10              ///< Green LED (system OK)
#define LAB32_LED_YELLOW 11             ///< Yellow LED (warning threshold)
#define LAB32_LED_RED 12                ///< Red LED (alarm threshold)

// Threshold values for LED indicators (based on potentiometer ADC)
#define LAB32_THRESHOLD_LOW 300         ///< Below this: Green only
#define LAB32_THRESHOLD_HIGH 700        ///< Above this: Red alarm
// Between LOW and HIGH: Yellow warning

// ============================================================================
// Lab 4.1 Configuration - Actuators Control (Relay, Light, Motor)
// ============================================================================
// Relay pins
#define LAB41_RELAY1_PIN 7              ///< Relay 1 control pin (diagram: Mega D7)
#define LAB41_RELAY2_PIN 23             ///< Relay 2 control pin (for motor direction)

// Light bulb control (through relay)
#define LAB41_LIGHT_RELAY_PIN LAB41_RELAY1_PIN

// DC Motor control pins
#define LAB41_MOTOR_ENABLE_PIN 9        ///< PWM pin for motor speed (enable)
#define LAB41_MOTOR_DIR_PIN1 24         ///< Motor direction pin 1
#define LAB41_MOTOR_DIR_PIN2 25         ///< Motor direction pin 2

// PWM configuration for motor
#define LAB41_MOTOR_PWM_MIN 0           ///< Minimum PWM value (stopped)
#define LAB41_MOTOR_PWM_MAX 255         ///< Maximum PWM value (full speed)
#define LAB41_MOTOR_DEFAULT_SPEED 128   ///< Default motor speed (50%)

// Command buffer
#define LAB41_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer size

// LCD update period
#define LAB41_LCD_UPDATE_PERIOD_MS 500  ///< LCD refresh period

#define LAB21_BUTTON_PIN 6
#define LAB21_LED_RED_PIN 4
#define LAB21_LED_GREEN_PIN 5

// ============================================================================
// Lab 4.2 Configuration - DC Motor Control (-100% to +100%)
// ============================================================================
// DC Motor pins (same as Lab 4.1 or different)
#define LAB42_MOTOR_ENABLE_PIN 6       ///< PWM pin for motor speed
#define LAB42_MOTOR_PIN1 5             ///< Motor direction pin 1
#define LAB42_MOTOR_PIN2 4             ///< Motor direction pin 2

// Power range
#define LAB42_MOTOR_POWER_MIN -100      ///< Min power (full reverse)
#define LAB42_MOTOR_POWER_MAX 100       ///< Max power (full forward)
#define LAB42_MOTOR_POWER_STOP 0        ///< Stop position

// LCD update
#define LAB42_LCD_UPDATE_PERIOD_MS 300  ///< LCD refresh period
#define LAB42_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer size

// ============================================================================
// Lab 5.1 Configuration - Temperature Safety Controller
// ============================================================================
#define LAB51_TMP36_PIN A0              ///< Analog pin for TMP36 sensor
#define LAB51_MOTOR_RELAY_PIN 13        ///< Relay to drive the fan/motor
#define LAB51_ALARM_RELAY_PIN 7         ///< Relay for alarm lamp/siren
#define LAB51_BUTTON_UP_PIN 3           ///< Increment setpoint button
#define LAB51_BUTTON_DOWN_PIN 4         ///< Decrement setpoint button
#define LAB51_LED_RED_PIN 10            ///< Red LED output
#define LAB51_LED_GREEN_PIN 11          ///< Green LED output
#define LAB51_LED_BLUE_PIN 12           ///< Blue LED output

#define LAB51_START_SETPOINT_C 25.0f    ///< Default temperature setpoint
#define LAB51_STEP_SETPOINT_C 0.5f      ///< Button step size in Celsius
#define LAB51_NORMAL_RANGE_C 2.0f       ///< +/- normal zone around setpoint
#define LAB51_EXTREME_DELTA_C 10.0f     ///< Alarm threshold above setpoint
#define LAB51_DEBOUNCE_MS 80            ///< Button debounce interval
#define LAB51_LCD_REFRESH_PERIOD_MS 200 ///< Minimum LCD refresh interval
#define LAB51_TELEMETRY_PERIOD_MS 1000  ///< StdIO log period

// ============================================================================
// Lab 5.2 Configuration - PID Temperature Control (PWM)
// ============================================================================
#define LAB52_TMP36_PIN A0                ///< Analog pin for TMP36 sensor
#define LAB52_PWM_PIN 9                   ///< PWM output pin driving nMOS/SSR
#define LAB52_BUTTON_UP_PIN 3             ///< Increment setpoint button
#define LAB52_BUTTON_DOWN_PIN 4           ///< Decrement setpoint button
#define LAB52_LED_RED_PIN 10              ///< Red LED output
#define LAB52_LED_GREEN_PIN 11            ///< Green LED output
#define LAB52_LED_BLUE_PIN 12             ///< Blue LED output

#define LAB52_START_SETPOINT_C 25.0f      ///< Default target temperature
#define LAB52_STEP_SETPOINT_C 0.5f        ///< Button step size in Celsius
#define LAB52_LED_BAND_C 1.0f             ///< +/- zone for LED color changes
#define LAB52_BTN_DEBOUNCE_MS 100         ///< Button debounce interval
#define LAB52_LCD_REFRESH_PERIOD_MS 200   ///< LCD refresh interval
#define LAB52_TELEMETRY_PERIOD_MS 200     ///< StdIO/Plotter log period
#define LAB52_LOOP_DELAY_MS 20            ///< Main loop delay to limit CPU load

#define LAB52_PID_KP 10.0                 ///< Proportional gain
#define LAB52_PID_KI 2.0                  ///< Integral gain
#define LAB52_PID_KD 1.0                  ///< Derivative gain
#define LAB52_PID_SAMPLE_MS 100           ///< PID compute interval
#define LAB52_PID_OUTPUT_MIN 0            ///< Minimum PWM value
#define LAB52_PID_OUTPUT_MAX 255          ///< Maximum PWM value

// ============================================================================
// Lab 6.1 Configuration - FSM LED Toggle with Button
// ============================================================================
#define LAB61_LED_PIN 13                  ///< LED output pin
#define LAB61_BUTTON_PIN 2                ///< Momentary button pin (active LOW)
#define LAB61_STATE_HOLD_MS 100           ///< State dwell time (ms)
#define LAB61_DEBOUNCE_MS 60              ///< Button debounce interval (ms)

// ============================================================================
// Lab 6.2 Configuration - Traffic Light FSM (North/East)
// ============================================================================
#define LAB62_NORTH_PIN 20                ///< North direction button/input (Wokwi: SDA pin)
#define LAB62_EAST_PIN 21                 ///< East direction button/input (Wokwi: SCL pin)

#define LAB62_EAST_RED_PIN 5
#define LAB62_EAST_YELLOW_PIN 6
#define LAB62_EAST_GREEN_PIN 7
#define LAB62_NORTH_RED_PIN 8
#define LAB62_NORTH_YELLOW_PIN 9
#define LAB62_NORTH_GREEN_PIN 10

#define LAB62_STATE_GO_N 0
#define LAB62_STATE_WAIT_N 1
#define LAB62_STATE_GO_E 2
#define LAB62_STATE_WAIT_E 3

#define LAB62_GO_N_OUTPUT 0b100001        ///< ER EY EG NR NY NG bits
#define LAB62_WAIT_N_OUTPUT 0b010001
#define LAB62_GO_E_OUTPUT 0b001100
#define LAB62_WAIT_E_OUTPUT 0b001010

#define LAB62_GO_N_TIME_MS 3000
#define LAB62_WAIT_N_TIME_MS 500
#define LAB62_GO_E_TIME_MS 3000
#define LAB62_WAIT_E_TIME_MS 500

#endif
