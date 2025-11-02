#include "pid_controller.hpp"

PIDController::PIDController(float kp, float ki, float kd, float outputMin, float outputMax)
    : m_kp(kp)
    , m_ki(ki)
    , m_kd(kd)
    , m_outputMin(outputMin)
    , m_outputMax(outputMax)
    , m_error(0.0f)
    , m_prevError(0.0f)
    , m_integral(0.0f)
    , m_pTerm(0.0f)
    , m_iTerm(0.0f)
    , m_dTerm(0.0f)
    , m_output(0.0f) {
}

float PIDController::compute(float setpoint, float measurement, float dt) {
  // Calculate error
  m_error = setpoint - measurement;
  
  // Proportional term
  m_pTerm = m_kp * m_error;
  
  // Integral term (with anti-windup)
  m_integral += m_error * dt;
  m_iTerm = m_ki * m_integral;
  
  // Derivative term
  float derivative = (m_error - m_prevError) / dt;
  m_dTerm = m_kd * derivative;
  
  // Compute output
  m_output = m_pTerm + m_iTerm + m_dTerm;
  
  // Clamp output to limits
  m_output = clamp(m_output, m_outputMin, m_outputMax);
  
  // Anti-windup: prevent integral buildup when saturated
  if (m_output == m_outputMin || m_output == m_outputMax) {
    // Back-calculate integral to prevent windup
    m_integral -= m_error * dt;
  }
  
  // Save error for next iteration
  m_prevError = m_error;
  
  return m_output;
}

void PIDController::setGains(float kp, float ki, float kd) {
  m_kp = kp;
  m_ki = ki;
  m_kd = kd;
}

void PIDController::setOutputLimits(float min, float max) {
  m_outputMin = min;
  m_outputMax = max;
}

void PIDController::reset() {
  m_error = 0.0f;
  m_prevError = 0.0f;
  m_integral = 0.0f;
  m_pTerm = 0.0f;
  m_iTerm = 0.0f;
  m_dTerm = 0.0f;
  m_output = 0.0f;
}

float PIDController::clamp(float value, float min, float max) {
  if (value < min) return min;
  if (value > max) return max;
  return value;
}
#ifndef PID_CONTROLLER_HPP
#define PID_CONTROLLER_HPP

#include <stdint.h>

/**
 * @file pid_controller.hpp
 * @brief PID Controller (Proportional-Integral-Derivative)
 * 
 * Classic PID control algorithm with:
 * - Proportional term (P): responds to current error
 * - Integral term (I): eliminates steady-state error
 * - Derivative term (D): predicts future error, reduces overshoot
 * 
 * Output = Kp*error + Ki*∫error*dt + Kd*(error-prev_error)/dt
 */

/**
 * @class PIDController
 * @brief PID control algorithm implementation
 */
class PIDController {
public:
  /**
   * @brief Constructor
   * @param kp Proportional gain
   * @param ki Integral gain
   * @param kd Derivative gain
   * @param outputMin Minimum output value
   * @param outputMax Maximum output value
   */
  PIDController(float kp, float ki, float kd, float outputMin, float outputMax);
  
  /**
   * @brief Compute PID output
   * @param setpoint Target value
   * @param measurement Current measured value
   * @param dt Time step since last update (seconds)
   * @return Control output (clamped to min/max)
   */
  float compute(float setpoint, float measurement, float dt);
  
  /**
   * @brief Set PID gains
   * @param kp Proportional gain
   * @param ki Integral gain
   * @param kd Derivative gain
   */
  void setGains(float kp, float ki, float kd);
  
  /**
   * @brief Set output limits
   * @param min Minimum output
   * @param max Maximum output
   */
  void setOutputLimits(float min, float max);
  
  /**
   * @brief Reset integral term (anti-windup)
   */
  void reset();
  
  /**
   * @brief Get proportional gain
   */
  float getKp() const { return m_kp; }
  
  /**
   * @brief Get integral gain
   */
  float getKi() const { return m_ki; }
  
  /**
   * @brief Get derivative gain
   */
  float getKd() const { return m_kd; }
  
  /**
   * @brief Get current error
   */
  float getError() const { return m_error; }
  
  /**
   * @brief Get proportional term
   */
  float getPTerm() const { return m_pTerm; }
  
  /**
   * @brief Get integral term
   */
  float getITerm() const { return m_iTerm; }
  
  /**
   * @brief Get derivative term
   */
  float getDTerm() const { return m_dTerm; }
  
  /**
   * @brief Get last computed output
   */
  float getOutput() const { return m_output; }

private:
  // PID gains
  float m_kp;
  float m_ki;
  float m_kd;
  
  // Output limits
  float m_outputMin;
  float m_outputMax;
  
  // Internal state
  float m_error;
  float m_prevError;
  float m_integral;
  
  // PID terms (for debugging)
  float m_pTerm;
  float m_iTerm;
  float m_dTerm;
  float m_output;
  
  float clamp(float value, float min, float max);
};

#endif // PID_CONTROLLER_HPP

