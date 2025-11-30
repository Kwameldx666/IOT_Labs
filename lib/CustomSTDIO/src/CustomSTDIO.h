#ifndef CUSTOM_STDIO_H
#define CUSTOM_STDIO_H

#include <Arduino.h>
#include <stdio.h>

void StdioSerialSetup();
void printFloat(const char* label, float value, const char* suffix);
bool stdioHasData();
String stdioGetString();

#endif
