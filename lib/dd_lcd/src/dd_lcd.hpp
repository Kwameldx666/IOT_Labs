#ifndef DD_LCD_HPP
#define DD_LCD_HPP

#include <Arduino.h>

void LcdIni();
void LcdStreamInit();
int LcdStreamPutchar(char c);
void LcdStreamBackspace();
void DisplayMessage(const char* message);
void DisplayMessageTwoLines(const char* line1, const char* line2);

#endif
