#ifndef DD_SERIAL_HPP
#define DD_SERIAL_HPP

#include <Arduino.h>
#include <stdio.h>

void SerialBegin();
int my_putchar(char c, FILE *stream);
int my_getchar(FILE *stream);

#endif // DD_SERIAL_HPP