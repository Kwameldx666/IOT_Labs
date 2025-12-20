#ifndef RELAY_DRIVER_HPP
#define RELAY_DRIVER_HPP

#include <stdint.h>

/**
 * @file relay_driver.hpp
 * @brief Reusable Relay Driver with Layered Architecture
 *
 * Three-layer architecture:
 * - Hardware Abstraction Layer (HAL): pinMode, digitalWrite
 * - Driver Layer: Relay control logic
 * - Application Layer: High-level interface
 */

// ============================================================================
// Hardware Abstraction Layer (HAL)
// ============================================================================

namespace RelayHAL {
	/**
	 * @brief Initialize GPIO pin for relay
	 * @param pin Pin number
	 */
	void initPin(uint8_t pin);

	/**
	 * @brief Set relay pin state
	 * @param pin Pin number
	 * @param state true = HIGH, false = LOW
	 */
	void setPinState(uint8_t pin, bool state);

	/**
	 * @brief Read relay pin state
	 * @param pin Pin number
	 * @return Current pin state
	 */
	bool getPinState(uint8_t pin);
}

// ============================================================================
// Driver Layer
// ============================================================================

enum class RelayState {
	OFF = 0,
	ON = 1
};

/**
 * @class RelayDriver
 * @brief Low-level relay control driver
 */
class RelayDriver {
public:
	/**
	 * @brief Constructor
	 * @param pin Control pin number
	 * @param activeHigh true if relay is active-high, false if active-low
	 */
	explicit RelayDriver(uint8_t pin, bool activeHigh = true);

	/**
	 * @brief Initialize relay hardware
	 */
	void begin();

	/**
	 * @brief Turn relay on
	 */
	void turnOn();

	/**
	 * @brief Turn relay off
	 */
	void turnOff();

	/**
	 * @brief Toggle relay state
	 */
	void toggle();

	/**
	 * @brief Set relay state
	 * @param state Desired relay state
	 */
	void setState(RelayState state);

	/**
	 * @brief Get current relay state
	 * @return Current state
	 */
	RelayState getState() const { return m_state; }

	/**
	 * @brief Check if relay is on
	 * @return true if relay is on
	 */
	bool isOn() const { return m_state == RelayState::ON; }

private:
	uint8_t m_pin;
	bool m_activeHigh;
	RelayState m_state;

	void updateHardware();
};

// ----------------------------------------------------------------------------
// C-style API for single relay usage (pure C interface for labs)
// ----------------------------------------------------------------------------

/**
 * @brief Initialize relay with given pin and polarity (activeHigh = true by default)
 */
void relay_init(uint8_t pin, bool activeHigh = true);

/**
 * @brief Turn relay on
 */
void relay_on();

/**
 * @brief Turn relay off
 */
void relay_off();

/**
 * @brief Toggle relay state
 */
void relay_toggle();

/**
 * @brief Check if relay is currently on
 */
bool relay_is_on();

/**
 * @brief Get current relay state
 */
RelayState relay_state();

// ----------------------------------------------------------------------------
// C-style API with explicit handle (multi-relay)
// ----------------------------------------------------------------------------
struct RelayHandle {
	uint8_t pin;
	bool activeHigh;
	RelayState state;
	bool initialized;
};

void relay_handle_init(RelayHandle* handle, uint8_t pin, bool activeHigh = true);
void relay_handle_on(RelayHandle* handle);
void relay_handle_off(RelayHandle* handle);
void relay_handle_toggle(RelayHandle* handle);
bool relay_handle_is_on(const RelayHandle* handle);
RelayState relay_handle_state(const RelayHandle* handle);

#endif // RELAY_DRIVER_HPP
