#include "app_manager.hpp"

#include <Arduino.h>
#include "dd_serial.hpp"

// Lab includes
#include "lab1/lab1_1.hpp"
#include "lab1/lab1_2.hpp"
#include "lab2/lab2_1.hpp"

void appManagerSetup() {
  SerialBegin();
  
  switch (ACTIVE_LAB) {
    case LabSelection::LAB1_1:
      setup_lab1_1();
      break;
      
    case LabSelection::LAB1_2:
      setup_lab1_2();
      break;
      
    case LabSelection::LAB2_1:
      setup_lab2_1();
      break;
      
    case LabSelection::NONE:
    default:
      Serial.println("No lab selected. Please configure ACTIVE_LAB in app_manager.hpp");
      break;
  }
}

void appManagerLoop() {
  switch (ACTIVE_LAB) {
    case LabSelection::LAB1_1:
      loop_lab1_1();
      break;
      
    case LabSelection::LAB1_2:
      loop_lab1_2();
      break;
      
    case LabSelection::LAB2_1:
      loop_lab2_1();
      break;
      
    case LabSelection::NONE:
    default:
      delay(1000);
      break;
  }
}
#ifndef APP_MANAGER_HPP
#define APP_MANAGER_HPP

/**
 * @file app_manager.hpp
 * @brief Application Manager - centralized control for lab selection and execution
 * 
 * This module provides a clean interface for managing different laboratory exercises.
 * Instead of using #define directives, it uses a configuration-based approach.
 */

enum class LabSelection {
  NONE,
  LAB1_1,  // Serial LED control
  LAB1_2,  // Additional lab 1 exercise
  LAB2_1,  // Sequential scheduler with LEDs and buttons
};

/**
 * Configure which lab should run
 * Change this constant to switch between labs
 */
constexpr LabSelection ACTIVE_LAB = LabSelection::LAB2_1;

/**
 * Initialize the selected laboratory
 */
void appManagerSetup();

/**
 * Run the main loop of the selected laboratory
 */
void appManagerLoop();

#endif // APP_MANAGER_HPP

