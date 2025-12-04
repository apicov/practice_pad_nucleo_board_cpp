#!/usr/bin/env python3
"""Raw serial monitor - see what STM32 is actually sending"""

import serial
import time

port = '/dev/ttyACM0'
baudrate = 115200

print(f"Opening {port} at {baudrate} baud...")
ser = serial.Serial(port, baudrate, timeout=0.1)
print("Waiting 2 seconds for STM32 reset...")
time.sleep(2)

print("\n=== Monitoring serial output for 5 seconds ===")
print("(Any startup messages from STM32 will appear here)\n")

start_time = time.time()
while time.time() - start_time < 5:
    if ser.in_waiting:
        data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        print(data, end='', flush=True)
    time.sleep(0.01)

print("\n\n=== Sending PING command ===")
ser.write(b"PING\n")
ser.flush()
print("Sent: PING\\n")

print("\n=== Waiting for response (5 seconds) ===\n")
start_time = time.time()
while time.time() - start_time < 5:
    if ser.in_waiting:
        data = ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        print(data, end='', flush=True)
    time.sleep(0.01)

print("\n\n=== Done ===")
ser.close()
