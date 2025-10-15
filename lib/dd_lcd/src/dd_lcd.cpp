#include "dd_lcd.hpp"
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include "config.h"

namespace {

// Use the minimal constructor signature so wiring-specific defaults remain in
// the library. This avoids hard-coding pin maps and keeps compatibility with
// typical PCF8574 backpacks.
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

char stream_buffer[LCD_ROWS][LCD_COLUMNS];
uint8_t stream_length[LCD_ROWS];
uint8_t stream_row = 0;
uint8_t stream_col = 0;
bool stream_active = false;

/**
 * Clears the specified row in the internal buffer by resetting length to zero
 * and filling all columns with spaces.
 * @param row Row index to clear (0-based)
 */
void clearRow(uint8_t row) {
    stream_length[row] = 0;
    for (uint8_t col = 0; col < LCD_COLUMNS; ++col) {
        stream_buffer[row][col] = ' ';
    }
}

/**
 * Synchronizes the physical LCD display with the internal framebuffer for a
 * specific row. Writes valid characters and fills the rest with spaces.
 * @param row Row index to refresh (0-based)
 */
void refreshRow(uint8_t row) {
    lcd.setCursor(0, row);
    for (uint8_t col = 0; col < LCD_COLUMNS; ++col) {
        char value = (col < stream_length[row]) ? stream_buffer[row][col] : ' ';
        lcd.write(value);
    }
}

/**
 * Refreshes all rows of the LCD display by iterating through each row and
 * calling refreshRow() to synchronize buffer contents with physical display.
 */
void refreshAllRows() {
    for (uint8_t row = 0; row < LCD_ROWS; ++row) {
        refreshRow(row);
    }
}

/**
 * Implements terminal-style scrolling by shifting all row contents upward.
 * The bottom row is cleared and ready to accept new content.
 */
void shiftRowsUp() {
    // Scroll the framebuffer up by one row so new text can appear at the bottom
    // once we reach the last physical LCD line.
    for (uint8_t row = 0; row < LCD_ROWS - 1; ++row) {
        stream_length[row] = stream_length[row + 1];
        memcpy(stream_buffer[row], stream_buffer[row + 1], LCD_COLUMNS);
    }
    clearRow(LCD_ROWS - 1);
}

/**
 * Copies a null-terminated string into the specified row buffer and updates
 * the physical LCD. Truncates text if it exceeds LCD_COLUMNS.
 * @param row Target row index (0-based)
 * @param text Source string to copy (may be NULL)
 * @return Number of characters actually copied
 */
uint8_t copyTextToRow(uint8_t row, const char* text) {
    clearRow(row);
    if (!text) {
        refreshRow(row);
        return 0;
    }

    uint8_t length = 0;
    while (text[length] != '\0' && length < LCD_COLUMNS) {
        stream_buffer[row][length] = text[length];
        ++length;
    }
    stream_length[row] = length;
    refreshRow(row);
    return length;
}

/**
 * Advances the cursor to the beginning of the next line. If already on the
 * last row, scrolls the entire display upward to make room for new content.
 */
void moveToNewLine() {
    if (!stream_active) {
        return;
    }

    if (stream_row < LCD_ROWS - 1) {
        ++stream_row;
    } else {
        // When we're already on the last row we need to shift everything up so
        // the stream behaves like a terminal with automatic scrolling.
        shiftRowsUp();
        stream_row = LCD_ROWS - 1;
    }
    stream_col = stream_length[stream_row];
    refreshAllRows();
}

/**
 * Directly writes a string to the physical LCD at the specified row without
 * updating the internal stream buffer. Pads with spaces to fill the row.
 * @param row Target row index (0-based)
 * @param text String to display (may be NULL)
 */
void printLine(uint8_t row, const char* text) {
    lcd.setCursor(0, row);
    for (uint8_t col = 0; col < LCD_COLUMNS; ++col) {
        char value = (text && col < strlen(text)) ? text[col] : ' ';
        lcd.write(value);
    }
}

} // namespace

/**
 * Initializes the LCD hardware via I2C, clears the display, activates the
 * backlight, and resets the internal streaming buffer to a clean state.
 */
void LcdIni() {
    lcd.init(); 
    delay(50); 
    lcd.begin(LCD_COLUMNS, LCD_ROWS);
    lcd.clear();
    lcd.backlight();

    stream_active = false;
    stream_row = 0;
    stream_col = 0;
    for (uint8_t row = 0; row < LCD_ROWS; ++row) {
        clearRow(row);
    }
}

/**
 * Activates the LCD stream mode by clearing the internal framebuffer and
 * resetting cursor position to (0,0). Prepares the display for stdio output.
 */
void LcdStreamInit() {
    stream_active = true;
    stream_row = 0;
    stream_col = 0;
    for (uint8_t row = 0; row < LCD_ROWS; ++row) {
        clearRow(row);
    }
    refreshAllRows();
}

/**
 * Stdio-compatible character output function. Handles newlines with automatic
 * scrolling and word-wraps at column boundaries. Ignores carriage returns.
 * @param c Character to display
 * @return Always returns 0 (success)
 */
int LcdStreamPutchar(char c) {
    if (!stream_active) {
        return 0;
    }

    if (c == '\r') {
        return 0;
    }

    if (c == '\n') {
        // Carriage return is handled separately by STDIO while newline moves to
        // the next row and redraws the screen as needed.
        moveToNewLine();
        return 0;
    }

    if (stream_col >= LCD_COLUMNS) {
        moveToNewLine();
    }

    if (stream_col >= LCD_COLUMNS) {
        return 0;
    }

    stream_buffer[stream_row][stream_col] = c;
    if (stream_length[stream_row] <= stream_col) {
        stream_length[stream_row] = stream_col + 1;
    }
    ++stream_col;
    refreshRow(stream_row);
    return 0;
}

/**
 * Deletes the last character from the stream buffer. If the cursor is at the
 * start of a line, steps back to the previous line. Updates the LCD display.
 */
void LcdStreamBackspace() {
    if (!stream_active) {
        return;
    }

    if (stream_col > 0) {
        // Remove a character within the current line and redraw the row so the
        // LCD no longer shows the trailing symbol.
        --stream_col;
        if (stream_length[stream_row] > stream_col) {
            stream_length[stream_row] = stream_col;
        }
        stream_buffer[stream_row][stream_col] = ' ';
        refreshRow(stream_row);
        return;
    }

    if (stream_row == 0) {
        return;
    }

    if (stream_length[stream_row] == 0) {
        // If the current line is empty, step to the previous (non-empty) line to
        // continue deleting characters from there.
        --stream_row;
    }

    if (stream_length[stream_row] > 0) {
        stream_col = stream_length[stream_row];
        if (stream_col > 0) {
            --stream_col;
            stream_length[stream_row] = stream_col;
            stream_buffer[stream_row][stream_col] = ' ';
            refreshRow(stream_row);
        }
    } else {
        stream_col = 0;
    }
}

/**
 * Displays a single-line message on the LCD. Clears the entire display first,
 * then shows the message on row 0. Updates stream buffer if streaming is active.
 * @param message Text to display (may be NULL for blank display)
 */
void DisplayMessage(const char* message) {
    lcd.clear();
    printLine(0, message ? message : "");
    for (uint8_t row = 1; row < LCD_ROWS; ++row) {
        printLine(row, "");
    }

    if (stream_active) {
        stream_row = 0;
        stream_col = copyTextToRow(0, message);
        for (uint8_t row = 1; row < LCD_ROWS; ++row) {
            clearRow(row);
        }
        refreshAllRows();
    }
}

/**
 * Displays two lines of text on the LCD. Clears the display first, then shows
 * line1 on row 0 and line2 on row 1. Updates stream buffer if streaming is active.
 * @param line1 Text for first row (may be NULL)
 * @param line2 Text for second row (may be NULL)
 */
void DisplayMessageTwoLines(const char* line1, const char* line2) {
    lcd.clear();
    printLine(0, line1 ? line1 : "");
    if (LCD_ROWS > 1) {
        printLine(1, line2 ? line2 : "");
    }

    if (stream_active) {
        stream_row = 0;
        stream_col = copyTextToRow(0, line1);
        if (LCD_ROWS > 1) {
            stream_row = 1;
            stream_col = copyTextToRow(1, line2);
        }
        for (uint8_t row = (LCD_ROWS > 1 ? 2 : 1); row < LCD_ROWS; ++row) {
            clearRow(row);
        }
        refreshAllRows();
    }
}
