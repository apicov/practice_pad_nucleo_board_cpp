# Qt GUI Application Guide

Beautiful, professional GUI for Stick Control practice with real-time visual feedback.

## Features

### 🎨 **Visual Interface**
- Real-time timing accuracy plot
- Color-coded strike feedback (green/yellow/orange/red)
- Progress bar showing 0-20 reps
- Event log with detailed feedback
- Clean, professional layout

### 📊 **Real-Time Visualization**
- **Timing Plot**: See your accuracy over last 80 strikes
- **Color Coding**:
  - 🟢 Green: Perfect (±20ms)
  - 🟡 Light Green: Good (±40ms)
  - 🟠 Orange: OK (±60ms)
  - 🔴 Red: Poor (>60ms)

### 🎮 **Full Session Control**
- Load any exercise (1-320)
- Set tempo (40-200 BPM)
- Start/Pause/Resume/Reset
- Real-time rep counter
- Automatic reset on failure
- Success notification

---

## Installation

```bash
# Install all dependencies
pip install -r requirements.txt
```

Dependencies:
- PyQt5 (GUI framework)
- matplotlib (plotting)
- numpy (data processing)
- pyserial (STM32 communication)

---

## Running the GUI

```bash
# Linux/Mac
./practice_gui.py

# Specify port
./practice_gui.py --port /dev/ttyACM0

# Windows
python practice_gui.py --port COM3
```

---

## User Interface

### Left Panel: Controls

#### 1. **Connection**
- Click "Connect" to connect to STM32
- Status indicator shows connection state
- Green 🟢 = Connected, Gray ⭕ = Disconnected

#### 2. **Exercise Selection**
- **Exercise ID**: Use spinner or type (1-320)
- **Tempo**: Set BPM (40-200, step by 10)
- Click "Load Exercise" to load

#### 3. **Current Exercise**
- Shows exercise name, pattern, tempo
- Example: "Exercise 1: Basic Alternating"
- Pattern displayed in monospace font

#### 4. **Session Control**
- **▶ Start Session**: Begin practicing
- **⏸ Pause**: Pause mid-session
- **🔄 Reset to 0/20**: Start over (with confirmation)
- **⏹ Stop Session**: Abort session (with confirmation)

#### 5. **Progress**
- Large "Rep: X/20" display
- Progress bar (green fill)
- Attempts counter (how many resets)

### Right Panel: Visualization

#### 1. **Real-Time Strike Analysis**
- Scatter plot of timing accuracy
- X-axis: Strike number
- Y-axis: Timing error (±ms)
- Green line at 0 = perfect timing
- Orange lines at ±60ms = acceptable range

#### 2. **Timing Guide**
- Legend explaining color codes
- Quick reference for accuracy

#### 3. **Event Log**
- Scrolling text log
- All strikes with timing
- Rep updates
- Session events (start, reset, complete)
- Auto-scrolls to latest

---

## Example Session

### 1. Connect
```
[Click "Connect" button]
✓ Connected successfully!
```

### 2. Load Exercise
```
Exercise ID: 1
Tempo: 60
[Click "Load Exercise"]

✓ Loaded: Basic Alternating
  Pattern: RLRL RLRL RLRL RLRL
  Tempo: 60 BPM
```

### 3. Practice
```
[Click "▶ Start Session"]

🎯 SESSION STARTED - GO!
Play the pattern 20 times perfectly!

  🎯 Strike: +8 ms    ← Plot updates
  ✓ Strike: -12 ms
  ✓ Strike: +15 ms
  ✓ Strike: -5 ms

📊 Progress: 1/20     ← Progress bar fills

  ✓ Strike: +10 ms
  ⚠ Strike: +45 ms
  ✓ Strike: -8 ms
  ✓ Strike: +12 ms

📊 Progress: 2/20
...

[On rep 15, one strike is >100ms]

⚠ RESET TO 0/20! Keep trying...  ← Plot clears
Rep: 0/20                          ← Bar resets

[Try again...]

📊 Progress: 20/20

========================================
🎉 SUCCESS! EXERCISE COMPLETED!
========================================

[Pop-up message shows]
```

---

## Visual Feedback Guide

### Strike Indicators in Log
- **🎯** Perfect (±20ms) - Green dot on plot
- **✓** Good (±40ms) - Light green dot
- **⚠** OK (±60ms) - Orange dot
- **✗** Poor (>60ms) - Red dot

### Plot Interpretation

**Good Session:**
```
Timing (ms)
    100│
     80│
     60│- - - - - -  (acceptable limit)
     40│     •   •
     20│  •    •   •  •
      0│━━━━━━━━━━━━━━  (perfect)
    -20│   •  •    •
    -40│        •
    -60│- - - - - -  (acceptable limit)
    -80│
   -100│
       └────────────────  Strike #
```
Most strikes clustered near 0 = excellent!

**Needs Work:**
```
Timing (ms)
    100│        •     (too late!)
     80│    •       •
     60│- - - - - -
     40│  •
     20│      •
      0│━━━━━━━━━━━━━━
    -20│    •
    -40│
    -60│- - - - - -
    -80│  •          (too early!)
   -100│      •
       └────────────────  Strike #
```
Too much scatter = needs practice!

---

## Keyboard Shortcuts

Currently none - all mouse-driven interface.
(Could add shortcuts in future: Space=Start, R=Reset, etc.)

---

## Troubleshooting

### "Connection Failed"
- Check USB cable connected
- Verify correct port (--port argument)
- Try: `ls /dev/ttyACM*` (Linux) or Device Manager (Windows)
- Close other programs using the port

### "Device not responding"
- Press reset button on STM32
- Disconnect and reconnect USB
- Verify firmware is flashed

### Plot not updating
- Check session is active (should see "Session active" in status bar)
- Verify you're hitting the practice pad
- Check event log for strike messages

### GUI not starting
```bash
# Check PyQt5 installed
pip list | grep PyQt5

# Reinstall if needed
pip install --upgrade PyQt5
```

### "Permission denied" (Linux)
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

---

## Advanced Features

### Window Layout
- Resizable window (drag edges)
- Panels maintain proportions
- Plot auto-scales to fit data

### Data Retention
- Last 80 strikes shown on plot
- Full log in event window
- Clears on exercise load or reset

### Status Bar
- Shows connection state
- Exercise level hints
- Session state updates

---

## Comparison: Terminal vs GUI

| Feature | Terminal (`practice.py`) | GUI (`practice_gui.py`) |
|---------|-------------------------|-------------------------|
| Strike timing | Text only | Visual plot |
| Feedback | Emojis in text | Color-coded graph |
| Progress | Text counter | Progress bar |
| History | Scrolling text | 80-strike plot |
| Session control | Keyboard | Buttons |
| User friendliness | Minimal | Professional |
| Resource usage | Very low | Moderate |

**Recommendation**: Use GUI for practice, terminal for debugging/testing.

---

## Tips for Best Results

1. **Full screen** - Maximize window to see everything
2. **Watch the plot** - Learn your timing patterns
3. **Use color cues** - Aim for all green strikes
4. **Check event log** - Detailed feedback on what went wrong
5. **Reset early** - Don't waste time on failed reps

---

## Future Enhancements

Possible additions:
- [ ] Statistics dashboard (avg timing, best session, etc.)
- [ ] Session history graph
- [ ] Audio feedback (click on beat)
- [ ] Keyboard shortcuts
- [ ] Save/load practice sessions
- [ ] Multiple exercise queue
- [ ] Custom difficulty presets

---

## Screenshot Description

```
┌─────────────────────────────────────────────────────────────────┐
│ Stick Control Practice Pad                                 [_][□][X]│
├───────────────┬──────────────────────────────────────────────────┤
│ Connection    │  Real-Time Strike Analysis                       │
│ 🟢 Connected  │                                                  │
│ [Disconnect]  │   100┤                                           │
│               │    50┤  • •   ••                                 │
│ Exercise      │     0┼━━━━━━━━━━━━━━  (Perfect)                 │
│ ID: [1  ▼]    │   -50┤    • •    •                              │
│ BPM:[60 ▼]    │  -100┤                                           │
│ [Load]        │      └────────────────  Strike #                │
│               │                                                  │
│ Current       │  🎯 Perfect  ✓ Good  ⚠ OK  ✗ Poor              │
│ ┌───────────┐ │                                                  │
│ │Exercise 1 │ │  Event Log:                                     │
│ │RLRL RLRL  │ │  ┌────────────────────────────────────────────┐│
│ │60 BPM     │ │  │  🎯 Strike: +8 ms                          ││
│ └───────────┘ │  │  ✓ Strike: -12 ms                         ││
│               │  │  📊 Progress: 1/20                          ││
│ [▶ Start]     │  │  ✓ Strike: +15 ms                         ││
│ [⏸ Pause]     │  │  ...                                        ││
│ [🔄 Reset]    │  └────────────────────────────────────────────┘│
│ [⏹ Stop]      │                                                  │
│               │                                                  │
│ Rep: 1/20     │                                                  │
│ [████░░░░░░░░]│                                                  │
│ Attempts: 0   │                                                  │
└───────────────┴──────────────────────────────────────────────────┤
│ Status: Session active - Play now!                               │
└────────────────────────────────────────────────────────────────┘
```

---

**Enjoy practicing with professional visual feedback!** 🥁
