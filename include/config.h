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
// Options: 0, 11, 12, 21, 31, 32, 41, 42, 51, 52, 61, 62, 71, 72, 73, 74, 75
// No interactive menu - change this value and rebuild to switch labs.
#ifndef LAB_SELECTED_CODE
#define LAB_SELECTED_CODE 32
#endif

// ============================================================================
// Button Pin Assignments
// ============================================================================
#define BUTTON_PIN 11
#define BUTTON_TOGGLE_PIN 7
#define BUTTON_INC_PIN 6
#define BUTTON_DEC_PIN 5

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
#define LAB41_RELAY1_PIN 22             ///< Relay 1 control pin
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

// ============================================================================
// Lab 4.2 Configuration - DC Motor Control (-100% to +100%)
// ============================================================================
// DC Motor pins (same as Lab 4.1 or different)
#define LAB42_MOTOR_ENABLE_PIN 9        ///< PWM pin for motor speed
#define LAB42_MOTOR_DIR_PIN1 24         ///< Motor direction pin 1
#define LAB42_MOTOR_DIR_PIN2 25         ///< Motor direction pin 2

// Power range
#define LAB42_MOTOR_POWER_MIN -100      ///< Min power (full reverse)
#define LAB42_MOTOR_POWER_MAX 100       ///< Max power (full forward)
#define LAB42_MOTOR_POWER_STOP 0        ///< Stop position

// LCD update
#define LAB42_LCD_UPDATE_PERIOD_MS 300  ///< LCD refresh period

// Command buffer
#define LAB42_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer size

// ============================================================================
// Lab 5.1 Configuration - ON-OFF Control with Hysteresis
// ============================================================================
// Temperature/Humidity sensor
#define LAB51_SENSOR_PIN A0             ///< Sensor analog pin
#define LAB51_SENSOR_MIN 0.0f           ///< Min sensor value
#define LAB51_SENSOR_MAX 100.0f         ///< Max sensor value

// Control relay
#define LAB51_RELAY_PIN 22              ///< Relay control pin

// ON-OFF Control parameters
#define LAB51_SETPOINT_DEFAULT 25.0f    ///< Default setpoint (°C or %RH)
#define LAB51_SETPOINT_MIN 0.0f         ///< Min setpoint
#define LAB51_SETPOINT_MAX 50.0f        ///< Max setpoint
#define LAB51_HYSTERESIS_DEFAULT 2.0f   ///< Default hysteresis (±2°C)
#define LAB51_HYSTERESIS_MIN 0.5f       ///< Min hysteresis
#define LAB51_HYSTERESIS_MAX 10.0f      ///< Max hysteresis

// Setpoint adjustment
#define LAB51_SETPOINT_STEP 0.5f        ///< Step for UP/DOWN buttons
#define LAB51_BUTTON_UP_PIN 6           ///< UP button pin
#define LAB51_BUTTON_DOWN_PIN 5         ///< DOWN button pin

// Update periods
#define LAB51_SENSOR_READ_PERIOD_MS 500 ///< Sensor reading period
#define LAB51_LCD_UPDATE_PERIOD_MS 500  ///< LCD update period
#define LAB51_PLOTTER_PERIOD_MS 500     ///< Serial plotter update

// Command buffer
#define LAB51_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer

// ============================================================================
// Lab 5.2 Configuration - PID Motor Speed Control with Encoder
// ============================================================================
// Motor control (reuse from Lab 4.2)
#define LAB52_MOTOR_ENABLE_PIN 9        ///< PWM pin for motor speed
#define LAB52_MOTOR_DIR_PIN1 24         ///< Motor direction pin 1
#define LAB52_MOTOR_DIR_PIN2 25         ///< Motor direction pin 2

// Encoder pins
#define LAB52_ENCODER_PIN_A 2           ///< Encoder channel A (interrupt)
#define LAB52_ENCODER_PIN_B 3           ///< Encoder channel B (interrupt)
#define LAB52_ENCODER_PPR 20            ///< Pulses per revolution

// Control parameters
#define LAB52_SETPOINT_DEFAULT 100.0f   ///< Default setpoint (RPM)
#define LAB52_SETPOINT_MIN 0.0f         ///< Min RPM
#define LAB52_SETPOINT_MAX 300.0f       ///< Max RPM
#define LAB52_SETPOINT_STEP 10.0f       ///< Step for UP/DOWN

// PID parameters (initial tuning)
#define LAB52_PID_KP_DEFAULT 2.0f       ///< Proportional gain
#define LAB52_PID_KI_DEFAULT 0.5f       ///< Integral gain
#define LAB52_PID_KD_DEFAULT 0.1f       ///< Derivative gain
#define LAB52_PID_OUTPUT_MIN 0.0f       ///< Min PID output
#define LAB52_PID_OUTPUT_MAX 255.0f     ///< Max PID output (PWM)

// ON-OFF parameters (for comparison)
#define LAB52_ONOFF_HYSTERESIS 10.0f    ///< Hysteresis for ON-OFF (±10 RPM)

// Update periods
#define LAB52_CONTROL_PERIOD_MS 50      ///< Control loop period (20 Hz)
#define LAB52_LCD_UPDATE_PERIOD_MS 500  ///< LCD update period
#define LAB52_PLOTTER_PERIOD_MS 100     ///< Serial plotter period

// Buttons
#define LAB52_BUTTON_UP_PIN 6           ///< UP button
#define LAB52_BUTTON_DOWN_PIN 5         ///< DOWN button
#define LAB52_BUTTON_MODE_PIN 4         ///< Mode switch button

// Command buffer
#define LAB52_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer

// ============================================================================
// Lab 6.1 Configuration - Finite State Machine (Button-LED)
// ============================================================================
// Hardware pins
#define LAB61_BUTTON_PIN 6              ///< Input button pin
#define LAB61_LED_RED_PIN 10            ///< Red LED pin
#define LAB61_LED_GREEN_PIN 11          ///< Green LED pin
#define LAB61_LED_BLUE_PIN 12           ///< Blue LED pin

// FSM timing
#define LAB61_DEBOUNCE_MS 50            ///< Button debounce time
#define LAB61_REPORT_PERIOD_MS 1000     ///< State report period
#define LAB61_BLINK_PERIOD_MS 500       ///< LED blink period

// Command buffer
#define LAB61_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer

// ============================================================================
// Lab 6.2 Configuration - Finite State Machine (Traffic Light)
// ============================================================================
// Traffic light LED pins
#define LAB62_LED_RED_PIN 10            ///< Red light pin
#define LAB62_LED_YELLOW_PIN 11         ///< Yellow light pin
#define LAB62_LED_GREEN_PIN 12          ///< Green light pin

// Timing for each state (in milliseconds)
#define LAB62_GREEN_DURATION_MS 5000    ///< Green light duration
#define LAB62_YELLOW_DURATION_MS 2000   ///< Yellow light duration
#define LAB62_RED_DURATION_MS 5000      ///< Red light duration

// Control
#define LAB62_BUTTON_PIN 6              ///< Button for manual control
#define LAB62_REPORT_PERIOD_MS 1000     ///< State report period

// Command buffer
#define LAB62_COMMAND_BUFFER_SIZE 64    ///< Serial command buffer

// ============================================================================
// Lab 7.1 Configuration - I²C Communication (Two MCUs)
// ============================================================================
// I²C Configuration
#define LAB71_I2C_ADDRESS 0x08          ///< I²C slave address for MCU1
#define LAB71_I2C_CLOCK 100000          ///< I²C clock speed (100 kHz)

// HC-SR04 Ultrasonic Sensor (connected to MCU1)
#define LAB71_ULTRASONIC_TRIG_PIN 7     ///< Trigger pin
#define LAB71_ULTRASONIC_ECHO_PIN 8     ///< Echo pin
#define LAB71_ULTRASONIC_MAX_DISTANCE 400  ///< Max distance in cm
#define LAB71_ULTRASONIC_TIMEOUT_US 30000  ///< Timeout in microseconds

// Data packet format
#define LAB71_PACKET_HEADER 0xAA        ///< Packet start marker
#define LAB71_PACKET_FOOTER 0x55        ///< Packet end marker
#define LAB71_PACKET_SIZE 8             ///< Total packet size in bytes

// MCU2 (Master) timing
#define LAB71_POLL_PERIOD_MS 500        ///< How often to request data
#define LAB71_MEASUREMENT_INTERVAL_MS 100  ///< MCU1 measurement interval

// Report periods
#define LAB71_REPORT_PERIOD_MS 1000     ///< Status report period

// ============================================================================
// Lab 7.2 Configuration - Serial Protocol Communication
// ============================================================================
// Serial configuration
#define LAB72_SERIAL_BAUD 9600          ///< Serial baud rate
#define LAB72_PACKET_START 0x7E         ///< Start marker '~'
#define LAB72_PACKET_END 0x7F           ///< End marker (DEL)
#define LAB72_MAX_PAYLOAD_SIZE 32       ///< Max payload bytes

// Device IDs
#define LAB72_MCU1_ID 0x01              ///< MCU1 device ID
#define LAB72_MCU2_ID 0x02              ///< MCU2 device ID

// Packet types
#define LAB72_CMD_REQUEST_DATA 0x10     ///< Request sensor data
#define LAB72_CMD_RESPONSE_DATA 0x11    ///< Response with data
#define LAB72_CMD_SET_LED 0x20          ///< Set LED state command
#define LAB72_CMD_ACK 0xF0              ///< Acknowledgment
#define LAB72_CMD_NACK 0xFF             ///< Negative acknowledgment

// Timing
#define LAB72_AUTO_SEND_PERIOD_MS 2000  ///< Auto-send period (MCU1)
#define LAB72_RESPONSE_TIMEOUT_MS 1000  ///< Response timeout

// HC-SR04 for Lab 7.2 (can reuse Lab 7.1 config or define new)
#define LAB72_ULTRASONIC_TRIG_PIN 7
#define LAB72_ULTRASONIC_ECHO_PIN 8

// ============================================================================
// Lab 7.3 Configuration - MQTT Internet Communication (ESP32)
// ============================================================================
// WiFi Configuration
#define LAB73_WIFI_SSID "Your_WiFi_SSID"        ///< WiFi network name
#define LAB73_WIFI_PASSWORD "Your_WiFi_Password" ///< WiFi password
#define LAB73_WIFI_TIMEOUT_MS 10000             ///< WiFi connection timeout

// MQTT Configuration (ThingsBoard or HiveMQ)
#define LAB73_MQTT_BROKER "demo.thingsboard.io" ///< MQTT broker address
#define LAB73_MQTT_PORT 1883                    ///< MQTT broker port
#define LAB73_MQTT_CLIENT_ID "ESP32_IOT_Client" ///< MQTT client ID
#define LAB73_MQTT_USER "YOUR_ACCESS_TOKEN"     ///< ThingsBoard access token
#define LAB73_MQTT_PASSWORD ""                  ///< MQTT password (if needed)

// MQTT Topics
#define LAB73_TOPIC_TELEMETRY "v1/devices/me/telemetry" ///< Telemetry topic
#define LAB73_TOPIC_ATTRIBUTES "v1/devices/me/attributes" ///< Attributes topic
#define LAB73_TOPIC_RPC_REQUEST "v1/devices/me/rpc/request/+" ///< RPC request topic
#define LAB73_TOPIC_RPC_RESPONSE "v1/devices/me/rpc/response/" ///< RPC response topic

// Sensor Configuration (DHT22 or similar)
#define LAB73_DHT_PIN 4                         ///< DHT sensor pin
#define LAB73_DHT_TYPE DHT22                    ///< DHT sensor type

// Actuator Configuration (LED)
#define LAB73_LED_PIN 2                         ///< Built-in LED or external
#define LAB73_RELAY_PIN 23                      ///< Relay control pin

// Timing
#define LAB73_TELEMETRY_INTERVAL_MS 5000        ///< Send telemetry every 5s
#define LAB73_RECONNECT_INTERVAL_MS 5000        ///< Reconnect attempt interval

#endif
