#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#include <LiquidCrystal_I2C.h>
#include "config.h"

extern LiquidCrystal_I2C lcd;

void lcdSetup();
void clearScreen();
void lcdPrint(const char* str);
void lcdPrint(const String& str);
void lcdSetCursor(int column, int row);
void lcdPrintLine(uint8_t row, const char* fmt, ...);

#endif
