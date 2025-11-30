#include "CustomKeypad.h"
#include <Arduino.h>

static const byte ROWS = 4;
static const byte COLS = 4;

static char kKeys[ROWS][COLS] = {
	{ '1', '2', '3', 'A' },
	{ '4', '5', '6', 'B' },
	{ '7', '8', '9', 'C' },
	{ '*', '0', '#', 'D' }
};

static byte rowPins[ROWS] = { A0, A1, A2, A3 };
static byte colPins[COLS] = { A4, A5, A6, A7 };

Keypad keypad = Keypad(makeKeymap(kKeys), rowPins, colPins, ROWS, COLS);
