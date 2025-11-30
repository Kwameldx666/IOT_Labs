#include "lab6/lab6_1.hpp"

#include <stdio.h>

#include "CustomSTDIO.h"
#include "DriverLed.h"
#include "config.h"

namespace {

enum class LedState : uint8_t { Off = 0, On = 1 };

struct State {
	uint8_t output;
	unsigned long holdMs;
	LedState next[2];
};

constexpr State kFsm[] = {
		{LOW, LAB61_STATE_HOLD_MS, {LedState::Off, LedState::On}},
		{HIGH, LAB61_STATE_HOLD_MS, {LedState::On, LedState::Off}},
};

LedState g_state = LedState::Off;

bool g_stablePressed = false;
bool g_lastRawPressed = false;
unsigned long g_lastDebounceMs = 0;

constexpr size_t idx(LedState state) {
	return static_cast<size_t>(state);
}

bool readButtonPress() {
	const bool rawPressed = digitalRead(LAB61_BUTTON_PIN) == HIGH;  // active HIGH (external pulldown)
	const unsigned long now = millis();

	if (rawPressed != g_lastRawPressed) {
		g_lastDebounceMs = now;
		g_lastRawPressed = rawPressed;
	}

	if ((now - g_lastDebounceMs) >= LAB61_DEBOUNCE_MS &&
			rawPressed != g_stablePressed) {
		g_stablePressed = rawPressed;
		if (g_stablePressed) {
			return true;  // rising edge (button pressed)
		}
	}

	return false;
}

void applyOutput(LedState state) {
	changeLedState(LAB61_LED_PIN, kFsm[idx(state)].output);
}

}  // namespace

void setup_lab6_1() {
	StdioSerialSetup();

	pinMode(LAB61_BUTTON_PIN, INPUT);  // external pulldown on schematic
	ledSetup(LAB61_LED_PIN);

	g_lastRawPressed = digitalRead(LAB61_BUTTON_PIN) == LOW;
	g_stablePressed = g_lastRawPressed;
	g_lastDebounceMs = millis();

	applyOutput(g_state);

	printf("[Lab6.1] FSM LED toggle ready.\r\n");
	printf("LED on D%d, button on D%d (active HIGH, debounce %d ms).\r\n",
				 LAB61_LED_PIN, LAB61_BUTTON_PIN, LAB61_DEBOUNCE_MS);
	printf("[Lab6.1] State -> %s\r\n",
				 g_state == LedState::On ? "ON" : "OFF");
}

void loop_lab6_1() {
	const State& current = kFsm[idx(g_state)];

	delay(current.holdMs);

	const bool pressed = readButtonPress();
	const LedState next = current.next[pressed ? 1 : 0];
	if (next != g_state) {
		g_state = next;
		applyOutput(g_state);
		printf("[Lab6.1] State -> %s\r\n",
					 g_state == LedState::On ? "ON" : "OFF");
	}
}
