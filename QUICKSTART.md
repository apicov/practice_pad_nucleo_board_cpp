# Quick Start Guide

Get up and running in 5 minutes!

## Step 1: Build & Flash Firmware (2 min)

```bash
# Build
cmake --build build/Debug

# Flash to STM32 (choose your method):
# Option A: STM32CubeProgrammer GUI
# Option B: Command line
st-flash write build/Debug/practice_pad_nucleo_board_cpp.bin 0x8000000
```

## Step 2: Setup Python App (1 min)

```bash
cd practice_pad_app
pip install -r requirements.txt
chmod +x practice.py
```

## Step 3: Run & Practice! (2 min)

```bash
# Linux/Mac
./practice.py

# Windows
python practice.py --port COM3
```

### First Practice Session

1. Choose option `1` (Start Practice Session)
2. Enter `1` for Exercise ID (first exercise)
3. Enter `60` for tempo (slow, beginner-friendly)
4. Press ENTER to start
5. **Play:** Right-Left-Right-Left (RLRL) 4 times = 1 repetition
6. **Repeat 20 times perfectly** to complete!

## Troubleshooting

**Can't connect?**
```bash
# Linux: Find your port
ls /dev/ttyACM*

# Then run with correct port
./practice.py --port /dev/ttyACM0
```

**Permission denied (Linux)?**
```bash
sudo usermod -a -G dialout $USER
# Log out and back in
```

**Still issues?**
- Check STM32 is connected via USB
- Try different USB port
- Reset STM32 (press reset button)
- Make sure no other program is using the serial port

## What You'll See

```
===========================================================
PRACTICE SESSION
===========================================================

Exercise ID (1-320): 1
Tempo (40-200 BPM): 60

✓ Loaded: Basic Alternating
  Pattern: RLRL RLRL RLRL RLRL

🎯 STARTING SESSION...

📊 Progress: 1/20
  ✓ Strike: +12 ms    ← Your timing feedback
  🎯 Strike: -8 ms
  ✓ Strike: +15 ms
  ✓ Strike: -18 ms

📊 Progress: 2/20
  ...

[On mistake: ⚠ RESET TO 0! Keep trying...]

📊 Progress: 20/20

🎉 SUCCESS! EXERCISE COMPLETED!
```

## Tips for Success

- **Start slow** (60 BPM) - build good habits
- **Stay relaxed** - tension kills technique
- **Expect resets** - that's how you learn!
- **Practice daily** - 15-30 minutes minimum

## Next Steps

After mastering Exercise 1 at 60 BPM:
- Try 80 BPM (faster!)
- Try Exercise 2 (different pattern)
- Explore other exercises (1-320 available)

## Documentation

- **Full system details:** [IMPLEMENTATION_COMPLETE.md](IMPLEMENTATION_COMPLETE.md)
- **Python app guide:** [practice_pad_app/README.md](practice_pad_app/README.md)
- **Implementation plan:** [docs/implementation_plan.md](docs/implementation_plan.md)
- **Learning system:** [docs/stick_control_learning_system_plan.md](docs/stick_control_learning_system_plan.md)

---

**Ready? Let's practice! 🥁**
