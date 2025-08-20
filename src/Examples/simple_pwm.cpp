#include "../emulated_pwm.h"

#include <iostream>
#include <cstdlib>
#include <limits>

float parse_float(int argc, char* argv[], const std::string& key, bool &found) {
    for (int i = 1; i < argc; ++i)
        if (key == argv[i] && i + 1 < argc) {
            found = true;

            return std::stof(argv[i + 1]);
        }
    found = false;

    return 0.0f;
}

std::string parse_string(int argc, char* argv[], const std::string& key, const std::string& default_val) {
    for (int i = 1; i < argc; ++i)
        if (key == argv[i] && i + 1 < argc)
            return std::string(argv[i + 1]);

    return default_val;
}

uint16_t parse_uint16(int argc, char* argv[], const std::string& key, uint16_t default_val) {
    for (int i = 1; i < argc; ++i)
        if (key == argv[i] && i + 1 < argc) {
            int val = std::stoi(argv[i + 1]);
            if (val < 0 || val > std::numeric_limits<uint16_t>::max())
                throw std::out_of_range("gpio_line must be between 0 and 65535");

            return static_cast<uint16_t>(val);
        }

    return default_val;
}

void print_help() {
    std::cout << "Usage: pwm_app [options]"                                             << std::endl;
              << "Options:"                                                             << std::endl;
              << "  --duty_cycle <0-100>     Duty cycle in % (default: 50.0)"           << std::endl;
              << "  --period_ms <ms>         Period in milliseconds (default: 1.0 ms)"  << std::endl;
              << "  --frequency_hz <Hz>      Frequency in Hz"                           << std::endl;
              << "  --path <string>          GPIO chip path (default: /dev/gpiochip0)"  << std::endl;
              << "  --gpio_line <uint16>     GPIO line number (default: 28)"            << std::endl;
              << "  --help                   Show this help message";                   << std::endl;

    return;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--help") {
            print_help();

            return 0;
        }

    try {
        // Duty cycle
        bool dummy = false;
        float duty = parse_float(argc, argv, "--duty_cycle", dummy);
        if (duty < 0.0f || duty > 100.0f)
            duty = 50.0f;

        // Period or frequency
        bool period_ms_found = false;
        bool freq_hz_found = false;
        float period_ms = parse_float(argc, argv, "--period_ms", period_ms_found);
        float frequency_hz = parse_float(argc, argv, "--frequency_hz", freq_hz_found);

        if (period_ms_found && freq_hz_found)
            throw std::invalid_argument("Use either --period_ms or --frequency_hz, not both.");

        if (freq_hz_found) {
            if (frequency_hz <= 0.0f)
                throw std::invalid_argument("Frequency must be > 0");

            period_ms = 1000.0f / frequency_hz;
        }

        if (!period_ms_found && !freq_hz_found)
            period_ms = 1.0f;

        // GPIO chip path and line
        std::string path = parse_string(argc, argv, "--path", "/dev/gpiochip0");
        uint16_t gpio_line = parse_uint16(argc, argv, "--gpio_line", 28);

        // Initialize PWM
        Emulated_PWM pwm(path, gpio_line);
        pwm.set_duty_cycle(duty);
        pwm.set_period_ms(period_ms);

        pwm.enable();

        std::cout << "PWM started:"                                     << std::endl;
                  << "  Duty cycle = " << pwm.get_duty_cycle() << "%"   << std::endl;
                  << "  Period     = " << pwm.get_period_ms() << " ms"  << std::endl;
                  << "  GPIO chip  = " << path                          << std::endl;
                  << "  GPIO line  = " << gpio_line                     << std::endl;

        std::cout << "Press Enter to stop..." << std::endl;
        std::cin.get();

        pwm.disable();
        std::cout << "PWM stopped." << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;

        return 1;
    }

    return 0;
}
