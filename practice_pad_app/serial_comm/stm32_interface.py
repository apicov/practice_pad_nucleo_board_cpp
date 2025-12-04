"""
STM32 Serial Interface for Practice Pad
Handles bidirectional communication with STM32 firmware
"""

import serial
import threading
import queue
import time
from typing import Callable, Optional, Dict


class STM32Interface:
    """Interface to communicate with STM32 practice pad via serial"""

    def __init__(self, port: str = '/dev/ttyACM0', baudrate: int = 115200):
        """
        Initialize serial interface

        Args:
            port: Serial port (e.g., '/dev/ttyACM0' on Linux, 'COM3' on Windows)
            baudrate: Communication speed (default 115200)
        """
        self.port = port
        self.baudrate = baudrate
        self.serial = None
        self.running = False
        self.read_thread = None
        self.message_queue = queue.Queue()
        self.event_callbacks = {}  # event_type -> callback function

    def connect(self) -> bool:
        """
        Connect to STM32

        Returns:
            True if connection successful
        """
        try:
            self.serial = serial.Serial(
                self.port,
                self.baudrate,
                timeout=0.1,
                write_timeout=1.0
            )
            time.sleep(2)  # Wait for STM32 to reset after serial connection
            self.running = True
            self.read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self.read_thread.start()
            print(f"✓ Connected to {self.port} at {self.baudrate} baud")
            return True
        except serial.SerialException as e:
            print(f"✗ Connection failed: {e}")
            return False

    def disconnect(self):
        """Disconnect from STM32"""
        self.running = False
        if self.read_thread:
            self.read_thread.join(timeout=1.0)
        if self.serial:
            self.serial.close()
            print("Disconnected")

    def send_command(self, command: str, timeout: float = 2.0) -> Optional[str]:
        """
        Send command and wait for response

        Args:
            command: Command string (without newline)
            timeout: Max time to wait for response (seconds)

        Returns:
            Response string, or None if timeout
        """
        if not self.serial or not self.serial.is_open:
            print("Error: Not connected")
            return None

        try:
            # Clear any old responses
            while not self.message_queue.empty():
                self.message_queue.get_nowait()

            # Send command
            cmd_bytes = f"{command}\n".encode('utf-8')
            self.serial.write(cmd_bytes)
            self.serial.flush()

            # Wait for response (CMD_OK, CMD_ERROR, or CMD_RESP)
            start_time = time.time()
            while time.time() - start_time < timeout:
                if not self.message_queue.empty():
                    msg = self.message_queue.get()
                    if msg.startswith("CMD_"):
                        return msg
                time.sleep(0.01)

            print(f"Timeout waiting for response to: {command}")
            return None

        except Exception as e:
            print(f"Error sending command: {e}")
            return None

    def register_event_callback(self, event_type: str, callback: Callable):
        """
        Register callback for real-time events

        Args:
            event_type: Event type (e.g., "STRIKE", "REP")
            callback: Function to call with event data
        """
        self.event_callbacks[event_type] = callback

    def _read_loop(self):
        """Background thread to continuously read from serial port"""
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

                        if line:
                            if line.startswith("EVENT "):
                                self._handle_event(line)
                            else:
                                # Command response or console output
                                self.message_queue.put(line)

            except Exception as e:
                if self.running:  # Only print if not intentionally stopped
                    print(f"Read error: {e}")

            time.sleep(0.001)  # Small delay to prevent CPU spinning

    def _handle_event(self, line: str):
        """Handle real-time event from STM32"""
        parts = line.split()
        if len(parts) >= 2:
            event_type = parts[1]  # "STRIKE", "REP", etc.
            event_data = parts[2:]  # Remaining data

            if event_type in self.event_callbacks:
                try:
                    self.event_callbacks[event_type](event_data)
                except Exception as e:
                    print(f"Error in event callback: {e}")

    # ========================================================================
    # High-level command methods
    # ========================================================================

    def ping(self) -> bool:
        """Test connection"""
        response = self.send_command("PING")
        return response and "PONG" in response

    def set_bpm(self, bpm: int) -> bool:
        """Set metronome tempo"""
        response = self.send_command(f"SET_BPM {bpm}")
        return response and "CMD_OK" in response

    def get_bpm(self) -> Optional[int]:
        """Get current metronome tempo"""
        response = self.send_command("GET_BPM")
        if response and "BPM" in response:
            try:
                return int(response.split()[-1])
            except:
                pass
        return None

    def load_exercise(self, exercise_id: int, tempo: int) -> Optional[Dict]:
        """
        Load an exercise

        Returns:
            Dict with exercise info, or None if failed
        """
        response = self.send_command(f"LOAD_EXERCISE {exercise_id} {tempo}")
        if response and "CMD_OK" in response:
            # Parse response: "CMD_OK EXERCISE:1 PATTERN:RLRL... NAME:Basic TEMPO:60"
            info = {}
            for part in response.split():
                if ':' in part:
                    key, value = part.split(':', 1)
                    info[key] = value
            return info
        return None

    def start_session(self) -> bool:
        """Start practice session"""
        response = self.send_command("START_SESSION")
        return response and "CMD_OK" in response

    def pause_session(self) -> bool:
        """Pause practice session"""
        response = self.send_command("PAUSE_SESSION")
        return response and "CMD_OK" in response

    def resume_session(self) -> bool:
        """Resume practice session"""
        response = self.send_command("RESUME_SESSION")
        return response and "CMD_OK" in response

    def reset_session(self) -> bool:
        """Reset session (back to 0/20)"""
        response = self.send_command("RESET_SESSION")
        return response and "CMD_OK" in response

    def abort_session(self) -> bool:
        """Abort practice session"""
        response = self.send_command("ABORT_SESSION")
        return response and "CMD_OK" in response

    def get_session_state(self) -> Optional[Dict]:
        """
        Get current session state

        Returns:
            Dict with state, rep count, attempts, duration
        """
        response = self.send_command("GET_SESSION_STATE")
        if response and "CMD_RESP" in response:
            # Parse: "CMD_RESP STATE:2 REP:5/20 ATTEMPTS:3 DURATION:12345"
            state = {}
            for part in response.split():
                if ':' in part:
                    key, value = part.split(':', 1)
                    state[key] = value
            return state
        return None

    def start_metronome(self) -> bool:
        """Start metronome"""
        response = self.send_command("START")
        return response and "CMD_OK" in response

    def stop_metronome(self) -> bool:
        """Stop metronome"""
        response = self.send_command("STOP")
        return response and "CMD_OK" in response

    def get_status(self) -> Optional[Dict]:
        """Get metronome status"""
        response = self.send_command("STATUS")
        if response and "CMD_RESP" in response:
            status = {}
            for part in response.split():
                if ':' in part:
                    key, value = part.split(':', 1)
                    status[key] = value
            return status
        return None
