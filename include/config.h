#ifndef CONFIG_H
#define CONFIG_H

// Command input buffer size
#define SETUP_INIT_SIZE 64
#define COMMAND_TOKEN_BUFFER 16

// Serial configuration
#define SERIAL_BAUD_RATE 9600
#define SERIAL_INIT_DELAY_MS 100

// Timing configuration
#define KEYPAD_POLL_DELAY_MS 10
#define LAB1_RESULT_HOLD_MS 2000
#define LAB1_RESULT_TICK_MS 10

// Keypad pin assignments (Arduino Mega)
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

// Access control
#define ACCESS_CODE "1234"
#define ACCESS_CODE_LENGTH 4
#define ACCESS_CODE_BUFFER_SIZE (ACCESS_CODE_LENGTH + 1)

// LCD configuration
#define LCD_I2C_ADDRESS 0x27
#define LCD_COLUMNS 16
#define LCD_ROWS 2


// LED pin assignments
#define LED_PIN 12
#define GREEN_LED_PIN 10
#define RED_LED_PIN 4

// Button inputs
#define BUTTON_PIN 11
#define BUTTON_TOGGLE_PIN 7
#define BUTTON_INC_PIN 6
#define BUTTON_DEC_PIN 5
#define BUTTON_DEBOUNCE_MS 35

#endif
