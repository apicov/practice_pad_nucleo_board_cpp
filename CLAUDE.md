# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

This is an embedded C/C++ project for an STM32F446xx microcontroller (STM32 Nucleo board) that implements a digital practice pad with metronome functionality. The project uses FreeRTOS for real-time task management and combines C and C++ code.

### Key Features
- **ADC-based sensor reading**: Reads from 5 piezo sensors to detect drum strikes
- **Strike detection**: Processes sensor data to identify drum hits with configurable thresholds
- **Metronome system**: Hardware timer-based metronome with configurable BPM
- **Timing analysis**: Compares strike timing against metronome beats to measure accuracy
- **Real-time processing**: Uses FreeRTOS tasks for concurrent ADC sampling, strike detection, and metronome handling

## Build System

### Commands
- **Configure**: `cmake --preset Debug` (or Release, RelWithDebInfo, MinSizeRel)
- **Build**: `cmake --build build/Debug`
- **Alternative build**: `ninja -C build/Debug`

### Build Presets
The project uses CMake presets defined in `CMakePresets.json`:
- `Debug`: Development build with debugging symbols
- `Release`: Optimized production build
- `RelWithDebInfo`: Release with debug info
- `MinSizeRel`: Size-optimized build

### Toolchain
- **Compiler**: ARM GCC (`arm-none-eabi-gcc/g++`)
- **Target**: Cortex-M4 with FPU (`-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard`)
- **Standards**: C11, C++17

## Architecture

### Core Components

1. **Main Application (`Core/Src/cpp_main.cpp`)**
   - Entry point: `cpp_main()` function called from C `main()`
   - Creates FreeRTOS tasks and synchronization primitives
   - Manages system initialization

2. **Metronome Class (`Core/Inc/Metronome.hpp`, `Core/Src/Metronome.cpp`)**
   - Hardware timer-based (TIM3) metronome implementation
   - Configurable BPM with automatic period calculation
   - Provides timing reference for strike accuracy measurement

3. **CircularBuffer Template (`Core/Inc/CircularBuffer.hpp`)**
   - Thread-safe circular buffer for sensor data storage
   - Template-based for type flexibility
   - Used for ADC data buffering

4. **FreeRTOS Tasks**
   - `sampling_task`: ADC data acquisition (5ms period)
   - `strike_task`: Strike detection and timing analysis
   - `metronome_tick_task`: Metronome LED indication
   - `uart_send_values_task`: UART data transmission (currently disabled)

### Hardware Configuration
- **ADC1**: 5 channels (0,1,8,10,11) for piezo sensors, 8-bit resolution
- **DMA**: Used for ADC and UART data transfer
- **TIM3**: Metronome timer with configurable period
- **USART2**: Serial communication for data output
- **GPIO**: LED control for metronome indication

### Data Flow
1. ADC samples 5 piezo sensors every 5ms via DMA
2. Raw data is calibrated and stored in circular buffer
3. Strike detection algorithm analyzes sensor data for peaks
4. Detected strikes are timestamped and compared against metronome ticks
5. Timing accuracy is calculated and output via printf/UART

## Development Guidelines

### File Organization
- `Core/Inc/`: Header files for application code
- `Core/Src/`: Implementation files
- `Drivers/`: STM32 HAL drivers and CMSIS
- `Middlewares/`: FreeRTOS source code
- `cmake/`: Build configuration files

### Mixed C/C++ Approach
- C code handles STM32 HAL initialization and interrupt handlers
- C++ code implements application logic with modern features
- Use `extern "C"` blocks for C/C++ interoperability

### Real-time Considerations
- All tasks use proper FreeRTOS synchronization (semaphores, mutexes, queues)
- Critical timing handled in ISR context with minimal processing
- Task priorities: sampling (10) > strike detection (7) > metronome UI (6)

### Memory Management
- Static allocation preferred for embedded context
- Circular buffers prevent memory fragmentation
- Fixed-size data structures with compile-time sizing

## Hardware-Specific Notes

### Calibration
Sensor values are calibrated using hardcoded coefficients in `get_adc1_values()`:
- Each channel has specific scale and offset values
- Channel 4 (force sensor) uses inverted scaling

### Timer Configuration
- TIM3 prescaler: 8400 (84MHz / 8400 = 10kHz = 100µs resolution)
- Auto-reload value calculated from BPM: `(600'000 / bpm) - 1`

### Strike Detection Algorithm
- Calculates average of first 4 sensor channels
- Detects rising edge with threshold (>10 difference from previous)
- Implements minimum time between strikes (60ms debounce)