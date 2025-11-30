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
#include "CustomSTDIO.h"

#include "lab1/lab1_1.hpp"
#include "lab1/lab1_2.hpp"
#include "lab2/lab2_1.hpp"
#include "lab3/lab3_1.hpp"
#include "lab4/lab4_1.hpp"
#include "lab4/lab4_2.hpp"
#include "lab5/lab5_1.hpp"
#include "lab5/lab5_2.hpp"
#include "lab6/lab6_1.hpp"
#include "lab6/lab6_2.hpp"

static LabSelection g_currentLab = DEFAULT_LAB;
static bool g_labInitialized = false;

const char* labToString(LabSelection lab) {
  switch (lab) {
    case LabSelection::NONE:
      return "Idle";
    case LabSelection::LAB1_1:
      return "LAB1.1";
    case LabSelection::LAB1_2:
      return "LAB1.2";
    case LabSelection::LAB3_1:
      return "LAB3.1";
    case LabSelection::LAB2_1:
      return "LAB2.1";
    case LabSelection::LAB4_1:
      return "LAB4.1";
    case LabSelection::LAB4_2:
      return "LAB4.2";
    case LabSelection::LAB5_1:
      return "LAB5.1";
    case LabSelection::LAB5_2:
      return "LAB5.2";
    case LabSelection::LAB6_1:
      return "LAB6.1";
    case LabSelection::LAB6_2:
      return "LAB6.2";
    default:
      return "?";
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
    StdioSerialSetup();
    delay(500);
    
    LabSelection newLab = g_currentLab;
    switch (labCode) {
      case 0:  newLab = LabSelection::NONE; break;
      case 11: newLab = LabSelection::LAB1_1; break;
      case 12: newLab = LabSelection::LAB1_2; break;
      case 21: newLab = LabSelection::LAB2_1; break;
      case 31: newLab = LabSelection::LAB3_1; break;
      case 41: newLab = LabSelection::LAB4_1; break;
      case 42: newLab = LabSelection::LAB4_2; break;
      case 51: newLab = LabSelection::LAB5_1; break;
      case 52: newLab = LabSelection::LAB5_2; break;
      case 61: newLab = LabSelection::LAB6_1; break;
      case 62: newLab = LabSelection::LAB6_2; break;
      default: newLab = LabSelection::LAB1_1; break;
    }
    g_currentLab = newLab;
    
    printf("\r\n>> Starting: LAB%d\r\n\r\n", labCode);
    
    firstRun = false;
  }
  
  if (g_labInitialized) return;
  
  printf("Init: LAB%d\r\n", labCode);
  
  switch (g_currentLab) {
    case LabSelection::NONE: 
      printf("Idle.\r\n");
      break;
    case LabSelection::LAB1_1:
      setup_lab1_1();
      break;
    case LabSelection::LAB1_2:
      setup_lab1_2();
      break;
    case LabSelection::LAB2_1:
      setup_lab2_1();
      break;
    case LabSelection::LAB3_1:
      setup_lab3_1();
      break;
    case LabSelection::LAB4_1: setup_lab4_1(); break;
    case LabSelection::LAB4_2: setup_lab4_2(); break;
    case LabSelection::LAB5_1: setup_lab5_1(); break;
    case LabSelection::LAB5_2: setup_lab5_2(); break;
    case LabSelection::LAB6_1: setup_lab6_1(); break;
    case LabSelection::LAB6_2: setup_lab6_2(); break;
    default:
      printf("Unknown!\r\n");
      break;
  }
  
  g_labInitialized = true;
  printf("Ready.\r\n\r\n");
}

void appManagerLoop() {
  // No menu or selection - just run the lab
  switch (g_currentLab) {
    case LabSelection::NONE: 
      delay(100);
      break;
    case LabSelection::LAB1_1:
      loop_lab1_1();
      break;
    case LabSelection::LAB1_2:
      loop_lab1_2();
      break;
    case LabSelection::LAB2_1:
      loop_lab2_1();
      break;
    case LabSelection::LAB3_1:
      loop_lab3_1();
      break;
    case LabSelection::LAB4_1: loop_lab4_1(); break;
    case LabSelection::LAB4_2: loop_lab4_2(); break;
    case LabSelection::LAB5_1: loop_lab5_1(); break;
    case LabSelection::LAB5_2: loop_lab5_2(); break;
    case LabSelection::LAB6_1: loop_lab6_1(); break;
    case LabSelection::LAB6_2: loop_lab6_2(); break;
    default:
      delay(100);
      break;
  }
}
