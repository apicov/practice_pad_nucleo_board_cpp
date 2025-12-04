#!/usr/bin/env python3
"""Test UART communication - monitor what's actually received"""

import serial
import time

port = '/dev/ttyACM0'
baudrate = 115200

print(f"Opening {port} at {baudrate} baud...")
ser = serial.Serial(port, baudrate, timeout=0.1)
print("Waiting 2 seconds for STM32 reset...")
time.sleep(2)

# Clear any startup data
ser.reset_input_buffer()
ser.reset_output_buffer()

print("\n=== Test 1: Send PING and wait 3 seconds ===")
ser.write(b"PING\n")
ser.flush()
print("Sent: PING\\n")

start_time = time.time()
received = b""
while time.time() - start_time < 3:
    if ser.in_waiting:
        data = ser.read(ser.in_waiting)
        received += data
        print(f"Received {len(data)} bytes: {data}")
    time.sleep(0.01)

print(f"\nTotal received: {received}")
print(f"As string: {received.decode('utf-8', errors='ignore')}")

print("\n=== Test 2: Send STATUS ===")
ser.write(b"STATUS\n")
ser.flush()
print("Sent: STATUS\\n")

start_time = time.time()
received = b""
while time.time() - start_time < 3:
    if ser.in_waiting:
        data = ser.read(ser.in_waiting)
        received += data
        print(f"Received {len(data)} bytes: {data}")
    time.sleep(0.01)

print(f"\nTotal received: {received}")
print(f"As string: {received.decode('utf-8', errors='ignore')}")

print("\n=== Done ===")
ser.close()
