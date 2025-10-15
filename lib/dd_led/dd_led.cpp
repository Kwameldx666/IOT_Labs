#include "dd_led.hpp"
#include "config.h"

/**
 * Initializes all LED pins as outputs. Must be called during setup before
 * using any LED control functions.
 */
void LedIni() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
}

/**
 * Turns on the main onboard LED (typically pin 13 on Arduino boards).
 */
void LedOn_13() {
  digitalWrite(LED_PIN, HIGH);
}

/**
 * Turns off the main onboard LED (typically pin 13 on Arduino boards).
 */
void LedOff_13() {
  digitalWrite(LED_PIN, LOW);
}

/**
 * Turns on the green indicator LED. Used to signal successful access.
 */
void LedOnGreen() {
  digitalWrite(GREEN_LED_PIN, HIGH);
}

/**
 * Turns on the red indicator LED. Used to signal denied access or errors.
 */
void LedOnRed() {
  digitalWrite(RED_LED_PIN, HIGH);
}

/**
 * Turns off all LEDs (main, green, and red). Used to reset visual indicators.
 */
void LedOffAll() {
  digitalWrite(LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW); // Turn off green LED
  digitalWrite(RED_LED_PIN, LOW);   // Turn off red LED
}

/**
 * Turns off only the green LED without affecting other indicators.
 */
void LedOffGreen() {
  digitalWrite(GREEN_LED_PIN, LOW);
}

/**
 * Turns off only the red LED without affecting other indicators.
 */
void LedOffRed() {
  digitalWrite(RED_LED_PIN, LOW);
}
