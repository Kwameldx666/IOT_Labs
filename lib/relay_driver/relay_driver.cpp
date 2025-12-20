#include "relay_driver.hpp"
#include <Arduino.h>

// ============================================================================
// Hardware Abstraction Layer (HAL) Implementation
// ============================================================================

static void RelayHAL_initPin(uint8_t pin) {
	pinMode(pin, OUTPUT);
}

static void RelayHAL_setPinState(uint8_t pin, bool state) {
	digitalWrite(pin, state ? HIGH : LOW);
}

namespace RelayHAL {
void initPin(uint8_t pin) { RelayHAL_initPin(pin); }
void setPinState(uint8_t pin, bool state) { RelayHAL_setPinState(pin, state); }
bool getPinState(uint8_t pin) { return digitalRead(pin) == HIGH; }
}  // namespace RelayHAL

// ============================================================================
// Driver Layer Implementation (class)
// ============================================================================

RelayDriver::RelayDriver(uint8_t pin, bool activeHigh)
    : m_pin(pin)
    , m_activeHigh(activeHigh)
    , m_state(RelayState::OFF) {}

void RelayDriver::begin() {
	RelayHAL_initPin(m_pin);
	turnOff();  // Safe default state
}

void RelayDriver::turnOn() {
	m_state = RelayState::ON;
	updateHardware();
}

void RelayDriver::turnOff() {
	m_state = RelayState::OFF;
	updateHardware();
}

void RelayDriver::toggle() {
	if (m_state == RelayState::ON) {
		turnOff();
	} else {
		turnOn();
	}
}

void RelayDriver::setState(RelayState state) {
	if (state == RelayState::ON) {
		turnOn();
	} else {
		turnOff();
	}
}

void RelayDriver::updateHardware() {
	bool pinState;

	if (m_activeHigh) {
		// Active-high: ON = HIGH, OFF = LOW
		pinState = (m_state == RelayState::ON);
	} else {
		// Active-low: ON = LOW, OFF = HIGH
		pinState = (m_state == RelayState::OFF);
	}

	RelayHAL_setPinState(m_pin, pinState);
}

// ============================================================================
// C-style API for single relay usage
// ============================================================================

static void relay_apply_state_handle(RelayHandle* handle) {
	if (!handle || !handle->initialized) {
		return;
	}

	bool pinState = handle->activeHigh ? (handle->state == RelayState::ON)
																		 : (handle->state == RelayState::OFF);
	RelayHAL_setPinState(handle->pin, pinState);
}

void relay_handle_init(RelayHandle* handle, uint8_t pin, bool activeHigh) {
	if (!handle) return;
	handle->pin = pin;
	handle->activeHigh = activeHigh;
	handle->state = RelayState::OFF;
	handle->initialized = true;
	RelayHAL_initPin(pin);
	relay_handle_off(handle);
}

void relay_handle_on(RelayHandle* handle) {
	if (!handle || !handle->initialized) return;
	handle->state = RelayState::ON;
	relay_apply_state_handle(handle);
}

void relay_handle_off(RelayHandle* handle) {
	if (!handle || !handle->initialized) return;
	handle->state = RelayState::OFF;
	relay_apply_state_handle(handle);
}

void relay_handle_toggle(RelayHandle* handle) {
	if (!handle || !handle->initialized) return;
	handle->state = (handle->state == RelayState::ON) ? RelayState::OFF : RelayState::ON;
	relay_apply_state_handle(handle);
}

bool relay_handle_is_on(const RelayHandle* handle) {
	return handle && handle->initialized && (handle->state == RelayState::ON);
}

RelayState relay_handle_state(const RelayHandle* handle) {
	if (!handle || !handle->initialized) return RelayState::OFF;
	return handle->state;
}

// Global single-handle helpers (compatible with labs 4)
static RelayHandle g_defaultRelay = {0, true, RelayState::OFF, false};

void relay_init(uint8_t pin, bool activeHigh) {
	relay_handle_init(&g_defaultRelay, pin, activeHigh);
}

void relay_on() { relay_handle_on(&g_defaultRelay); }
void relay_off() { relay_handle_off(&g_defaultRelay); }
void relay_toggle() { relay_handle_toggle(&g_defaultRelay); }
bool relay_is_on() { return relay_handle_is_on(&g_defaultRelay); }
RelayState relay_state() { return relay_handle_state(&g_defaultRelay); }
