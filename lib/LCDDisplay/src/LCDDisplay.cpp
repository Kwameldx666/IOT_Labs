#include "LCDDisplay.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include <Arduino.h>

LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

void lcdSetup() {
	Wire.begin();
	delay(20);  // give I2C expander a moment after power-up
	lcd.init();
	lcd.backlight();
	lcd.clear();
	lcd.setCursor(0, 0);
}

void clearScreen() {
	lcd.clear();
	lcd.setCursor(0, 0);
}

void lcdPrint(const char* str) {
	lcd.print(str);
}

void lcdPrint(const String& str) {
	lcd.print(str);
}

void lcdSetCursor(int column, int row) {
	lcd.setCursor(column, row);
}

void lcdPrintLine(uint8_t row, const char* fmt, ...) {
	char buffer[LCD_COLUMNS + 1];
	va_list args;
	va_start(args, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, args);
	va_end(args);

	size_t len = strlen(buffer);
	while (len < LCD_COLUMNS) {
		buffer[len++] = ' ';
	}
	buffer[LCD_COLUMNS] = '\0';

	lcd.setCursor(0, row);
	lcd.print(buffer);
}
