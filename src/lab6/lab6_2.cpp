#include "lab6/lab6_2.hpp"

#include <stdio.h>

#include "CustomSTDIO.h"
#include "config.h"

enum class TrafficPhase { GoNorth = 0, WaitNorth, GoEast, WaitEast };

struct TrafficState {
	bool eastRed;
	bool eastYellow;
	bool eastGreen;
	bool northRed;
	bool northYellow;
	bool northGreen;
	unsigned long holdMs;
};

struct ButtonDebounce {
	bool stablePressed;
	bool lastRaw;
	unsigned long lastChangeMs;
};

// Explicit outputs per phase to avoid bit masks.
static const TrafficState kStates[] = {
		// GoNorth: North green, East red
		{true, false, false, false, false, true, LAB62_GO_N_TIME_MS},
		// WaitNorth: North yellow, East red
		{true, false, false, false, true, false, LAB62_WAIT_N_TIME_MS},
		// GoEast: East green, North red
		{false, false, true, true, false, false, LAB62_GO_E_TIME_MS},
		// WaitEast: East yellow, North red
		{false, true, false, true, false, false, LAB62_WAIT_E_TIME_MS},
};

static TrafficPhase g_state = TrafficPhase::GoNorth;
static ButtonDebounce g_northBtn{};
static ButtonDebounce g_eastBtn{};

static const char* stateName(TrafficPhase state) {
	switch (state) {
		case TrafficPhase::GoNorth: return "goN";
		case TrafficPhase::WaitNorth: return "waitN";
		case TrafficPhase::GoEast: return "goE";
		case TrafficPhase::WaitEast: return "waitE";
		default: return "?";
	}
}

static bool isNorthPressed() {
	// Buttons use pull-up, so LOW means pressed.
	return digitalRead(LAB62_NORTH_PIN) == LOW;
}

static bool isEastPressed() {
	return digitalRead(LAB62_EAST_PIN) == LOW;
}

static void initButton(ButtonDebounce& b, bool rawPressed) {
	b.stablePressed = rawPressed;
	b.lastRaw = rawPressed;
	b.lastChangeMs = millis();
}

static bool readButtonEdge(ButtonDebounce& b, uint8_t pin) {
	const bool rawPressed = digitalRead(pin) == LOW;
	const unsigned long now = millis();

	if (rawPressed != b.lastRaw) {
		b.lastRaw = rawPressed;
		b.lastChangeMs = now;
	}

	if ((now - b.lastChangeMs) >= BUTTON_DEBOUNCE_MS && rawPressed != b.stablePressed) {
		b.stablePressed = rawPressed;
		if (b.stablePressed) {
			return true;  // rising edge (button pressed)
		}
	}

	return false;
}

static void applyOutput(const TrafficState& out) {
	digitalWrite(LAB62_EAST_RED_PIN, out.eastRed ? HIGH : LOW);
	digitalWrite(LAB62_EAST_YELLOW_PIN, out.eastYellow ? HIGH : LOW);
	digitalWrite(LAB62_EAST_GREEN_PIN, out.eastGreen ? HIGH : LOW);

	digitalWrite(LAB62_NORTH_RED_PIN, out.northRed ? HIGH : LOW);
	digitalWrite(LAB62_NORTH_YELLOW_PIN, out.northYellow ? HIGH : LOW);
	digitalWrite(LAB62_NORTH_GREEN_PIN, out.northGreen ? HIGH : LOW);
}

void setup_lab6_2() {
	StdioSerialSetup();

	pinMode(LAB62_NORTH_PIN, INPUT_PULLUP);
	pinMode(LAB62_EAST_PIN, INPUT_PULLUP);

	pinMode(LAB62_EAST_RED_PIN, OUTPUT);
	pinMode(LAB62_EAST_YELLOW_PIN, OUTPUT);
	pinMode(LAB62_EAST_GREEN_PIN, OUTPUT);

	pinMode(LAB62_NORTH_RED_PIN, OUTPUT);
	pinMode(LAB62_NORTH_YELLOW_PIN, OUTPUT);
	pinMode(LAB62_NORTH_GREEN_PIN, OUTPUT);

	initButton(g_northBtn, isNorthPressed());
	initButton(g_eastBtn, isEastPressed());

	applyOutput(kStates[static_cast<int>(g_state)]);
	printf("[Lab6.2] Traffic light FSM ready.\r\n");
	printf("North btn: D%d, East btn: D%d\r\n", LAB62_NORTH_PIN, LAB62_EAST_PIN);
	printf("[Lab6.2] State -> %s\r\n", stateName(g_state));
}

void loop_lab6_2() {
	const TrafficState& current = kStates[static_cast<int>(g_state)];

	applyOutput(current);

	// Poll buttons frequently during the dwell time so short presses are not missed.
	const unsigned long startMs = millis();
	bool northEdge = false;
	bool eastEdge = false;
	while (millis() - startMs < current.holdMs) {
		northEdge |= readButtonEdge(g_northBtn, LAB62_NORTH_PIN);
		eastEdge |= readButtonEdge(g_eastBtn, LAB62_EAST_PIN);
		delay(5);
	}

	printf("[Lab6.2] Hold %s | IN north=%d east=%d\r\n",
				 stateName(g_state), g_northBtn.stablePressed ? 1 : 0,
				 g_eastBtn.stablePressed ? 1 : 0);

	TrafficPhase next = g_state;
	switch (g_state) {
		case TrafficPhase::GoNorth:
			if (northEdge || eastEdge) next = TrafficPhase::WaitNorth;
			break;
		case TrafficPhase::WaitNorth:
			next = TrafficPhase::GoEast;
			break;
		case TrafficPhase::GoEast:
			if (northEdge) next = TrafficPhase::WaitEast;
			break;
		case TrafficPhase::WaitEast:
			next = TrafficPhase::GoNorth;
			break;
	}

	if (next != g_state) {
		g_state = next;
		applyOutput(kStates[static_cast<int>(g_state)]);
		printf("[Lab6.2] State -> %s\r\n", stateName(g_state));
	}
}
