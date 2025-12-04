#!/usr/bin/env python3
"""Quick connection test with debug output"""

import sys
sys.path.insert(0, '/home/pico/code/practice_pad_nucleo_board_cpp/practice_pad_app')

from serial_comm.stm32_interface import STM32Interface
import time

print("Creating STM32Interface...")
stm32 = STM32Interface(port='/dev/ttyACM0')

print("Connecting...")
if stm32.connect():
    print("✓ Connection established")

    print("\nWaiting 3 seconds for STM32 to be ready...")
    time.sleep(3)

    print("\nSending PING command...")
    response = stm32.send_command("PING", timeout=5.0)
    print(f"Response: {response}")

    if response and "PONG" in response:
        print("✓ PING successful!")
    else:
        print("✗ PING failed - device not responding properly")
        print(f"  Expected response with 'PONG', got: {response}")

    print("\nDisconnecting...")
    stm32.disconnect()
else:
    print("✗ Connection failed")
