#include "../emulated_pwm.h"

#include <iostream>
#include <cstdlib>
#include <sstream>
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
    std::cout << "Usage: runtime_pwm [options]"                                         << std::endl
              << "Options:"                                                             << std::endl
              << "  --duty_cycle <0-100>     Duty cycle in % (default: 50.0)"           << std::endl
              << "  --period_ms <ms>         Period in milliseconds (default: 1.0 ms)"  << std::endl
              << "  --frequency_hz <Hz>      Frequency in Hz"                           << std::endl
              << "  --path <string>          GPIO chip path (default: /dev/gpiochip0)"  << std::endl
              << "  --gpio_line <uint16>     GPIO line number (default: 28)"            << std::endl
              << "  --help                   Show this help message"                    << std::endl
              << std::endl
              << "Runtime commands:"                                                    << std::endl
              << "  duty <0-100>             Change duty cycle"                         << std::endl
              << "  period <ms>              Change period in milliseconds"             << std::endl
              << "  freq <Hz>                Change frequency in Hz"                    << std::endl
              << "  exit                     Stop PWM and exit"                         << std::endl;

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

        std::cout << "PWM started:"                                     << std::endl
                  << "  Duty cycle = " << pwm.get_duty_cycle() << "%"   << std::endl
                  << "  Period     = " << pwm.get_period_ms() << " ms"  << std::endl
                  << "  GPIO chip  = " << path                          << std::endl
                  << "  GPIO line  = " << gpio_line                     << std::endl
                  << std::endl;

        std::atomic<bool> running(true);

        // Thread to read commands
        std::thread input_thread([&]() {
            std::string line;
            while(running.load()) {
                std::cout << "Command (duty <0-100>, period <ms>, freq <Hz>, exit): ";

                std::getline(std::cin, line);
                std::istringstream iss(line);
                std::string cmd;
                iss >> cmd;

                if(cmd == "duty") {
                    float new_duty;

                    if(iss >> new_duty) {
                        if (new_duty >= 0.0f && new_duty <= 100.0f) {
                            pwm.set_duty_cycle(new_duty);

                            std::cout << "Duty cycle updated to " << pwm.get_duty_cycle() << "%" << std::endl;
                        }
                        else
                            std::cout << "Error: Duty cycle must be between 0 and 100" << std::endl;
                    }
                    else
                        std::cout << "Error: Invalid duty cycle value" << std::endl;
                }
                else if(cmd == "period") {
                    float new_period;

                    if(iss >> new_period) {
                        if (new_period > 0.0f) {
                            pwm.set_period_ms(new_period);

                            std::cout << "Period updated to " << pwm.get_period_ms() << " ms" << std::endl;
                        }
                        else
                            std::cout << "Error: Period must be > 0" << std::endl;
                    }
                    else
                        std::cout << "Error: Invalid period value" << std::endl;
                }
                else if(cmd == "freq") {
                    float new_freq;

                    if(iss >> new_freq && new_freq > 0.0f) {
                        pwm.set_period_ms(1000.0f / new_freq);

                        std::cout << "Frequency updated to " << new_freq << " Hz (period " 
                                  << pwm.get_period_ms() << " ms)" << std::endl;
                    }
                    else
                        std::cout << "Error: Frequency must be > 0" << std::endl;
                }
                else if(cmd == "exit") {
                    running.store(false);

                    break;
                }
                else if(!cmd.empty())
                    std::cout << "Unknown command: " << cmd << ". Type 'exit' to quit." << std::endl;
            }
        });

        input_thread.join();
        pwm.disable();
        std::cout << "PWM stopped." << std::endl;
    }
    catch(const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;

        return 1;
    }

    return 0;
}
