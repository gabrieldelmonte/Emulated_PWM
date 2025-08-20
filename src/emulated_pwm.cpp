#include <algorithm>
#include <pthread.h>
#include <stdexcept>
#include <gpiod.h>
#include <chrono>
#include <thread>

#include "emulated_pwm.h"

static float clamp(float value, float minimum_value, float maximum_value) {
	return std::max(minimum_value, std::min(value, maximum_value));
}

Emulated_PWM::Emulated_PWM(const std::string &path_parameter, uint16_t line_offset_parameter) :
	path(path_parameter),
	line_offset(line_offset_parameter),
	period(0.0f),
	duty_cycle(0.0f),
	enabled_flag(false)
	{
	//
}

void Emulated_PWM::set_period_ms(float period_ms_parameter) {
	if (period_ms_parameter <= 0.0f)
		throw std::invalid_argument("Period must be greater than 0!");

	period.store(period_ms_parameter);
}

void Emulated_PWM::set_period_hz(float period_hz_parameter) {
	if (period_hz_parameter <= 0.0f)
		throw std::invalid_argument("Frequency must be greater than 0!");

	period.store(1000.0f / period_hz_parameter);
}

float Emulated_PWM::get_period_ms() const {
	return period.load();
}

float Emulated_PWM::get_period_hz() const {
	return (1000.0f / period.load());
}

void Emulated_PWM::set_duty_cycle(float duty_cycle_parameter) {
	duty_cycle.store(clamp(duty_cycle_parameter, 0.0f, 100.0f));
}

float Emulated_PWM::get_duty_cycle() const {
	return duty_cycle.load();
}

void Emulated_PWM::enable() {
	if (enabled_flag.load())
		return;

	enabled_flag.store(true);

	gpiod_chip *chip = gpiod_chip_open(path.c_str());
	if (!chip)
		throw std::runtime_error("Failed to open the desired chip!");

	gpiod_line *line = gpiod_chip_get_line(chip, line_offset);
	if (!line) {
		gpiod_chip_close(chip);

		throw std::runtime_error("Failed to get GPIO line!");
	}

	if (gpiod_line_request_output(line, "emulated_pwm_gpio", 0) < 0) {
		gpiod_chip_close(chip);

		throw std::runtime_error("Failed to request line as output!");
	}

	std::thread([line, this]() {
		struct sched_param sch_params;

        // NOTE: The line below sets the thread to the HIGHEST real-time priority (99)
        // It requires root or CAP_SYS_NICE privileges
        // USE WITH EXTREME CAUTION: a tight loop without sleeps/yields may freeze the system!
		sch_params.sched_priority = 99;
		pthread_setschedparam(pthread_self(), SCHED_FIFO, &sch_params);

		while (enabled_flag.load()) {
			float current_period = period.load();
			float current_duty_cycle = duty_cycle.load();

			auto high_time = std::chrono::microseconds(static_cast<int>(current_period * (current_duty_cycle / 100.0f) * 1000.0f));
			auto low_time = std::chrono::microseconds(static_cast<int>((current_period * 1000.0f) - high_time.count()));

			gpiod_line_set_value(line, 1);
			std::this_thread::sleep_for(high_time);

			gpiod_line_set_value(line, 0);
			std::this_thread::sleep_for(low_time);
		}

		gpiod_line_set_value(line, 0);
		gpiod_line_release(line);
		gpiod_chip_close(gpiod_chip_open(path.c_str()));
	}).detach();
}

void Emulated_PWM::disable() {
	enabled_flag.store(false);
}

bool Emulated_PWM::is_enabled() const {
	return enabled_flag.load();
}
