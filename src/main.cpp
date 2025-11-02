/**
 * @file main.cpp
 * @brief Main entry point for IOT Laboratory exercises
 * 
 * This file contains the standard Arduino setup() and loop() functions.
 * Lab selection and execution is delegated to the AppManager module.
 * 
 * To switch between labs, modify ACTIVE_LAB constant in app_manager.hpp
 */

#include <Arduino.h>
#include "app_manager.hpp"

void setup() {
  appManagerSetup();
}

void loop() {
  appManagerLoop();
}
