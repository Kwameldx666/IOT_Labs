#include "dd_keypad.hpp"
#include "config.h"

namespace {

constexpr byte kRows = KEYPAD_ROWS;
constexpr byte kCols = KEYPAD_COLS;

// Physical keypad legend laid out row-by-row to match the hardware wiring.
char kKeys[kRows][kCols] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte kRowPins[kRows] = {
  KEYPAD_ROW0_PIN,
  KEYPAD_ROW1_PIN,
  KEYPAD_ROW2_PIN,
  KEYPAD_ROW3_PIN
};

byte kColPins[kCols] = {
  KEYPAD_COL0_PIN,
  KEYPAD_COL1_PIN,
  KEYPAD_COL2_PIN,
  KEYPAD_COL3_PIN
};

Keypad keypad = Keypad(makeKeymap(kKeys), kRowPins, kColPins, kRows, kCols);

} // namespace

/**
 * Reads the current keypad state and returns the pressed key character.
 * Non-blocking: returns 0 (NO_KEY) immediately if no key is pressed.
 * @return Character code of pressed key, or 0 if no key is pressed
 */
char ReadKeypad() {
  // Wrap the library call so the rest of the project has a single entrypoint.
    char key = keypad.getKey();
}
