#include "dd_serial.hpp"
#include <stdio.h>
#include "config.h"

// STDIO functions for Arduino Serial

static FILE serial_stdio_stream = {0};

/**
 * Initializes the hardware Serial port and redirects stdin/stdout/stderr to
 * use Serial communication. Allows standard printf/scanf to work over USB.
 */
void SerialBegin() {
    Serial.begin(SERIAL_BAUD_RATE);
    while (!Serial) {
        ; // Wait for Serial port to connect (needed for some boards)
    }
    delay(SERIAL_INIT_DELAY_MS); // Wait for Serial port to stabilize
    Serial.flush(); // Clear any garbage in buffer
    fdev_setup_stream(&serial_stdio_stream, my_putchar, my_getchar, _FDEV_SETUP_RW);
    stdin  = stdout = stderr = &serial_stdio_stream;
}

/**
 * Stdio output function for Serial. Converts Unix newlines to Windows-style
 * CRLF for proper terminal display.
 * @param c Character to transmit
 * @param stream FILE pointer (unused)
 * @return Always returns 0 (success)
 */
int my_putchar(char c, FILE *stream) {
    if(c == '\n') {
        Serial.write('\r');
    }
    Serial.write(c);
    return 0;
}

/**
 * Stdio input function for Serial. Blocks until a character is available,
 * converts carriage return to newline, and echoes back to terminal.
 * @param stream FILE pointer (unused)
 * @return Character received from Serial
 */
int my_getchar(FILE *stream) {
    while (!Serial.available());
    int c = Serial.read();

    if (c == '\r') {          
        Serial.write("\r\n"); 
        return '\n';     
    }

    Serial.write(c);          
    return c;
}
