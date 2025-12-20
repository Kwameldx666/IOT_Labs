#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>

/**
 * @brief Простое управление DC‑мотором через драйвер типа L293D/L298.
 *
 * Реализован чистый C‑интерфейс с handle, чтобы можно было
 * создавать несколько моторов без глобального состояния.
 */

typedef struct {
	uint8_t enablePin;
	uint8_t pin1;
	uint8_t pin2;
	bool initialized;
	int currentPercent;  // -100..100
	int currentPwm;      // -255..255
} MotorHandle;

// Инициализация пинов под мотор (ENA + IN1/IN2)
void motor_handle_init(MotorHandle* handle, uint8_t enablePin, uint8_t pin1, uint8_t pin2);

// Установка скорости в процентах (-100..100) с преобразованием в PWM
void motor_handle_set_percent(MotorHandle* handle, int percent);

// Установка сырых PWM значений (-255..255)
void motor_handle_set_pwm(MotorHandle* handle, int pwm);

// Остановка мотора (эквивалент PWM=0)
void motor_handle_stop(MotorHandle* handle);

// Текущее состояние
int motor_handle_get_percent(const MotorHandle* handle);
int motor_handle_get_pwm(const MotorHandle* handle);

// ----------------------------------------------------------------------------
// Совместимость с текущими лабами (single-motor wrappers)
// ----------------------------------------------------------------------------
void motorBegin(uint8_t enablePin, uint8_t pin1, uint8_t pin2);
void motorSetPercent(int percent);
int motorGetPercent();
void motorSetup(int enablePin, int pin1, int pin2);  // alias
void setSpeed(int speed);                            // alias for PWM
int getSpeed();                                      // alias for PWM

#endif
