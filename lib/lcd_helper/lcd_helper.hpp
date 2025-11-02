#ifndef LCD_HELPER_HPP
#define LCD_HELPER_HPP

class LcdHelper {
public:
  static void showMessage(const char* line1, const char* line2 = nullptr);
  static void showStatus(const char* status, const char* value = nullptr);
  static void showInput(const char* label, const char* value);
};

#endif
