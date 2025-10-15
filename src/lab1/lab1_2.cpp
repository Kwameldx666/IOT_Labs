// lab1_2.cpp
#include "lab1_2.hpp"
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "own_stdio.h"

namespace {

constexpr size_t kCodeBufferSize = ACCESS_CODE_BUFFER_SIZE;
constexpr char kConsoleInstructions[] = "Lab 1.2: Enter the 4-digit access code using the keypad.";
constexpr char kConsoleHelp[] = "Use '*' to erase the last digit and '#' to submit.";
constexpr char kConsoleNote[] = "Keypad input appears on the LCD; press '#' to submit.";
constexpr char kStatusWaiting[] = "Access: WAIT";
constexpr char kStatusGranted[] = "Access: OK";
constexpr char kStatusDenied[] = "Access: FAIL";
constexpr char kInputLabel[] = "Code: ";

bool g_accessGranted = false;
bool g_promptShown = false;

/**
 * Turns off all LED indicators (red, green, and main LED).
 * Used to reset visual feedback before showing new status.
 */
void resetIndicators() {
    LedOffAll();
}

/**
 * Updates the LCD display with current access status and the code entered so far.
 * Clears and reinitializes the LCD stream before displaying the status message.
 * @param status Status message to display on line 1 (e.g., "Access: WAIT")
 * @param codeSuffix Partial or complete access code to show on line 2 after "Code: "
 */
void renderStatusScreen(const char* status, const char* codeSuffix) {
    // Refresh the LCD-backed stdio surface so the user always sees the latest
    // state headline and the code they've typed so far.
    own_stdio_reset_display();
    printf("%s\n", status);
    printf("%s%s", kInputLabel, (codeSuffix && codeSuffix[0] != '\0') ? codeSuffix : "");
    fflush(stdout);
}

/**
 * Displays the input prompt on the LCD if not already shown. Sets input limit
 * to exactly 4 characters and shows the waiting status.
 */
void showPrompt() {
    if (g_promptShown) {
        return;
    }

    // Clamp keypad input to the expected four digits and draw the waiting UI.
    own_stdio_set_input_limit(ACCESS_CODE_LENGTH);
    renderStatusScreen(kStatusWaiting, "");
    g_promptShown = true;
}

/**
 * Resets the prompt state flag so the next iteration can display a fresh prompt.
 */
void resetPromptState() {
    g_promptShown = false;
}

/**
 * Shows the access result on LCD and via LEDs, then blocks for a brief period
 * to let the user see the feedback before continuing.
 * @param success True if access was granted, false if denied
 * @param code The code that was entered (for display purposes)
 */
void reportResult(bool success, const char* code) {
    resetIndicators();
    if (success) {
        Serial.println(F("[Lab1.2] Access granted."));
        LedOnGreen();
        renderStatusScreen(kStatusGranted, code);
        g_accessGranted = true;
    } else {
        Serial.println(F("[Lab1.2] Access denied."));
        LedOnRed();
        renderStatusScreen(kStatusDenied, code);
    }

    unsigned long start = millis();
    while (millis() - start < LAB1_RESULT_HOLD_MS) {
        // Provide a short dwell so the user can see the LEDs/LCD feedback while
        // still yielding to the scheduler via a small delay.
        delay(LAB1_RESULT_TICK_MS);
    }
}

} // namespace

/**
 * Initializes Lab 1.2: sets up LEDs, prints instructions to Serial monitor,
 * initializes the custom stdio system with LCD/keypad, and displays the initial
 * prompt asking for the 4-digit access code.
 */
void setup_lab1_2() {
    LedIni();
    resetIndicators();

    Serial.println();
    Serial.println(F("==== Lab 1.2 ===="));
    Serial.println(kConsoleInstructions);
    Serial.println(kConsoleHelp);
    Serial.println(kConsoleNote);
    Serial.println();

    own_stdio_init(SERIAL_BAUD_RATE);
    own_stdio_set_input_limit(ACCESS_CODE_LENGTH);

    g_accessGranted = false;
    g_promptShown = false;

    showPrompt();
}

/**
 * Main loop for Lab 1.2: waits for user to enter 4-digit code via keypad,
 * validates the code, displays result with LED feedback, and allows retry if
 * access was denied. Exits early if access has already been granted.
 */
void loop_lab1_2() {
    static char input[kCodeBufferSize] = {0};

    if (g_accessGranted) {
        return;
    }

    showPrompt();

    // Block until the keypad stdio layer returns a full submission.
    int result = scanf("%4s", input);
    if (result != 1) {
        return;
    }

    resetPromptState();

    bool accessGranted = (strlen(input) == ACCESS_CODE_LENGTH) && (strcmp(input, ACCESS_CODE) == 0);
    reportResult(accessGranted, input);

    if (!accessGranted) {
        // Reset state so the next attempt starts with a blank LCD and tight limit.
        memset(input, 0, sizeof(input));
        own_stdio_set_input_limit(ACCESS_CODE_LENGTH);
        showPrompt();
    }
}
