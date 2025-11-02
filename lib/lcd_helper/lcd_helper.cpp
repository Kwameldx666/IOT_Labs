#include "lcd_helper.hpp"
#include <stdio.h>
#include "own_stdio.h"

void LcdHelper::showMessage(const char* line1, const char* line2) {
  own_stdio_reset_display();
  printf("%s", line1);
  if (line2 != nullptr) {
    printf("\n%s", line2);
  }
  fflush(stdout);
}

void LcdHelper::showStatus(const char* status, const char* value) {
  own_stdio_reset_display();
  printf("Access: %s", status);
  if (value != nullptr && value[0] != '\0') {
    printf("\n%s", value);
  }
  fflush(stdout);
}

void LcdHelper::showInput(const char* label, const char* value) {
  own_stdio_reset_display();
  printf("%s%s", label, value != nullptr ? value : "");
  fflush(stdout);
}
#ifndef LCD_HELPER_HPP
#define LCD_HELPER_HPP

/**
 * @file lcd_helper.hpp
 * @brief Reusable LCD display formatting helpers
 * 
 * Provides utilities for formatted LCD output.
 */

/**
 * @class LcdHelper
 * @brief Helper for formatted LCD output
 */
class LcdHelper {
public:
  /**
   * @brief Clear display and show two-line message
   * @param line1 First line text
   * @param line2 Second line text
   */
  static void showMessage(const char* line1, const char* line2 = nullptr);

  /**
   * @brief Show status with label
   * @param status Status text (e.g., "WAIT", "OK", "FAIL")
   * @param value Optional value to display
   */
  static void showStatus(const char* status, const char* value = nullptr);

  /**
   * @brief Show labeled input field
   * @param label Label text (e.g., "Code: ")
   * @param value Current input value
   */
  static void showInput(const char* label, const char* value);
};

#endif // LCD_HELPER_HPP

