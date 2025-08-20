#ifndef EMULATED_PWM_H
#define EMULATED_PWM_H

#include <cstdint>
#include <atomic>
#include <string>

class Emulated_PWM {
	private:
		std::string path;
		uint16_t line_offset;

		std::atomic<float> period;
		std::atomic<float> duty_cycle;
		std::atomic<bool> enabled_flag;

	public:
		Emulated_PWM(const std::string &path_parameter, uint16_t line_offset_parameter);

		void set_period_ms(float period_ms_parameter);
		void set_period_hz(float period_hz_parameter);
		[[nodiscard]] float get_period_ms() const;
		[[nodiscard]] float get_period_hz() const;

		// Duty cycle
		void set_duty_cycle(float duty_cycle_parameter);
		[[nodiscard]] float get_duty_cycle() const;

		// Control
		void enable();
		void disable();
		[[nodiscard]] bool is_enabled() const;
};

#endif /* EMULATED_PWM_H */
