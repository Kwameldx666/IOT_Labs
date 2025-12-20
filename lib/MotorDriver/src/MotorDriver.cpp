#include "MotorDriver.h"

static void motor_write_outputs(MotorHandle* handle, int pwm) {
	if (!handle || !handle->initialized) {
		return;
	}

	if (pwm == 0) {
		digitalWrite(handle->enablePin, LOW);
		digitalWrite(handle->pin1, LOW);
		digitalWrite(handle->pin2, LOW);
		return;
	}

	const bool forward = (pwm > 0);
	digitalWrite(handle->pin1, forward ? HIGH : LOW);
	digitalWrite(handle->pin2, forward ? LOW : HIGH);
	analogWrite(handle->enablePin, abs(pwm));
}

void motor_handle_init(MotorHandle* handle, uint8_t enablePin, uint8_t pin1, uint8_t pin2) {
	if (!handle) return;

	handle->enablePin = enablePin;
	handle->pin1 = pin1;
	handle->pin2 = pin2;
	handle->initialized = true;
	handle->currentPercent = 0;
	handle->currentPwm = 0;

	pinMode(handle->enablePin, OUTPUT);
	pinMode(handle->pin1, OUTPUT);
	pinMode(handle->pin2, OUTPUT);
	digitalWrite(handle->enablePin, LOW);
	digitalWrite(handle->pin1, LOW);
	digitalWrite(handle->pin2, LOW);
}

void motor_handle_set_pwm(MotorHandle* handle, int pwm) {
	if (!handle || !handle->initialized) {
		return;
	}

	pwm = constrain(pwm, -255, 255);
	handle->currentPwm = pwm;
	handle->currentPercent = map(pwm, -255, 255, -100, 100);
	motor_write_outputs(handle, pwm);
}

void motor_handle_set_percent(MotorHandle* handle, int percent) {
	if (!handle || !handle->initialized) {
		return;
	}
	percent = constrain(percent, -100, 100);
	const int pwm = map(percent, -100, 100, -255, 255);
	motor_handle_set_pwm(handle, pwm);
	// синхронизируем процент после ограничений PWM
	handle->currentPercent = map(handle->currentPwm, -255, 255, -100, 100);
}

void motor_handle_stop(MotorHandle* handle) {
	motor_handle_set_pwm(handle, 0);
}

int motor_handle_get_percent(const MotorHandle* handle) {
	return (handle && handle->initialized) ? handle->currentPercent : 0;
}

int motor_handle_get_pwm(const MotorHandle* handle) {
	return (handle && handle->initialized) ? handle->currentPwm : 0;
}

// ----------------------------------------------------------------------------
// Совместимость с существующим кодом (один мотор)
// ----------------------------------------------------------------------------
static MotorHandle g_defaultMotor = {0, 0, 0, false, 0, 0};

void motorBegin(uint8_t enablePin, uint8_t pin1, uint8_t pin2) {
	motor_handle_init(&g_defaultMotor, enablePin, pin1, pin2);
}

void motorSetPercent(int percent) {
	motor_handle_set_percent(&g_defaultMotor, percent);
}

int motorGetPercent() {
	return motor_handle_get_percent(&g_defaultMotor);
}

void motorSetup(int enablePin, int pin1, int pin2) {
	motorBegin(enablePin, pin1, pin2);
}

void setSpeed(int speed) {
	motor_handle_set_pwm(&g_defaultMotor, speed);
}

int getSpeed() {
	return motor_handle_get_pwm(&g_defaultMotor);
}
