#ifndef LAB5_2_HPP
#define LAB5_2_HPP

/**
 * @file lab5_2.hpp
 * @brief Lab 5.2 - Control Comparison: ON-OFF vs PID
 * 
 * Dual control system demonstration:
 * a) ON-OFF control with hysteresis (temperature/humidity)
 * b) PID control for motor speed with encoder feedback
 * 
 * Features:
 * - Mode switching between ON-OFF and PID
 * - Setpoint adjustment via Serial/buttons
 * - LCD display with controller status
 * - Serial Plotter for performance comparison
 */

void setup_lab5_2();
void loop_lab5_2();

#endif // LAB5_2_HPP

