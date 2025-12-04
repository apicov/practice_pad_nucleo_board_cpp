# Practice Pad Application

Simple terminal-based application for practicing with the STM32 practice pad using George Lawrence Stone's "Stick Control" method.

## Installation

1. **Install Python dependencies:**
   ```bash
   pip install -r requirements.txt
   ```

2. **Make the script executable (Linux/Mac):**
   ```bash
   chmod +x practice.py
   ```

## Usage

### Basic Usage (Linux/Mac)
```bash
./practice.py
```

### Specify Serial Port
```bash
# Linux
./practice.py --port /dev/ttyACM0

# Mac
./practice.py --port /dev/tty.usbmodem*

# Windows
python practice.py --port COM3
```

### Finding Your Serial Port

**Linux:**
```bash
ls /dev/ttyACM*
# or
ls /dev/ttyUSB*
```

**Mac:**
```bash
ls /dev/tty.usb*
```

**Windows:**
- Check Device Manager → Ports (COM & LPT)

## Features

### 1. Practice Session
- Load any exercise (1-320)
- Set custom tempo (40-200 BPM)
- Real-time strike feedback with timing (±ms)
- Rep counter (X/20)
- **Automatic reset on failure** (Stone's method!)
- Success notification on completion

### 2. Metronome Test
- Test metronome at any tempo
- Visual LED feedback on STM32 board

### 3. Connection Test
- Verify serial communication
- Check device status

## Exercise Library

- **Level 1 (1-72)**: Single Beat Combinations
- **Level 2 (73-120)**: Triplet Patterns
- **Level 3 (121-220)**: Short Roll Combinations
- **Level 4 (221-320)**: Flam Beats

## Stone's Method

The application enforces George Lawrence Stone's practice methodology:

1. **20 repetitions rule**: You must play the pattern 20 times perfectly
2. **No stopping**: All 20 reps must be continuous
3. **Reset on failure**: If you mess up on rep 15, you start over from 0!
4. **Tempo progression**: Only after mastering one tempo, move to faster

## Real-Time Feedback

### Strike Indicators
- 🎯 **Perfect** (±20ms)
- ✓ **Good** (±40ms)
- ⚠ **Acceptable** (±60ms)
- ✗ **Poor** (>60ms)

### Session Flow
```
Progress: 1/20
  ✓ Strike: +12 ms
  🎯 Strike: -8 ms
  ✓ Strike: +15 ms
  ✓ Strike: -18 ms

Progress: 2/20
  ...

⚠ RESET TO 0! Keep trying...

Progress: 1/20
  ...

Progress: 20/20

🎉 SUCCESS! EXERCISE COMPLETED!
```

## Troubleshooting

### Connection Issues

**"Failed to connect"**
- Check USB cable connection
- Verify STM32 is powered on
- Try different USB port
- Check correct serial port with `ls /dev/tty*`

**"Device not responding"**
- Reset STM32 (press reset button)
- Disconnect and reconnect USB
- Check firmware is flashed correctly

**"Permission denied" (Linux)**
```bash
sudo usermod -a -G dialout $USER
# Then log out and log back in
```

### Serial Port Conflicts
- Close any other programs using the port (Arduino IDE, serial monitors, etc.)
- Only one program can access the serial port at a time

## Development

### Project Structure
```
practice_pad_app/
├── practice.py              # Main application
├── requirements.txt         # Python dependencies
├── serial_comm/
│   ├── __init__.py
│   └── stm32_interface.py  # STM32 communication
└── README.md               # This file
```

### STM32 Commands

The application uses these serial commands:

- `PING` - Test connection
- `SET_BPM <bpm>` - Set metronome tempo
- `LOAD_EXERCISE <id> <tempo>` - Load exercise
- `START_SESSION` - Begin practice
- `PAUSE_SESSION` - Pause
- `RESUME_SESSION` - Resume
- `RESET_SESSION` - Reset to 0/20
- `ABORT_SESSION` - Stop session
- `GET_SESSION_STATE` - Query status

### Real-Time Events

STM32 sends these events during practice:

- `EVENT STRIKE <timing_ms>` - Strike detected with timing offset
- `EVENT REP <current>/<total>` - Rep counter update

## Tips for Practice

1. **Start slow** - Begin at 60 BPM, focus on control
2. **Stay relaxed** - Stop if you feel tension (Stone's advice!)
3. **Be patient** - The reset rule is frustrating but builds real skill
4. **Tempo progression** - Only increase when comfortable (every 10-20 BPM)
5. **Consistency** - Practice daily for best results

## Next Steps

After getting comfortable with the terminal app:
- Add database for progress tracking
- Create GUI with PyQt or Tkinter
- Implement statistics and analytics
- Add achievement system
- Create progress visualization

## License

This practice application is based on "Stick Control for the Snare Drummer" by George Lawrence Stone (1935).
