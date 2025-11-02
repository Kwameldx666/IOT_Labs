/**
 * @file app_manager.cpp
 * @brief Fixed Lab Selection from Config
 * 
 * No interactive menu - the lab is selected from config.h at compile time.
 * Change LAB_SELECTED_CODE in config.h and rebuild to switch labs.
 */

#include "app_manager.hpp"
#include "config.h"

#include <Arduino.h>
#include "dd_serial.hpp"

#include "lab1/lab1_1.hpp"
#include "lab1/lab1_2.hpp"
#include "lab2/lab2_1.hpp"
#include "lab3/lab3_1.hpp"
#include "lab3/lab3_2.hpp"
#include "lab4/lab4_1.hpp"
#include "lab4/lab4_2.hpp"
// #include "lab5/lab5_1.hpp"
// #include "lab5/lab5_2.hpp"
// #include "lab6/lab6_1.hpp"
// #include "lab6/lab6_2.hpp"
// #include "lab7/lab7_1_mcu1.hpp"
// #include "lab7/lab7_1_mcu2.hpp"
// #include "lab7/lab7_2_mcu1.hpp"
// #include "lab7/lab7_2_mcu2.hpp"
// #include "lab7/lab7_3.hpp"

// Global state
namespace {
  LabSelection g_currentLab = DEFAULT_LAB;
  bool g_labInitialized = false;
}

const char* labToString(LabSelection lab) {
  switch (lab) {
    case LabSelection::NONE: return "Idle";
    case LabSelection::LAB1_1: return "LAB1.1";
    case LabSelection::LAB1_2: return "LAB1.2";
    case LabSelection::LAB2_1: return "LAB2.1";
    case LabSelection::LAB3_1: return "LAB3.1";
    case LabSelection::LAB3_2: return "LAB3.2";
    case LabSelection::LAB4_1: return "LAB4.1";
    case LabSelection::LAB4_2: return "LAB4.2";
    case LabSelection::LAB5_1: return "LAB5.1";
    case LabSelection::LAB5_2: return "LAB5.2";
    case LabSelection::LAB6_1: return "LAB6.1";
    case LabSelection::LAB6_2: return "LAB6.2";
    case LabSelection::LAB7_1_MCU1: return "LAB7.1_MCU1";
    case LabSelection::LAB7_1_MCU2: return "LAB7.1_MCU2";
    case LabSelection::LAB7_2_MCU1: return "LAB7.2_MCU1";
    case LabSelection::LAB7_2_MCU2: return "LAB7.2_MCU2";
    case LabSelection::LAB7_3: return "LAB7.3";
    default: return "?";
  }
}

// No interactive selection - lab is fixed from config

LabSelection getCurrentLab() {
  return g_currentLab;
}

void setCurrentLab(LabSelection lab) {
  g_currentLab = lab;
  g_labInitialized = false;
}

void appManagerSetup() {
  static bool firstRun = true;
  
  // Auto-select lab from config
  int labCode = LAB_SELECTED_CODE;
  
  if (firstRun) {
    SerialBegin();
    delay(1000);
    
    LabSelection newLab = g_currentLab;
    switch (labCode) {
      case 0:  newLab = LabSelection::NONE; break;
      case 11: newLab = LabSelection::LAB1_1; break;
      case 12: newLab = LabSelection::LAB1_2; break;
      case 21: newLab = LabSelection::LAB2_1; break;
      case 31: newLab = LabSelection::LAB3_1; break;
      case 32: newLab = LabSelection::LAB3_2; break;
      case 41: newLab = LabSelection::LAB4_1; break;
      case 42: newLab = LabSelection::LAB4_2; break;
      case 51: newLab = LabSelection::LAB5_1; break;
      case 52: newLab = LabSelection::LAB5_2; break;
      case 61: newLab = LabSelection::LAB6_1; break;
      case 62: newLab = LabSelection::LAB6_2; break;
      case 71: newLab = LabSelection::LAB7_1_MCU1; break;
      case 72: newLab = LabSelection::LAB7_1_MCU2; break;
      case 73: newLab = LabSelection::LAB7_2_MCU1; break;
      case 74: newLab = LabSelection::LAB7_2_MCU2; break;
      case 75: newLab = LabSelection::LAB7_3; break;
      default: newLab = LabSelection::NONE; break;
    }
    g_currentLab = newLab;
    
    printf("\n");
    printf(">> Starting: LAB");
    printf("%d", labCode);
    printf("\n\n");
    
    firstRun = false;
  }
  
  if (g_labInitialized) return;
  
  printf("Init: LAB");
  printf("%d", labCode);
  printf("\n");
  
  switch (g_currentLab) {
    case LabSelection::NONE: 
      printf("Idle.\n");
      break;
    case LabSelection::LAB1_1: setup_lab1_1(); break;
    case LabSelection::LAB1_2: setup_lab1_2(); break;
    case LabSelection::LAB2_1: setup_lab2_1(); break;
    case LabSelection::LAB3_1: setup_lab3_1(); break;
    case LabSelection::LAB3_2: setup_lab3_2(); break;
    case LabSelection::LAB4_1: setup_lab4_1(); break;
    case LabSelection::LAB4_2: setup_lab4_2(); break;
    case LabSelection::LAB5_1:
    case LabSelection::LAB5_2:
    case LabSelection::LAB6_1:
    case LabSelection::LAB6_2:
    case LabSelection::LAB7_1_MCU1:
    case LabSelection::LAB7_1_MCU2:
    case LabSelection::LAB7_2_MCU1:
    case LabSelection::LAB7_2_MCU2:
    case LabSelection::LAB7_3:
      printf("Not impl.\n");
      break;
    default:
      printf("Unknown!\n");
      break;
  }
  
  g_labInitialized = true;
  printf("Ready.\n\n");
}

void appManagerLoop() {
  // No menu or selection - just run the lab
  switch (g_currentLab) {
    case LabSelection::NONE: 
      delay(100);
      break;
    case LabSelection::LAB1_1: loop_lab1_1(); break;
    case LabSelection::LAB1_2: loop_lab1_2(); break;
    case LabSelection::LAB2_1: loop_lab2_1(); break;
    case LabSelection::LAB3_1: loop_lab3_1(); break;
    case LabSelection::LAB3_2: loop_lab3_2(); break;
    case LabSelection::LAB4_1: loop_lab4_1(); break;
    case LabSelection::LAB4_2: loop_lab4_2(); break;
    case LabSelection::LAB5_1:
    case LabSelection::LAB5_2:
    case LabSelection::LAB6_1:
    case LabSelection::LAB6_2:
    case LabSelection::LAB7_1_MCU1:
    case LabSelection::LAB7_1_MCU2:
    case LabSelection::LAB7_2_MCU1:
    case LabSelection::LAB7_2_MCU2:
    case LabSelection::LAB7_3:
      delay(100);
      break;
    default:
      delay(100);
      break;
  }
}
