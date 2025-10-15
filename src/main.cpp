#define LAB2_1
// #define LAB1_1
// #define LAB1_2

#include <Arduino.h>
#include "dd_serial.hpp"

#ifdef LAB1_1
#include "lab1/lab1_1.hpp"
#endif

#ifdef LAB1_2
#include "lab1/lab1_2.hpp"
#endif

#ifdef LAB2_1
#include "lab2/lab2_1.hpp"
#endif

void setup() {
  SerialBegin();

#ifdef LAB1_1
  setup_lab1_1();
#endif

#ifdef LAB1_2
  setup_lab1_2();
#endif

#ifdef LAB2_1
  setup_lab2_1();
#endif
}

void loop() {
#ifdef LAB1_1
  loop_lab1_1();
#endif

#ifdef LAB1_2
  loop_lab1_2();
#endif

#ifdef LAB2_1
  loop_lab2_1();
#endif
}
