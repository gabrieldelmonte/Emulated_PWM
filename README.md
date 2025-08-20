# Emulated PWM Library

A C++ library for generating software-emulated PWM (Pulse Width Modulation) signals on Linux systems using GPIO pins through the `libgpiod` library. This library provides precise timing control for PWM signals when hardware PWM is not available or when you need more flexibility in PWM generation.

## Features

- **Software-based PWM generation** using high-resolution timers
- **Configurable duty cycle** (0-100%)
- **Flexible frequency/period control** (supports both Hz and milliseconds)
- **GPIO chip and line selection** for different hardware configurations
- **Thread-safe operation** with atomic controls
- **Real-time parameter adjustment** in runtime examples
- **Comprehensive error handling** and validation

## Requirements

- **Linux system** with GPIO support
- **libgpiod** development library
- **C++17 compatible compiler** (GCC 7+ or Clang 5+)
- **pthread** library for threading support

### Installing Dependencies

On Ubuntu/Debian:
```bash
sudo apt update
sudo apt install build-essential libgpiod-dev
```

## Project Structure

```
Emulated_PWM/
├── src/
│   ├── emulated_pwm.h          # Main library header
│   ├── emulated_pwm.cpp        # Main library implementation
│   └── Examples/
│       ├── simple_pwm.cpp      # Basic PWM example
│       └── runtime_pwm.cpp     # Interactive PWM control
├── LICENSE
└── README.md
```

## Library API

### Core Class: `Emulated_PWM`

```cpp
class Emulated_PWM {
public:
    // Constructor
    Emulated_PWM(const std::string& chip_path, uint16_t gpio_line);
    
    // Configuration methods
    void set_duty_cycle(float duty_percent);
    void set_period_ms(float period_ms);
    
    // Control methods
    void enable();
    void disable();
    
    // Status methods
    float get_duty_cycle() const;
    float get_period_ms() const;
    bool is_enabled() const;
};
```

### Basic Usage Example

```cpp
#include "emulated_pwm.h"

int main() {
    // Create PWM instance on GPIO chip 0, line 28
    Emulated_PWM pwm("/dev/gpiochip0", 28);
    
    // Configure PWM: 75% duty cycle, 2ms period (500 Hz)
    pwm.set_duty_cycle(75.0f);
    pwm.set_period_ms(2.0f);
    
    // Start PWM
    pwm.enable();
    
    // PWM runs in background...
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // Stop PWM
    pwm.disable();
    
    return 0;
}
```

## Examples

The repository includes two comprehensive examples demonstrating different use cases:

### 1. Simple PWM (`simple_pwm.cpp`)

A basic example that starts a PWM signal with specified parameters and runs until user input.

#### Compilation
```bash
cd src/Examples/
g++ -std=c++17 -O2 -pthread ../emulated_pwm.cpp simple_pwm.cpp -lgpiod -o simple_pwm
```

#### Usage
```bash
# Show help
sudo ./simple_pwm --help

# Using period in milliseconds
sudo ./simple_pwm --duty_cycle 55.25 --period_ms 2.5 --path /dev/gpiochip0 --gpio_line 28

# Using frequency in Hz
sudo ./simple_pwm --duty_cycle 60.0 --frequency_hz 500 --path /dev/gpiochip0 --gpio_line 28

# Minimal usage (uses defaults: 50% duty, 1ms period, GPIO 28)
sudo ./simple_pwm
```

#### Command Line Options
- `--duty_cycle <0-100>`: Duty cycle percentage (default: 50.0)
- `--period_ms <ms>`: Period in milliseconds (default: 1.0)
- `--frequency_hz <Hz>`: Frequency in Hz (alternative to period)
- `--path <string>`: GPIO chip path (default: /dev/gpiochip0)
- `--gpio_line <uint16>`: GPIO line number (default: 28)
- `--help`: Show help message

### 2. Runtime PWM (`runtime_pwm.cpp`)

An interactive example that allows real-time adjustment of PWM parameters during execution.

#### Compilation
```bash
cd src/Examples/
g++ -std=c++17 -O2 -pthread ../emulated_pwm.cpp runtime_pwm.cpp -lgpiod -o runtime_pwm
```

#### Usage
```bash
# Using period in milliseconds
sudo ./runtime_pwm --duty_cycle 55.25 --period_ms 2.5 --path /dev/gpiochip0 --gpio_line 28

# Using frequency in Hz
sudo ./runtime_pwm --duty_cycle 60.0 --frequency_hz 500 --path /dev/gpiochip0 --gpio_line 28

# Start with defaults and adjust in runtime
sudo ./runtime_pwm
```

#### Runtime Commands
Once running, you can use these interactive commands:

- `duty <0-100>`: Change duty cycle
  ```
  duty 75.5
  ```
- `period <ms>`: Change period in milliseconds
  ```
  period 5.0
  ```
- `freq <Hz>`: Change frequency in Hz
  ```
  freq 1000
  ```
- `info`: Display current PWM configuration
  ```
  info
  ```
- `exit`: Stop PWM and exit

#### Example Interactive Session
```
$ sudo ./runtime_pwm --duty_cycle 50 --frequency_hz 100

PWM started:
  Duty cycle = 50%
  Period     = 10 ms
  GPIO chip  = /dev/gpiochip0
  GPIO line  = 28

Command (duty <0-100>, period <ms>, freq <Hz>, info, exit): duty 75
Duty cycle updated to 75%

Command (duty <0-100>, period <ms>, freq <Hz>, info, exit): freq 500
Frequency updated to 500 Hz (period 2 ms)

Command (duty <0-100>, period <ms>, freq <Hz>, info, exit): info
Current PWM configuration:
  Duty cycle = 75%
  Period     = 2 ms
  Frequency  = 500 Hz
  GPIO chip  = /dev/gpiochip0
  GPIO line  = 28
  Status     = ENABLED

Command (duty <0-100>, period <ms>, freq <Hz>, info, exit): exit
PWM stopped!
```

## Hardware Setup

### GPIO Pin Configuration

1. **Identify your GPIO chip**: List available GPIO chips
   ```bash
   gpiodetect
   ```

2. **Check GPIO lines**: List lines for a specific chip
   ```bash
   gpioinfo gpiochip0
   ```

### Common GPIO Chip Paths
- Raspberry Pi: `/dev/gpiochip0`
- BeagleBone: `/dev/gpiochip0`, `/dev/gpiochip1`
- Generic Linux: `/dev/gpiochipX` (where X is the chip number)

## Performance Considerations

- **Timing Precision**: The library uses high-resolution timers for accurate timing
- **CPU Usage**: Software PWM consumes more CPU than hardware PWM
- **Real-time Priority**: For best performance, consider running with elevated priority:
  ```bash
  sudo nice -n -20 ./your_pwm_app
  ```
- **Frequency Limits**: Practical upper limit depends on system load and timing requirements

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -m 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Open a Pull Request

## License

This project is licensed under the GNU AFFERO GENERAL PUBLIC LICENSE - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built using the excellent [libgpiod](https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git/) library
- Inspired by the need for flexible software PWM solutions on embedded Linux systems
