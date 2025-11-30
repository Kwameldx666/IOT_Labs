#ifndef LAB3_1_HPP
#define LAB3_1_HPP

#include <Arduino.h>

void setup_lab3_1();
void loop_lab3_1();

void changeRunningMode();
void updateGasReadings(int analogValue, int voltageMv);
void updateUltrasonicReadings(int durationUs, int distanceCm);

int getGasAnalogValue();
int getGasVoltageMv();
int getUltrasonicDurationUs();
int getUltrasonicDistanceCm();
byte getRunningMode();

#endif
