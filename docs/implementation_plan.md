# Stick Control Learning System - Implementation Plan
## Based on Current Codebase Analysis

---

## Current System Analysis

### ✅ What You Already Have (Excellent Foundation!)

1. **Hardware Integration** ✓
   - ADC sampling working (5 piezo sensors, 5ms rate)
   - Strike detection algorithm (`is_strike()`)
   - Metronome with TIM3 (configurable BPM)
   - Timing difference calculation (strike vs metronome tick)

2. **FreeRTOS Infrastructure** ✓
   - Task management (`sampling_task`, `strike_task`, `metronome_tick_task`)
   - Semaphores and mutexes for synchronization
   - Queue system for strike events
   - Circular buffers for data

3. **Timing Analysis** ✓
   - Real-time timing offset calculation (early/late detection)
   - Half-period logic for beat assignment
   - Practice statistics tracking:
     - Total strikes, accurate strikes
     - Early/late categorization
     - Missed beat detection
     - Average timing offset

4. **Communication Layer** ✓
   - UART/Serial with DMA (bidirectional)
   - Command interface (`command_interface_task`)
   - Existing commands: `SET_BPM`, `START_PRACTICE`, `STOP_PRACTICE`, `GET_STATS`
   - Real-time event messages (`EVENT STRIKE`)

5. **Code Organization** ✓
   - C++ classes (`Metronome`, `CircularBuffer`)
   - Clean C/C++ interop
   - Well-structured tasks

---

## What Needs to Be Added

### Phase 1: Core Exercise System (Week 1-2)

#### 1.1 Create Exercise Data Structures

**File: `Core/Inc/Exercise.hpp`** (NEW)

```cpp
#pragma once
#include <cstdint>

// Exercise definition
struct Exercise {
    uint16_t id;                      // 1-400+
    uint8_t level;                    // 1-8 (Level 1 = Single Beats, etc.)
    uint8_t section;                  // 1-4 (A, B, C, D sections)
    char pattern[20];                 // "RLRL RLRL RLRL RLRL"
    char name[32];                    // "Basic Alternating"
    uint8_t difficulty;               // 1-5 stars
    uint8_t beats_per_pattern;        // 4 for most (RLRL = 4 beats)
    uint8_t default_tempo_start;      // 60 BPM
    uint8_t default_tempo_end;        // 120 BPM
    uint8_t tempo_step;               // 20 BPM increments
};

// Tempo milestone for an exercise
struct TempoMilestone {
    uint16_t exercise_id;
    uint8_t tempo;                    // 60, 80, 100, 120
    bool completed;
};
```

**File: `Core/Inc/ExerciseLibrary.hpp`** (NEW)

```cpp
#pragma once
#include "Exercise.hpp"

class ExerciseLibrary {
public:
    static const Exercise* get_exercise(uint16_t id);
    static uint16_t get_total_exercises();
    static const Exercise* get_first_exercise();
    static const Exercise* get_next_exercise(uint16_t current_id);

private:
    static const Exercise exercises[];
    static const uint16_t exercise_count;
};
```

**File: `Core/Src/ExerciseLibrary.cpp`** (NEW)

```cpp
#include "ExerciseLibrary.hpp"

// Exercise database - start with Level 1.1 (first 12)
const Exercise ExerciseLibrary::exercises[] = {
    // Level 1, Section A: Basic Patterns (1-12)
    {1, 1, 1, "RLRL RLRL RLRL RLRL", "Basic Alternating", 1, 4, 60, 120, 20},
    {2, 1, 1, "LRLR LRLR LRLR LRLR", "Reverse Alternating", 1, 4, 60, 120, 20},
    {3, 1, 1, "RRLL RRLL RRLL RRLL", "Double Beats", 1, 4, 60, 120, 20},
    {4, 1, 1, "LLRR LLRR LLRR LLRR", "Double Beats Reversed", 1, 4, 60, 120, 20},
    {5, 1, 1, "RLRR LRLL RLRR LRLL", "Mixed Doubles", 2, 4, 60, 120, 20},
    {6, 1, 1, "RLLR LRRL RLLR LRRL", "Mirrored Pattern", 2, 4, 60, 120, 20},
    {7, 1, 1, "RRLR LLRL RRLR LLRL", "Syncopated Doubles", 2, 4, 60, 120, 20},
    {8, 1, 1, "RLRL LRLR RLRL LRLR", "Alternating Groups", 2, 4, 60, 120, 20},
    {9, 1, 1, "RRRL RRRL RRRL RRRL", "Triple Rights", 2, 4, 60, 120, 20},
    {10, 1, 1, "LLLR LLLR LLLR LLLR", "Triple Lefts", 2, 4, 60, 120, 20},
    {11, 1, 1, "RLLL RLLL RLLL RLLL", "Right Lead Left Heavy", 2, 4, 60, 120, 20},
    {12, 1, 1, "LRRR LRRR LRRR LRRR", "Left Lead Right Heavy", 2, 4, 60, 120, 20},
    // Add more exercises later...
};

const uint16_t ExerciseLibrary::exercise_count = sizeof(exercises) / sizeof(Exercise);

const Exercise* ExerciseLibrary::get_exercise(uint16_t id) {
    for (uint16_t i = 0; i < exercise_count; i++) {
        if (exercises[i].id == id) {
            return &exercises[i];
        }
    }
    return nullptr;
}

uint16_t ExerciseLibrary::get_total_exercises() {
    return exercise_count;
}

const Exercise* ExerciseLibrary::get_first_exercise() {
    return exercise_count > 0 ? &exercises[0] : nullptr;
}

const Exercise* ExerciseLibrary::get_next_exercise(uint16_t current_id) {
    for (uint16_t i = 0; i < exercise_count - 1; i++) {
        if (exercises[i].id == current_id) {
            return &exercises[i + 1];
        }
    }
    return nullptr;
}
```

#### 1.2 Create Session Manager

**File: `Core/Inc/SessionManager.hpp`** (NEW)

```cpp
#pragma once
#include "Exercise.hpp"
#include <cstdint>

extern "C" {
    #include "cmsis_os.h"
}

// Session state
enum SessionState {
    SESSION_IDLE,           // Not practicing
    SESSION_READY,          // Exercise loaded, ready to start
    SESSION_ACTIVE,         // Currently practicing
    SESSION_PAUSED,         // Paused mid-session
    SESSION_SUCCESS,        // Completed 20/20 reps
    SESSION_FAILED          // Failed mid-session
};

// Repetition result
struct RepetitionResult {
    uint8_t rep_number;         // 1-20
    uint8_t strikes_detected;   // Should match exercise pattern
    uint8_t strikes_expected;   // From exercise definition
    int32_t avg_timing_error;   // Average ±ms for this rep
    bool success;               // All strikes present and accurate
};

class SessionManager {
public:
    SessionManager();

    // Exercise management
    bool load_exercise(uint16_t exercise_id, uint8_t tempo);
    const Exercise* get_current_exercise() const;
    uint8_t get_current_tempo() const;

    // Session control
    void start_session();
    void pause_session();
    void resume_session();
    void reset_session();
    void abort_session();

    // Strike processing
    void process_strike(int32_t timing_offset_ms);
    void process_beat();  // Called on each metronome tick

    // State queries
    SessionState get_state() const;
    uint8_t get_current_rep() const;
    uint8_t get_total_reps() const { return 20; }
    RepetitionResult get_last_rep_result() const;

    // Statistics
    uint32_t get_session_duration_ms() const;
    uint8_t get_attempt_count() const;

private:
    const Exercise* current_exercise_;
    uint8_t current_tempo_;
    SessionState state_;

    // Current rep tracking
    uint8_t current_rep_;               // 0-19 (20 total)
    uint8_t strikes_in_current_rep_;
    uint8_t expected_strikes_per_rep_;
    int32_t timing_error_sum_;          // For averaging
    TickType_t rep_start_time_;

    // Session stats
    uint32_t session_start_time_;
    uint8_t attempt_count_;
    RepetitionResult last_rep_result_;

    // Internal logic
    void finalize_repetition();
    void fail_session();
    void complete_session();
};
```

**Implementation highlights for `SessionManager.cpp`:**

```cpp
void SessionManager::process_strike(int32_t timing_offset_ms) {
    if (state_ != SESSION_ACTIVE) return;

    strikes_in_current_rep_++;
    timing_error_sum_ += timing_offset_ms;

    // Check if rep is complete (got all expected strikes)
    if (strikes_in_current_rep_ >= expected_strikes_per_rep_) {
        finalize_repetition();
    }
}

void SessionManager::finalize_repetition() {
    int32_t avg_error = timing_error_sum_ / strikes_in_current_rep_;

    // Success criteria: got all strikes, timing acceptable
    bool success = (strikes_in_current_rep_ == expected_strikes_per_rep_) &&
                   (abs(avg_error) <= 60);  // ±60ms tolerance

    if (success) {
        current_rep_++;
        if (current_rep_ >= 20) {
            complete_session();  // SUCCESS!
        }
    } else {
        fail_session();  // RESET to 0
    }

    // Reset for next rep
    strikes_in_current_rep_ = 0;
    timing_error_sum_ = 0;
}

void SessionManager::fail_session() {
    current_rep_ = 0;  // CRITICAL: Reset to 0!
    attempt_count_++;
    // Stay in SESSION_ACTIVE state - continue practicing
}
```

#### 1.3 Integrate into cpp_main.cpp

**Modifications to `Core/Src/cpp_main.cpp`:**

```cpp
#include "SessionManager.hpp"
#include "ExerciseLibrary.hpp"

// Global session manager
SessionManager* g_session_manager = nullptr;

void cpp_main() {
    // ... existing initialization ...

    // Initialize session manager
    g_session_manager = new SessionManager();
    printf("Session manager initialized\n");

    // ... rest of initialization ...
}

// Modify strike_task to use SessionManager
void strike_task(void *pvParameters) {
    // ... existing code ...

    // After calculating time_diff:
    if (g_session_manager && g_session_manager->get_state() == SESSION_ACTIVE) {
        g_session_manager->process_strike(time_diff);

        // Send rep count update
        sprintf(strike_msg, "EVENT REP %d/20\n", g_session_manager->get_current_rep());
        // ... send via UART ...
    }
}

// Modify metronome_tick_task
void metronome_tick_task(void *pvParameters) {
    // ... existing code ...

    if (g_session_manager && g_session_manager->get_state() == SESSION_ACTIVE) {
        g_session_manager->process_beat();
    }
}
```

#### 1.4 Extend UART Command Protocol

**Add to `process_command()` in cpp_main.cpp:**

```cpp
else if (strncmp(command, "LOAD_EXERCISE ", 14) == 0) {
    // Format: "LOAD_EXERCISE <id> <tempo>"
    int ex_id, tempo;
    if (sscanf(command + 14, "%d %d", &ex_id, &tempo) == 2) {
        if (g_session_manager->load_exercise(ex_id, tempo)) {
            const Exercise* ex = g_session_manager->get_current_exercise();
            sprintf(response, "CMD_OK EXERCISE:%d PATTERN:%s TEMPO:%d\n",
                    ex->id, ex->pattern, tempo);
        } else {
            sprintf(response, "CMD_ERROR Invalid exercise ID\n");
        }
    }
}
else if (strncmp(command, "START_SESSION", 13) == 0) {
    g_session_manager->start_session();
    sprintf(response, "CMD_OK Session started\n");
}
else if (strncmp(command, "GET_SESSION_STATE", 17) == 0) {
    sprintf(response, "CMD_RESP STATE:%d REP:%d/20 ATTEMPTS:%d\n",
            g_session_manager->get_state(),
            g_session_manager->get_current_rep(),
            g_session_manager->get_attempt_count());
}
else if (strncmp(command, "RESET_SESSION", 13) == 0) {
    g_session_manager->reset_session();
    sprintf(response, "CMD_OK Session reset\n");
}
```

---

### Phase 2: Python PC Application (Week 2-3)

#### 2.1 Project Structure

```
practice_pad_app/
├── main.py                      # Entry point
├── requirements.txt             # Dependencies
├── config.py                    # Configuration
├── serial_comm/
│   ├── __init__.py
│   ├── stm32_interface.py      # Serial communication
│   └── protocol.py             # Message parsing
├── database/
│   ├── __init__.py
│   ├── db_manager.py           # Database interface
│   ├── schema.sql              # DB schema
│   └── models.py               # Data models
├── ui/
│   ├── __init__.py
│   ├── main_window.py          # Main application window
│   ├── practice_screen.py      # Practice interface
│   ├── progress_map.py         # Level map
│   └── stats_dashboard.py      # Analytics
└── logic/
    ├── __init__.py
    ├── session_controller.py   # Session management
    └── exercise_manager.py     # Exercise logic
```

#### 2.2 Serial Communication Module

**File: `serial_comm/stm32_interface.py`**

```python
import serial
import threading
import queue
import time
from typing import Callable, Optional

class STM32Interface:
    def __init__(self, port: str = '/dev/ttyACM0', baudrate: int = 115200):
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.running = False
        self.read_thread = None
        self.message_queue = queue.Queue()
        self.event_callbacks = {}  # event_type -> callback

    def connect(self) -> bool:
        try:
            self.serial = serial.Serial(self.port, self.baudrate, timeout=0.1)
            time.sleep(2)  # Wait for STM32 reset
            self.running = True
            self.read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self.read_thread.start()
            print(f"Connected to {self.port}")
            return True
        except Exception as e:
            print(f"Connection failed: {e}")
            return False

    def disconnect(self):
        self.running = False
        if self.read_thread:
            self.read_thread.join(timeout=1.0)
        if self.serial:
            self.serial.close()

    def send_command(self, command: str) -> Optional[str]:
        """Send command and wait for response"""
        if not self.serial:
            return None

        try:
            self.serial.write(f"{command}\n".encode())
            # Wait for response (CMD_OK, CMD_ERROR, or CMD_RESP)
            start_time = time.time()
            while time.time() - start_time < 2.0:
                if not self.message_queue.empty():
                    msg = self.message_queue.get()
                    if msg.startswith("CMD_"):
                        return msg
                time.sleep(0.01)
            return None
        except Exception as e:
            print(f"Send error: {e}")
            return None

    def register_event_callback(self, event_type: str, callback: Callable):
        """Register callback for real-time events (EVENT STRIKE, EVENT REP, etc.)"""
        self.event_callbacks[event_type] = callback

    def _read_loop(self):
        """Background thread to read serial data"""
        buffer = ""
        while self.running:
            try:
                if self.serial and self.serial.in_waiting:
                    data = self.serial.read(self.serial.in_waiting).decode('utf-8', errors='ignore')
                    buffer += data

                    # Process complete lines
                    while '\n' in buffer:
                        line, buffer = buffer.split('\n', 1)
                        line = line.strip()

                        if line.startswith("EVENT "):
                            self._handle_event(line)
                        else:
                            self.message_queue.put(line)

            except Exception as e:
                print(f"Read error: {e}")
            time.sleep(0.001)

    def _handle_event(self, line: str):
        """Handle real-time events"""
        parts = line.split()
        if len(parts) >= 2:
            event_type = parts[1]  # "STRIKE", "REP", etc.
            if event_type in self.event_callbacks:
                self.event_callbacks[event_type](parts[2:])  # Pass remaining data

    # High-level command methods
    def set_bpm(self, bpm: int) -> bool:
        response = self.send_command(f"SET_BPM {bpm}")
        return response and "CMD_OK" in response

    def load_exercise(self, exercise_id: int, tempo: int) -> bool:
        response = self.send_command(f"LOAD_EXERCISE {exercise_id} {tempo}")
        return response and "CMD_OK" in response

    def start_session(self) -> bool:
        response = self.send_command("START_SESSION")
        return response and "CMD_OK" in response

    def get_session_state(self) -> dict:
        response = self.send_command("GET_SESSION_STATE")
        if response:
            # Parse: "CMD_RESP STATE:2 REP:5/20 ATTEMPTS:3"
            data = {}
            for part in response.split():
                if ':' in part:
                    key, value = part.split(':', 1)
                    data[key] = value
            return data
        return {}
```

#### 2.3 Simple Practice UI (Terminal-based for Phase 1)

**File: `main.py`** (Initial version)

```python
import sys
import time
from serial_comm.stm32_interface import STM32Interface

class SimplePracticeApp:
    def __init__(self):
        self.stm32 = STM32Interface()
        self.current_rep = 0

    def run(self):
        print("=== Stick Control Practice App ===")

        # Connect to STM32
        if not self.stm32.connect():
            print("Failed to connect to practice pad")
            return

        # Register event callbacks
        self.stm32.register_event_callback("STRIKE", self.on_strike)
        self.stm32.register_event_callback("REP", self.on_rep_update)

        # Set BPM
        self.stm32.set_bpm(60)
        print("BPM set to 60")

        # Load first exercise
        self.stm32.load_exercise(1, 60)
        print("Loaded Exercise 1: RLRL RLRL RLRL RLRL")

        # Start session
        print("\nStarting practice session...")
        print("Play the pattern 20 times without stopping!")
        print("Progress: 0/20")

        self.stm32.start_session()

        # Keep running
        try:
            while True:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\nStopping...")
            self.stm32.disconnect()

    def on_strike(self, data):
        """Called when strike is detected"""
        timing_error = int(data[0]) if data else 0
        indicator = "✓" if abs(timing_error) < 50 else "⚠"
        print(f"  {indicator} Strike: {timing_error:+4d}ms")

    def on_rep_update(self, data):
        """Called when rep counter updates"""
        rep_str = data[0] if data else "0/20"
        print(f"Progress: {rep_str}")

        # Check if completed
        if "20/20" in rep_str:
            print("\n🎉 SUCCESS! Exercise completed!")
            print("You can now try a faster tempo or move to next exercise")

if __name__ == "__main__":
    app = SimplePracticeApp()
    app.run()
```

---

## Implementation Checklist

### ✅ Phase 1: Foundation (Current Focus)

- [ ] Create `Exercise.hpp` with data structures
- [ ] Create `ExerciseLibrary.hpp/.cpp` with first 12 exercises
- [ ] Create `SessionManager.hpp/.cpp` with 20-rep logic
- [ ] Integrate SessionManager into `cpp_main.cpp`
- [ ] Extend UART protocol with new commands
- [ ] Test: Load exercise, start session, detect 20 reps
- [ ] Create Python `stm32_interface.py`
- [ ] Create simple terminal-based practice app
- [ ] Test: End-to-end exercise completion

**Success Criteria:**
- Can load Exercise 1 at 60 BPM
- Rep counter increments on successful patterns
- Counter resets to 0 on failure
- Success message after 20 perfect reps

---

### Phase 2: Multi-Tempo & Database (Next)

- [ ] Add tempo progression logic to SessionManager
- [ ] Create SQLite database schema
- [ ] Implement progress tracking (which exercises/tempos completed)
- [ ] Add unlock logic (next exercise only after all tempos)
- [ ] GUI practice screen (PyQt or Tkinter)
- [ ] Visual feedback for strikes and reps

---

### Phase 3: Full Exercise Library

- [ ] Add all 72 Level 1 exercises to ExerciseLibrary
- [ ] Add exercises for Levels 2-8
- [ ] Exercise browser UI
- [ ] Progress map visualization

---

### Phase 4: Gamification

- [ ] Achievement system
- [ ] Statistics dashboard
- [ ] Streak tracking
- [ ] Level progression with badges

---

## Quick Start Guide

### Step 1: Add New Files to STM32 Project

1. Create `Core/Inc/Exercise.hpp`
2. Create `Core/Inc/ExerciseLibrary.hpp`
3. Create `Core/Src/ExerciseLibrary.cpp`
4. Create `Core/Inc/SessionManager.hpp`
5. Create `Core/Src/SessionManager.cpp`

### Step 2: Modify CMakeLists.txt

Add new source files to `CMakeLists.txt`:

```cmake
# In the C++ sources section
set(SOURCES
    # ... existing sources ...
    Core/Src/ExerciseLibrary.cpp
    Core/Src/SessionManager.cpp
)
```

### Step 3: Build and Flash

```bash
cmake --build build/Debug
# Flash to STM32
```

### Step 4: Create Python App

```bash
mkdir practice_pad_app
cd practice_pad_app
python -m venv venv
source venv/bin/activate
pip install pyserial
# Create main.py and stm32_interface.py
python main.py
```

---

## Key Design Decisions

### 1. **SessionManager is the Brain**
- Handles all exercise logic
- Implements the 20-rep rule
- Tracks timing accuracy
- Decides success/failure

### 2. **Minimal Changes to Existing Code**
- Your strike detection and timing code stays intact
- Just pipe strike events to SessionManager
- UART protocol is extended, not replaced

### 3. **Firmware = Logic, PC = UI**
- STM32 handles all critical timing and practice logic
- PC application just displays state and handles user input
- This ensures practice works even if UI crashes

### 4. **Progressive Enhancement**
- Phase 1: Basic single-exercise practice working
- Phase 2: Multi-tempo progression
- Phase 3: Full exercise library
- Phase 4: Gamification polish

---

## Testing Strategy

### Unit Tests (Firmware)
- Test SessionManager rep counter logic
- Test exercise lookup
- Test timing tolerance validation

### Integration Tests
- Test full practice session (load → start → 20 reps → success)
- Test failure and reset
- Test tempo changes
- Test UART command parsing

### System Tests
- Practice Exercise 1 at 60 BPM to completion
- Practice at multiple tempos (60, 80, 100, 120)
- Complete exercise and unlock next
- Test missed beat detection
- Test early/late strike categorization

---

## Estimated Timeline

- **Week 1**: Create Exercise/SessionManager classes, integrate into firmware
- **Week 2**: UART protocol extension, Python serial interface
- **Week 3**: Simple terminal UI, end-to-end testing
- **Week 4**: Multi-tempo logic, database setup
- **Week 5**: GUI practice screen
- **Week 6**: Add remaining Level 1 exercises
- **Week 8**: Progress map and statistics
- **Week 10**: Gamification and polish

**Minimum Viable Product**: Week 3
**Full Featured System**: Week 10-12

---

## Next Immediate Steps

1. **Create `Exercise.hpp`** - Start with the data structure
2. **Create `ExerciseLibrary.cpp`** - Add first 12 exercises
3. **Create `SessionManager.hpp`** - Implement 20-rep logic
4. **Test** - Load exercise, practice, verify rep counting

Would you like me to start implementing Phase 1 files now?
