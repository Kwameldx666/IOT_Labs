#include "lab1_1.hpp"
#include <stdio.h>
#include <string.h>
#include "config.h"

/**
 * Displays the list of available commands to the user via Serial output.
 */
void Instructions() {
  printf("Available commands:\n");
  printf("  led on  - Turn on the LED\n");
  printf("  led off - Turn off the LED\n");
}

/**
 * Initializes Lab 1.1: sets up Serial communication and LED hardware,
 * then prints welcome message and command instructions.
 */
void setup_lab1_1() {
  SerialBegin();
  LedIni();
  printf("Setup complete.\n");
  Instructions();
}

/**
 * Main loop for Lab 1.1: continuously reads commands from Serial input
 * and executes LED control actions. Runs in an infinite loop, waiting
 * for user to type "led on" or "led off" commands.
 */
void loop_lab1_1() {
  char input[SETUP_INIT_SIZE];

  while (true) {
    printf("\nEnter command: ");
    scanf(" %[^\n]", input);

    if (!strcmp(input, "led on")) {
      LedOn_13();
      printf("LED ON\n");
    } 
    else if (!strcmp(input, "led off")) {
      LedOff_13();
      printf("LED OFF\n");
    } 
    else {
      printf("Unknown command\n");
    }
  }
}
