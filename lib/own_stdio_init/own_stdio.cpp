#include "own_stdio.h"
#include <Arduino.h>
#include <stdio.h>
#include "config.h"
#include <dd_lcd.hpp>
#include "../keypad/dd_keypad.hpp"

namespace {

// FILE structure that glues the stdio layer to our LCD/keypad handlers.
static FILE lcd_stdio_stream = {0};
// Circular-ish buffer that captures keypad characters until the user submits.
static char pending_buffer[COMMAND_TOKEN_BUFFER];
static size_t pending_length = 0;
static size_t pending_index = 0;
// Upper bound enforced for `scanf`-style input so higher layers can restrict
// how much data a user can enter before pressing the confirm key.
static size_t pending_limit = sizeof(pending_buffer) - 1;

/**
 * Stdio output function that redirects all outgoing characters to the LCD
 * stream for display. Required for fdev_setup_stream integration.
 * @param c Character to output
 * @param stream FILE pointer (unused)
 * @return Always returns 0 (success)
 */
int lcd_stream_putchar(char c, FILE* /*stream*/) {
    // Forward every outgoing stdio character to the LCD stream renderer.
	LcdStreamPutchar(c);
	return 0;
}

/**
 * Stdio input function that blocks until the user enters a complete line via
 * keypad. Handles special keys: '#' submits, '*' deletes. Enforces input limit.
 * @param stream FILE pointer (unused)
 * @return Next character from the pending buffer
 */
int keypad_stream_getchar(FILE* /*stream*/) {
	while (true) {
		if (pending_index < pending_length) {
			return pending_buffer[pending_index++];
		}

		pending_index = 0;
		pending_length = 0;

		while (true) {
			char key = ReadKeypad();
			if (!key) {
				delay(KEYPAD_POLL_DELAY_MS);
				continue;
			}

			if (key == '#') {
				// The hash key acts as an “enter”: terminate the current line and
				// return it to the stdio consumer.
				if (pending_length < sizeof(pending_buffer) - 1) {
					pending_buffer[pending_length++] = '\n';
				}
				LcdStreamPutchar('\n');
				break;
			}

			if (key == '*') {
				// Star mimics a backspace. Adjust the buffer and mirror the change on
				// the LCD so the user sees the deletion immediately.
				if (pending_length > 0) {
					--pending_length;
					LcdStreamBackspace();
				}
				continue;
			}

			if (pending_length < pending_limit) {
				// Printable character path: append to the buffer and echo to the LCD.
				pending_buffer[pending_length++] = key;
				LcdStreamPutchar(key);
			}
		}
	}
}

} // namespace

/**
 * Initializes the custom stdio system by setting up LCD, keypad hardware,
 * and redirecting stdin/stdout/stderr to use the LCD/keypad bridge.
 * @param baud_rate Serial baud rate (unused in this implementation)
 */
void own_stdio_init(uint32_t baud_rate) {
	(void)baud_rate;

	LcdIni();
	LcdStreamInit();
	KeypadIni();
	// Default to the largest safe limit unless an individual module narrows it.
	pending_limit = sizeof(pending_buffer) - 1;

	fdev_setup_stream(&lcd_stdio_stream, lcd_stream_putchar, keypad_stream_getchar, _FDEV_SETUP_RW);
	stdin = stdout = stderr = &lcd_stdio_stream;
}

/**
 * Resets the LCD display state by clearing the pending input buffer and
 * reinitializing the LCD stream. Useful for starting fresh after displaying results.
 */
void own_stdio_reset_display() {
	pending_index = 0;
	pending_length = 0;
	LcdStreamInit();
}

/**
 * Sets the maximum number of characters the user can enter before being forced
 * to submit with '#'. Useful for constraining input length (e.g., 4-digit codes).
 * @param limit Maximum input length (0 or values >= buffer size reset to default)
 */
void own_stdio_set_input_limit(size_t limit) {
	if (limit == 0 || limit >= sizeof(pending_buffer)) {
		pending_limit = sizeof(pending_buffer) - 1;
		return;
	}
	pending_limit = limit;
}
