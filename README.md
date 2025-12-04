# STM32 Digital Practice Pad with Metronome

A real-time embedded C/C++ application for STM32F446xx (Nucleo board) that transforms piezo sensors into a digital practice pad with integrated metronome and strike timing analysis.

## Features

- **Multi-Sensor Input**: Reads 5 piezo sensors via ADC with DMA for drum strike detection
- **Hardware Metronome**: Timer-based metronome with configurable BPM (40-208)
- **Strike Timing Analysis**: Real-time accuracy measurement comparing strikes against metronome beats
- **Visual Feedback**: LED indication synchronized with metronome ticks
- **Missed Beat Detection**: Tracks and reports practice session statistics
- **UART Communication**: Serial output for timing data and analytics
- **FreeRTOS Integration**: Multi-tasking architecture for responsive real-time performance

## Hardware Requirements

- **Board**: STM32 Nucleo-F446RE (or compatible STM32F446xx)
- **Sensors**: 5 piezo sensors connected to ADC channels (PA0, PA1, PB0, PC0, PC1)
- **Optional**: LED for metronome visual feedback
- **Communication**: USB serial (USART2) for data output

## Quick Start

### Prerequisites

- ARM GCC toolchain (`arm-none-eabi-gcc`)
- CMake 3.22 or higher
- Ninja build system (recommended)
- ST-Link tools for flashing

### Build

```bash
# Configure the project (Debug preset)
cmake --preset Debug

# Build
cmake --build build/Debug

# Or use Ninja directly
ninja -C build/Debug
```

### Available Build Presets

- `Debug`: Development build with debugging symbols
- `Release`: Optimized production build
- `RelWithDebInfo`: Release with debug information
- `MinSizeRel`: Size-optimized build

### Flash to Board

```bash
# Using ST-Link
st-flash write build/Debug/practice_pad_nucleo_board_cpp.bin 0x8000000
```

## Project Structure

```
.
├── Core/
│   ├── Inc/              # Application headers
│   │   ├── Metronome.hpp
│   │   ├── CircularBuffer.hpp
│   │   ├── SessionManager.hpp
│   │   ├── ExerciseLibrary.hpp
│   │   └── ...
│   └── Src/              # Application implementation
│       ├── cpp_main.cpp  # Main C++ entry point
│       ├── Metronome.cpp
│       └── ...
├── Drivers/              # STM32 HAL and CMSIS
├── Middlewares/          # FreeRTOS source
├── cmake/                # Build configuration
└── CMakeLists.txt
```

## Architecture

### Core Components

**Metronome System** ([Metronome.hpp](Core/Inc/Metronome.hpp))
- Hardware timer-based (TIM3) for precise timing
- Configurable BPM with automatic period calculation
- Provides timing reference for accuracy measurement

**Circular Buffer** ([CircularBuffer.hpp](Core/Inc/CircularBuffer.hpp))
- Thread-safe template-based buffer for sensor data
- Lock-free operation for real-time performance

**Session Manager** ([SessionManager.hpp](Core/Inc/SessionManager.hpp))
- Tracks practice sessions with comprehensive statistics
- Missed beat detection and accuracy metrics

**Exercise Library** ([ExerciseLibrary.hpp](Core/Inc/ExerciseLibrary.hpp))
- Predefined practice patterns and exercises
- Configurable difficulty levels

### FreeRTOS Tasks

| Task | Priority | Period | Function |
|------|----------|--------|----------|
| `sampling_task` | 10 | 5ms | ADC data acquisition via DMA |
| `strike_task` | 7 | Event-driven | Strike detection and timing analysis |
| `metronome_tick_task` | 6 | BPM-based | LED indication and beat tracking |
| `uart_send_values_task` | 5 | Configurable | Serial data transmission |

### Data Flow

```
Piezo Sensors → ADC + DMA → Calibration → Circular Buffer
                                              ↓
                                        Strike Detection
                                              ↓
                                    Timing Comparison ← Metronome Timer
                                              ↓
                                    Accuracy Calculation
                                              ↓
                                        UART Output
```

## Configuration

### Metronome BPM

Modify in [cpp_main.cpp](Core/Src/cpp_main.cpp):
```cpp
metronome.setBPM(120); // Set to desired BPM (40-208)
```

### Strike Detection Threshold

Adjust sensitivity in strike detection algorithm:
```cpp
constexpr int16_t STRIKE_THRESHOLD = 10; // Lower = more sensitive
```

### Sensor Calibration

Update coefficients in `get_adc1_values()` function to match your sensors.

## Serial Communication

### Connection
- **Baud Rate**: 115200
- **Data Bits**: 8
- **Stop Bits**: 1
- **Parity**: None

### Output Format
The system outputs timing data including:
- Strike timestamps
- Timing error (ms from beat)
- Accuracy percentage
- Missed beats count

## Development

### Toolchain Configuration

- **Compiler**: ARM GCC
- **Target**: Cortex-M4 with FPU
- **Flags**: `-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard`
- **C Standard**: C11
- **C++ Standard**: C++17

### Adding New Features

1. Create header in `Core/Inc/`
2. Implement in `Core/Src/`
3. Add source files to `CMakeLists.txt`
4. Build and test

### Mixed C/C++ Approach

- C handles HAL initialization and interrupts
- C++ implements application logic
- Use `extern "C"` for interoperability

## Performance Characteristics

- **ADC Sampling Rate**: 200 Hz (5ms period)
- **Strike Detection Latency**: < 10ms
- **Metronome Accuracy**: ±1ms (hardware timer-based)
- **Minimum Strike Interval**: 60ms (debounce)

## Troubleshooting

**No sensor readings**
- Check ADC channel connections (PA0, PA1, PB0, PC0, PC1)
- Verify piezo sensor polarity and wiring
- Review calibration coefficients

**Timing inaccuracies**
- Ensure metronome timer clock configuration is correct (84 MHz)
- Check FreeRTOS tick rate (1 kHz default)
- Verify task priorities

**Build errors**
- Confirm ARM toolchain is in PATH
- Check CMake version (3.22+)
- Verify all submodules are initialized

## License

[Add your license here]

## Contributing

[Add contribution guidelines here]

## Contact

[Add contact information here]

## Acknowledgments

- STM32 HAL Library by STMicroelectronics
- FreeRTOS by Amazon Web Services
- ARM CMSIS

---
