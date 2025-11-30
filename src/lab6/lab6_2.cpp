#include "lab6/lab6_2.hpp"

#include <stdio.h>

#include "CustomSTDIO.h"
#include "config.h"

namespace {

struct State {
	unsigned long out;
	unsigned long timeMs;
	unsigned long next[4];
};

constexpr State kFsm[] = {
		{LAB62_GO_N_OUTPUT, LAB62_GO_N_TIME_MS,
		 {LAB62_STATE_GO_N, LAB62_STATE_WAIT_N, LAB62_STATE_WAIT_N, LAB62_STATE_WAIT_N}},
		{LAB62_WAIT_N_OUTPUT, LAB62_WAIT_N_TIME_MS,
		 {LAB62_STATE_GO_E, LAB62_STATE_GO_E, LAB62_STATE_GO_E, LAB62_STATE_GO_E}},
		{LAB62_GO_E_OUTPUT, LAB62_GO_E_TIME_MS,
		 {LAB62_STATE_GO_E, LAB62_STATE_GO_E, LAB62_STATE_WAIT_E, LAB62_STATE_WAIT_E}},
		{LAB62_WAIT_E_OUTPUT, LAB62_WAIT_E_TIME_MS,
		 {LAB62_STATE_GO_N, LAB62_STATE_GO_N, LAB62_STATE_GO_N, LAB62_STATE_GO_N}},
};

int g_state = LAB62_STATE_GO_N;

const char* stateName(int state) {
	switch (state) {
		case LAB62_STATE_GO_N: return "goN";
		case LAB62_STATE_WAIT_N: return "waitN";
		case LAB62_STATE_GO_E: return "goE";
		case LAB62_STATE_WAIT_E: return "waitE";
		default: return "?";
	}
}

int readInput() {
	const int north = digitalRead(LAB62_NORTH_PIN) ? 0b10 : 0;
	const int east = digitalRead(LAB62_EAST_PIN) ? 0b01 : 0;
	return north | east;
}

void setOutput(unsigned long out) {
	digitalWrite(LAB62_EAST_RED_PIN, (out & (1 << 5)) ? HIGH : LOW);
	digitalWrite(LAB62_EAST_YELLOW_PIN, (out & (1 << 4)) ? HIGH : LOW);
	digitalWrite(LAB62_EAST_GREEN_PIN, (out & (1 << 3)) ? HIGH : LOW);
	digitalWrite(LAB62_NORTH_RED_PIN, (out & (1 << 2)) ? HIGH : LOW);
	digitalWrite(LAB62_NORTH_YELLOW_PIN, (out & (1 << 1)) ? HIGH : LOW);
	digitalWrite(LAB62_NORTH_GREEN_PIN, (out & (1 << 0)) ? HIGH : LOW);
}

}  // namespace

void setup_lab6_2() {
	StdioSerialSetup();

	pinMode(LAB62_NORTH_PIN, INPUT);
	pinMode(LAB62_EAST_PIN, INPUT);

	pinMode(LAB62_EAST_RED_PIN, OUTPUT);
	pinMode(LAB62_EAST_YELLOW_PIN, OUTPUT);
	pinMode(LAB62_EAST_GREEN_PIN, OUTPUT);

	pinMode(LAB62_NORTH_RED_PIN, OUTPUT);
	pinMode(LAB62_NORTH_YELLOW_PIN, OUTPUT);
	pinMode(LAB62_NORTH_GREEN_PIN, OUTPUT);

	setOutput(kFsm[g_state].out);
	printf("[Lab6.2] Traffic light FSM ready.\r\n");
	printf("North btn: D%d, East btn: D%d\r\n", LAB62_NORTH_PIN, LAB62_EAST_PIN);
	printf("[Lab6.2] State -> %s\r\n", stateName(g_state));
}

void loop_lab6_2() {
	const State& current = kFsm[g_state];

	setOutput(current.out);
	delay(current.timeMs);

	const int input = readInput();  // bits: north<<1 | east
	const int next = current.next[input & 0b11];

	printf("[Lab6.2] Hold %s | IN north=%d east=%d\r\n", stateName(g_state),
				 (input & 0b10) ? 1 : 0, (input & 0b01) ? 1 : 0);

	if (next != g_state) {
		g_state = next;
		printf("[Lab6.2] State -> %s\r\n", stateName(g_state));
	}
}
