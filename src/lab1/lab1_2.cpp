/**
 * @file lab1_2.cpp
 * @brief Lab 1.2 - Access Control with Keypad (Unified, Uses Libraries)
 * 
 * Complete implementation using reusable libraries:
 * - StatusIndicator for LED feedback
 * - LcdHelper for display formatting
 * - own_stdio for keypad input
 */

#include "lab1_2.hpp"
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "own_stdio.h"
#include "status_indicator.hpp"
#include "lcd_helper.hpp"
#include "dd_led.hpp"

namespace {

bool g_accessGranted = false;
bool g_promptShown = false;

// Use reusable StatusIndicator library
StatusIndicator statusLed(GREEN_LED_PIN, RED_LED_PIN);

/**
 * Displays the input prompt on the LCD if not already shown.
 */
void showPrompt() {
    if (g_promptShown) {
        return;
    }

    own_stdio_set_input_limit(ACCESS_CODE_LENGTH);
    LcdHelper::showStatus("WAIT", "Code: ");
    g_promptShown = true;
}

/**
 * Shows the access result on LCD and via LEDs with timed hold.
 */
void reportResult(bool success, const char* code) {
    char displayLine[20];
    snprintf(displayLine, sizeof(displayLine), "Code: %s", code);
    
    if (success) {
        Serial.println(F("[Lab1.2] Access granted."));
        LcdHelper::showStatus("OK", displayLine);
        statusLed.showAndHold(StatusType::SUCCESS, LAB1_RESULT_HOLD_MS, LAB1_RESULT_TICK_MS);
        g_accessGranted = true;
    } else {
        Serial.println(F("[Lab1.2] Access denied."));
        LcdHelper::showStatus("FAIL", displayLine);
        statusLed.showAndHold(StatusType::FAILURE, LAB1_RESULT_HOLD_MS, LAB1_RESULT_TICK_MS);
    }
}

} // namespace

/**
 * Initializes Lab 1.2: sets up LEDs, prints instructions to Serial monitor,
 * initializes the custom stdio system with LCD/keypad, and displays the initial
 * prompt asking for the 4-digit access code.
 */
void setup_lab1_2() {
    // Initialize hardware
    LedIni();
    statusLed.begin();
    statusLed.clear();

    // Print instructions
    Serial.println(F("\n==== Lab 1.2: Access Control ===="));
    Serial.println(F("Enter 4-digit code on keypad"));
    Serial.println(F("'*' = erase, '#' = submit\n"));

    // Initialize LCD/Keypad stdio
    own_stdio_init(SERIAL_BAUD_RATE);
    own_stdio_set_input_limit(ACCESS_CODE_LENGTH);

    g_accessGranted = false;
    g_promptShown = false;

    showPrompt();
}

void loop_lab1_2() {
    static char input[ACCESS_CODE_BUFFER_SIZE] = {0};

    if (g_accessGranted) {
        return;  // Exit if access granted
    }

    showPrompt();

    // Wait for keypad input
    int result = scanf("%4s", input);
    if (result != 1) {
        return;
    }

    g_promptShown = false;

    // Validate code
    bool accessGranted = (strlen(input) == ACCESS_CODE_LENGTH) && 
                         (strcmp(input, ACCESS_CODE) == 0);
    
    reportResult(accessGranted, input);

    if (!accessGranted) {
        // Reset for retry
        memset(input, 0, sizeof(input));
        own_stdio_set_input_limit(ACCESS_CODE_LENGTH);
        showPrompt();
    }
}
