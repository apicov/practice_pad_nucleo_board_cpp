#!/usr/bin/env python3
"""
Smart Practice Pad Interface
A PyQt5 application for controlling the STM32-based drum practice pad
"""

import sys
import time
import threading
import sqlite3
import json
from datetime import datetime, timedelta
from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                             QWidget, QPushButton, QLabel, QSpinBox, QComboBox,
                             QTextEdit, QGroupBox, QGridLayout, QProgressBar, QFrame,
                             QTableWidget, QTableWidgetItem, QTabWidget, QHeaderView)
from PyQt5.QtCore import QTimer, pyqtSignal, QObject, pyqtSlot, Qt
from PyQt5.QtGui import QFont, QPalette, QColor
import serial
import serial.tools.list_ports


class PracticeDatabase:
    """Manages persistent storage of practice sessions and progress data"""
    
    def __init__(self, db_path="practice_pad_data.db"):
        self.db_path = db_path
        self.init_database()
    
    def init_database(self):
        """Initialize database tables if they don't exist"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Practice sessions table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS practice_sessions (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                start_time TEXT NOT NULL,
                end_time TEXT,
                duration_seconds INTEGER,
                bpm INTEGER,
                total_strikes INTEGER DEFAULT 0,
                accurate_strikes INTEGER DEFAULT 0,
                early_strikes INTEGER DEFAULT 0,
                late_strikes INTEGER DEFAULT 0,
                accuracy_percentage REAL DEFAULT 0.0,
                avg_timing_offset INTEGER DEFAULT 0,
                notes TEXT,
                created_at TEXT DEFAULT CURRENT_TIMESTAMP
            )
        ''')
        
        # Daily practice summary table
        cursor.execute('''
            CREATE TABLE IF NOT EXISTS daily_summary (
                date TEXT PRIMARY KEY,
                total_practice_time INTEGER DEFAULT 0,
                total_sessions INTEGER DEFAULT 0,
                best_accuracy REAL DEFAULT 0.0,
                avg_accuracy REAL DEFAULT 0.0,
                total_strikes INTEGER DEFAULT 0,
                improvement_score REAL DEFAULT 0.0
            )
        ''')
        
        conn.commit()
        conn.close()
    
    def save_session(self, session_data):
        """Save a complete practice session"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            INSERT INTO practice_sessions 
            (start_time, end_time, duration_seconds, bpm, total_strikes, 
             accurate_strikes, early_strikes, late_strikes, accuracy_percentage, 
             avg_timing_offset, notes)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        ''', (
            session_data['start_time'],
            session_data['end_time'], 
            session_data['duration_seconds'],
            session_data['bpm'],
            session_data['total_strikes'],
            session_data['accurate_strikes'],
            session_data['early_strikes'],
            session_data['late_strikes'],
            session_data['accuracy_percentage'],
            session_data['avg_timing_offset'],
            session_data.get('notes', '')
        ))
        
        session_id = cursor.lastrowid
        conn.commit()
        conn.close()
        
        # Update daily summary
        self.update_daily_summary(session_data['start_time'][:10], session_data)
        
        return session_id
    
    def update_daily_summary(self, date, session_data):
        """Update daily practice summary statistics"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Get existing daily data
        cursor.execute('SELECT * FROM daily_summary WHERE date = ?', (date,))
        existing = cursor.fetchone()
        
        if existing:
            # Update existing record
            new_total_time = existing[1] + session_data['duration_seconds']
            new_session_count = existing[2] + 1
            new_best_accuracy = max(existing[3], session_data['accuracy_percentage'])
            new_avg_accuracy = ((existing[4] * existing[2]) + session_data['accuracy_percentage']) / new_session_count
            new_total_strikes = existing[5] + session_data['total_strikes']
            
            cursor.execute('''
                UPDATE daily_summary 
                SET total_practice_time = ?, total_sessions = ?, best_accuracy = ?, 
                    avg_accuracy = ?, total_strikes = ?
                WHERE date = ?
            ''', (new_total_time, new_session_count, new_best_accuracy, 
                  new_avg_accuracy, new_total_strikes, date))
        else:
            # Create new record
            cursor.execute('''
                INSERT INTO daily_summary 
                (date, total_practice_time, total_sessions, best_accuracy, 
                 avg_accuracy, total_strikes)
                VALUES (?, ?, ?, ?, ?, ?)
            ''', (date, session_data['duration_seconds'], 1, 
                  session_data['accuracy_percentage'], session_data['accuracy_percentage'], 
                  session_data['total_strikes']))
        
        conn.commit()
        conn.close()
    
    def get_recent_sessions(self, limit=10):
        """Get recent practice sessions"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT * FROM practice_sessions 
            ORDER BY start_time DESC 
            LIMIT ?
        ''', (limit,))
        
        sessions = cursor.fetchall()
        conn.close()
        return sessions
    
    def get_daily_summaries(self, days=30):
        """Get daily summaries for the last N days"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        cursor.execute('''
            SELECT * FROM daily_summary 
            ORDER BY date DESC 
            LIMIT ?
        ''', (days,))
        
        summaries = cursor.fetchall()
        conn.close()
        return summaries
    
    def get_progress_stats(self):
        """Get overall progress statistics"""
        conn = sqlite3.connect(self.db_path)
        cursor = conn.cursor()
        
        # Total sessions and practice time
        cursor.execute('SELECT COUNT(*), SUM(duration_seconds) FROM practice_sessions')
        total_sessions, total_time = cursor.fetchone()
        
        # Best accuracy
        cursor.execute('SELECT MAX(accuracy_percentage) FROM practice_sessions')
        best_accuracy = cursor.fetchone()[0] or 0
        
        # Recent accuracy trend (last 10 sessions)
        cursor.execute('''
            SELECT accuracy_percentage FROM practice_sessions 
            ORDER BY start_time DESC 
            LIMIT 10
        ''')
        recent_accuracies = [row[0] for row in cursor.fetchall()]
        
        conn.close()
        
        return {
            'total_sessions': total_sessions or 0,
            'total_time_hours': (total_time or 0) / 3600,
            'best_accuracy': best_accuracy,
            'recent_avg_accuracy': sum(recent_accuracies) / len(recent_accuracies) if recent_accuracies else 0
        }


class SerialController(QObject):
    """Handles serial communication with the STM32"""
    
    status_updated = pyqtSignal(str)
    response_received = pyqtSignal(str)
    connection_changed = pyqtSignal(bool)
    
    def __init__(self):
        super().__init__()
        self.serial_port = None
        self.is_connected = False
        
    def list_ports(self):
        """Return list of available serial ports"""
        ports = serial.tools.list_ports.comports()
        return [port.device for port in ports]
    
    def connect(self, port, baudrate=230400):
        """Connect to serial port"""
        try:
            if self.serial_port and self.serial_port.is_open:
                self.serial_port.close()
                
            self.serial_port = serial.Serial(port, baudrate, timeout=1)
            time.sleep(2)  # Wait for connection to stabilize
            
            # Flush any pending data
            self.serial_port.reset_input_buffer()
            self.serial_port.reset_output_buffer()
            
            self.is_connected = True
            self.connection_changed.emit(True)
            self.status_updated.emit(f"Connected to {port}")
            return True
        except Exception as e:
            self.status_updated.emit(f"Connection failed: {str(e)}")
            self.connection_changed.emit(False)
            return False
    
    def disconnect(self):
        """Disconnect from serial port"""
        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()
        self.is_connected = False
        self.connection_changed.emit(False)
        self.status_updated.emit("Disconnected")
    
    def send_command(self, command):
        """Send command to STM32 and return response"""
        if not self.is_connected or not self.serial_port:
            return "ERROR: Not connected"
        
        try:
            # Aggressive buffer clearing
            self.serial_port.reset_input_buffer()
            self.serial_port.reset_output_buffer()
            time.sleep(0.05)  # Shorter delay
            
            # Send command with explicit encoding
            cmd_bytes = f"{command}\r\n".encode('utf-8')  # Use \r\n for better compatibility
            self.serial_port.write(cmd_bytes)
            self.serial_port.flush()
            
            # More robust response reading
            start_time = time.time()
            response_lines = []
            current_line = ""
            
            while time.time() - start_time < 3.0:  # Slightly longer timeout
                if self.serial_port.in_waiting > 0:
                    try:
                        # Read available data in chunks for efficiency
                        chunk = self.serial_port.read(self.serial_port.in_waiting)
                        if chunk:
                            text = chunk.decode('utf-8', errors='replace')
                            
                            for char in text:
                                if char in ['\n', '\r']:
                                    if current_line.strip():
                                        line = current_line.strip()
                                        response_lines.append(line)
                                        
                                        # Check for command response
                                        if line.startswith('CMD_'):
                                            self.response_received.emit(f"CMD: {command} -> {line}")
                                            return line
                                        
                                        # Also log debug output from microcontroller
                                        if not line.startswith('CMD_'):
                                            self.status_updated.emit(f"MCU Debug: {line}")
                                    
                                    current_line = ""
                                else:
                                    current_line += char
                                    
                    except UnicodeDecodeError as e:
                        self.status_updated.emit(f"Decode error: {e}")
                        continue
                
                time.sleep(0.005)  # Shorter polling interval
            
            # Handle timeout
            if current_line.strip():
                response_lines.append(current_line.strip())
            
            if response_lines:
                # Return the last line that might be a response
                for line in reversed(response_lines):
                    if line.startswith('CMD_'):
                        return line
                # If no CMD_ line found, return last non-empty line
                self.status_updated.emit(f"Timeout - got lines: {response_lines}")
                return f"ERROR: Timeout - last: {response_lines[-1] if response_lines else 'none'}"
            
            return "ERROR: No response"
                
        except Exception as e:
            error_msg = f"Communication error: {str(e)}"
            self.status_updated.emit(error_msg)
            return error_msg


class MetronomeWidget(QWidget):
    """Widget for controlling metronome settings"""
    
    def __init__(self, serial_controller):
        super().__init__()
        self.serial_controller = serial_controller
        self.init_ui()
        
    def init_ui(self):
        layout = QVBoxLayout()
        
        # BPM Control
        bpm_group = QGroupBox("Tempo (BPM)")
        bpm_layout = QHBoxLayout()
        
        self.bpm_spinbox = QSpinBox()
        self.bpm_spinbox.setRange(40, 200)
        self.bpm_spinbox.setValue(60)
        self.bpm_spinbox.setSuffix(" BPM")
        
        self.set_bpm_btn = QPushButton("Set BPM")
        self.set_bpm_btn.clicked.connect(self.set_bpm)
        
        self.get_bpm_btn = QPushButton("Get BPM")
        self.get_bpm_btn.clicked.connect(self.get_bpm)
        
        bpm_layout.addWidget(self.bpm_spinbox)
        bpm_layout.addWidget(self.set_bpm_btn)
        bpm_layout.addWidget(self.get_bpm_btn)
        bpm_group.setLayout(bpm_layout)
        
        # Control Buttons
        control_group = QGroupBox("Control")
        control_layout = QHBoxLayout()
        
        self.start_btn = QPushButton("Start")
        self.start_btn.clicked.connect(self.start_metronome)
        self.start_btn.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }")
        
        self.stop_btn = QPushButton("Stop")
        self.stop_btn.clicked.connect(self.stop_metronome)
        self.stop_btn.setStyleSheet("QPushButton { background-color: #f44336; color: white; }")
        
        self.status_btn = QPushButton("Status")
        self.status_btn.clicked.connect(self.get_status)
        
        control_layout.addWidget(self.start_btn)
        control_layout.addWidget(self.stop_btn)
        control_layout.addWidget(self.status_btn)
        control_group.setLayout(control_layout)
        
        # Status Display
        status_group = QGroupBox("Status")
        status_layout = QVBoxLayout()
        
        self.status_label = QLabel("Not connected")
        self.status_label.setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; }")
        
        status_layout.addWidget(self.status_label)
        status_group.setLayout(status_layout)
        
        layout.addWidget(bpm_group)
        layout.addWidget(control_group)
        layout.addWidget(status_group)
        
        self.setLayout(layout)
    
    def set_bpm(self):
        bpm = self.bpm_spinbox.value()
        response = self.serial_controller.send_command(f"SET_BPM {bpm}")
        self.update_status(f"Set BPM to {bpm}: {response}")
    
    def get_bpm(self):
        response = self.serial_controller.send_command("GET_BPM")
        self.update_status(f"Current BPM: {response}")
        
        # Try to extract BPM value and update spinbox
        try:
            if "BPM" in response:
                bpm_value = int(response.split()[1])
                self.bpm_spinbox.setValue(bpm_value)
        except:
            pass
    
    def start_metronome(self):
        response = self.serial_controller.send_command("START")
        self.update_status(f"Start metronome: {response}")
    
    def stop_metronome(self):
        response = self.serial_controller.send_command("STOP")
        self.update_status(f"Stop metronome: {response}")
    
    def get_status(self):
        response = self.serial_controller.send_command("STATUS")
        self.update_status(f"Status: {response}")
    
    def update_status(self, message):
        self.status_label.setText(message)


class PracticeWidget(QWidget):
    """Widget for practice sessions and analytics"""
    
    def __init__(self, serial_controller, database):
        super().__init__()
        self.serial_controller = serial_controller
        self.database = database
        self.practice_active = False
        self.session_start_time = None
        self.current_bpm = 60
        self.init_ui()
        
        # Auto-refresh timer for stats
        self.stats_timer = QTimer()
        self.stats_timer.timeout.connect(self.refresh_stats)
        
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Practice Session Control
        session_group = QGroupBox("Practice Session")
        session_layout = QHBoxLayout()
        
        self.start_practice_btn = QPushButton("Start Practice")
        self.start_practice_btn.clicked.connect(self.start_practice)
        self.start_practice_btn.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; font-weight: bold; }")
        
        self.stop_practice_btn = QPushButton("Stop Practice")
        self.stop_practice_btn.clicked.connect(self.stop_practice)
        self.stop_practice_btn.setStyleSheet("QPushButton { background-color: #f44336; color: white; font-weight: bold; }")
        self.stop_practice_btn.setEnabled(False)
        
        self.reset_stats_btn = QPushButton("Reset Stats")
        self.reset_stats_btn.clicked.connect(self.reset_stats)
        
        session_layout.addWidget(self.start_practice_btn)
        session_layout.addWidget(self.stop_practice_btn)
        session_layout.addWidget(self.reset_stats_btn)
        session_group.setLayout(session_layout)
        
        # Statistics Display
        stats_group = QGroupBox("Real-time Statistics")
        stats_layout = QGridLayout()
        
        # Labels for statistics
        stats_layout.addWidget(QLabel("Total Strikes:"), 0, 0)
        self.total_strikes_label = QLabel("0")
        self.total_strikes_label.setStyleSheet("QLabel { font-weight: bold; color: #333; }")
        stats_layout.addWidget(self.total_strikes_label, 0, 1)
        
        stats_layout.addWidget(QLabel("Accurate (±50ms):"), 1, 0)
        self.accurate_label = QLabel("0")
        self.accurate_label.setStyleSheet("QLabel { font-weight: bold; color: #4CAF50; }")
        stats_layout.addWidget(self.accurate_label, 1, 1)
        
        stats_layout.addWidget(QLabel("Early Strikes:"), 2, 0)
        self.early_label = QLabel("0")
        self.early_label.setStyleSheet("QLabel { font-weight: bold; color: #FF9800; }")
        stats_layout.addWidget(self.early_label, 2, 1)
        
        stats_layout.addWidget(QLabel("Late Strikes:"), 3, 0)
        self.late_label = QLabel("0")
        self.late_label.setStyleSheet("QLabel { font-weight: bold; color: #2196F3; }")
        stats_layout.addWidget(self.late_label, 3, 1)
        
        stats_layout.addWidget(QLabel("Accuracy:"), 0, 2)
        self.accuracy_label = QLabel("0%")
        self.accuracy_label.setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: #4CAF50; }")
        stats_layout.addWidget(self.accuracy_label, 0, 3)
        
        stats_layout.addWidget(QLabel("Avg Offset:"), 1, 2)
        self.avg_offset_label = QLabel("0ms")
        self.avg_offset_label.setStyleSheet("QLabel { font-weight: bold; color: #666; }")
        stats_layout.addWidget(self.avg_offset_label, 1, 3)
        
        stats_group.setLayout(stats_layout)
        
        # Practice Status
        status_group = QGroupBox("Practice Status")
        status_layout = QVBoxLayout()
        
        self.practice_status_label = QLabel("Practice session not active")
        self.practice_status_label.setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; font-size: 12px; }")
        
        status_layout.addWidget(self.practice_status_label)
        status_group.setLayout(status_layout)
        
        layout.addWidget(session_group)
        layout.addWidget(stats_group) 
        layout.addWidget(status_group)
        
        self.setLayout(layout)
        
    def start_practice(self):
        response = self.serial_controller.send_command("START_PRACTICE")
        if "OK" in response:
            self.practice_active = True
            self.session_start_time = datetime.now()
            
            # Get current BPM for session tracking
            bpm_response = self.serial_controller.send_command("GET_BPM")
            try:
                if "BPM" in bpm_response:
                    self.current_bpm = int(bpm_response.split()[1])
            except (IndexError, ValueError):
                self.current_bpm = 60  # Default BPM
            
            self.start_practice_btn.setEnabled(False)
            self.stop_practice_btn.setEnabled(True)
            self.practice_status_label.setText("🎯 Practice session ACTIVE - Start playing!")
            self.practice_status_label.setStyleSheet("QLabel { padding: 10px; background-color: #E8F5E8; color: #2E7D32; font-weight: bold; }")
            
            # Start auto-refreshing stats every 3 seconds (slower to reduce UART congestion)
            self.stats_timer.start(3000)  
            self.refresh_stats()
    
    def stop_practice(self):
        response = self.serial_controller.send_command("STOP_PRACTICE")
        if "OK" in response:
            self.practice_active = False
            self.start_practice_btn.setEnabled(True)
            self.stop_practice_btn.setEnabled(False)
            self.practice_status_label.setText("Practice session stopped. Review your stats!")
            self.practice_status_label.setStyleSheet("QLabel { padding: 10px; background-color: #FFF3E0; color: #F57C00; }")
            
            # Stop auto-refresh and get final stats
            self.stats_timer.stop()
            self.refresh_stats()
            
            # Save session to database
            self.save_current_session()
    
    def reset_stats(self):
        response = self.serial_controller.send_command("RESET_STATS")
        if "OK" in response:
            # Clear all displayed stats
            self.total_strikes_label.setText("0")
            self.accurate_label.setText("0")
            self.early_label.setText("0")
            self.late_label.setText("0")
            self.accuracy_label.setText("0%")
            self.avg_offset_label.setText("0ms")
            self.practice_status_label.setText("Statistics reset - ready for new session!")
    
    def refresh_stats(self):
        response = self.serial_controller.send_command("GET_STATS")
        print(f"DEBUG: GET_STATS response: '{response}'")
        if response.startswith("CMD_RESP STATS"):
            try:
                # Parse: CMD_RESP STATS TOTAL:10 ACC:8 EARLY:1 LATE:1 ACCURACY:80% AVGOFF:5
                parts = response.split()
                
                total = parts[2].split(':')[1]
                accurate = parts[3].split(':')[1] 
                early = parts[4].split(':')[1]
                late = parts[5].split(':')[1]
                accuracy = parts[6].split(':')[1]
                avg_offset = parts[7].split(':')[1]
                
                # Update labels
                self.total_strikes_label.setText(total)
                self.accurate_label.setText(f"{accurate} ({accuracy})")
                self.early_label.setText(early)
                self.late_label.setText(late)
                self.accuracy_label.setText(accuracy)
                self.avg_offset_label.setText(f"{avg_offset}ms")
                
            except (IndexError, ValueError) as e:
                print(f"Error parsing stats: {e}")
    
    def save_current_session(self):
        """Save the current practice session to database"""
        if not self.session_start_time:
            return
        
        # Get final stats from the device
        stats_response = self.serial_controller.send_command("GET_STATS")
        
        if stats_response.startswith("CMD_RESP STATS"):
            try:
                # Parse final statistics
                parts = stats_response.split()
                total_strikes = int(parts[2].split(':')[1])
                accurate_strikes = int(parts[3].split(':')[1])
                early_strikes = int(parts[4].split(':')[1])
                late_strikes = int(parts[5].split(':')[1])
                accuracy_str = parts[6].split(':')[1].replace('%', '')
                accuracy_percentage = float(accuracy_str)
                avg_offset = int(parts[7].split(':')[1])
                
                # Calculate session duration
                end_time = datetime.now()
                duration = (end_time - self.session_start_time).total_seconds()
                
                # Create session data dictionary
                session_data = {
                    'start_time': self.session_start_time.isoformat(),
                    'end_time': end_time.isoformat(),
                    'duration_seconds': int(duration),
                    'bpm': self.current_bpm,
                    'total_strikes': total_strikes,
                    'accurate_strikes': accurate_strikes,
                    'early_strikes': early_strikes,
                    'late_strikes': late_strikes,
                    'accuracy_percentage': accuracy_percentage,
                    'avg_timing_offset': avg_offset,
                    'notes': ''
                }
                
                # Save to database
                session_id = self.database.save_session(session_data)
                print(f"Practice session saved with ID: {session_id}")
                
                # Update status with session summary
                duration_str = f"{int(duration//60)}:{int(duration%60):02d}"
                self.practice_status_label.setText(
                    f"Session saved! Duration: {duration_str}, Accuracy: {accuracy_percentage:.1f}%"
                )
                
            except (IndexError, ValueError) as e:
                print(f"Error saving session: {e}")
                self.practice_status_label.setText("Session completed but could not save data")


class ConnectionWidget(QWidget):
    """Widget for managing serial connection"""
    
    def __init__(self, serial_controller):
        super().__init__()
        self.serial_controller = serial_controller
        self.init_ui()
        self.refresh_ports()
        
        # Connect signals
        self.serial_controller.connection_changed.connect(self.on_connection_changed)
        
    def init_ui(self):
        layout = QHBoxLayout()
        
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
        
        self.refresh_btn = QPushButton("Refresh")
        self.refresh_btn.clicked.connect(self.refresh_ports)
        
        self.connect_btn = QPushButton("Connect")
        self.connect_btn.clicked.connect(self.toggle_connection)
        
        self.ping_btn = QPushButton("Ping")
        self.ping_btn.clicked.connect(self.ping_device)
        self.ping_btn.setEnabled(False)
        
        layout.addWidget(QLabel("Port:"))
        layout.addWidget(self.port_combo)
        layout.addWidget(self.refresh_btn)
        layout.addWidget(self.connect_btn)
        layout.addWidget(self.ping_btn)
        layout.addStretch()
        
        self.setLayout(layout)
    
    def refresh_ports(self):
        self.port_combo.clear()
        ports = self.serial_controller.list_ports()
        self.port_combo.addItems(ports)
    
    def toggle_connection(self):
        if self.serial_controller.is_connected:
            self.serial_controller.disconnect()
        else:
            port = self.port_combo.currentText()
            if port:
                self.serial_controller.connect(port)
    
    def ping_device(self):
        response = self.serial_controller.send_command("PING")
        print(f"Ping response: {response}")
        
    @pyqtSlot(bool)
    def on_connection_changed(self, connected):
        if connected:
            self.connect_btn.setText("Disconnect")
            self.connect_btn.setStyleSheet("QPushButton { background-color: #f44336; color: white; }")
            self.ping_btn.setEnabled(True)
        else:
            self.connect_btn.setText("Connect")
            self.connect_btn.setStyleSheet("QPushButton { background-color: #4CAF50; color: white; }")
            self.ping_btn.setEnabled(False)


class ProgressWidget(QWidget):
    """Widget for displaying practice progress and history"""
    
    def __init__(self, database):
        super().__init__()
        self.database = database
        self.init_ui()
        
        # Auto-refresh timer
        self.refresh_timer = QTimer()
        self.refresh_timer.timeout.connect(self.refresh_data)
        self.refresh_timer.start(10000)  # Refresh every 10 seconds
        
        # Initial load
        self.refresh_data()
        
    def init_ui(self):
        layout = QVBoxLayout()
        
        # Overall progress statistics
        stats_group = QGroupBox("Overall Progress")
        stats_layout = QGridLayout()
        
        # Progress labels
        stats_layout.addWidget(QLabel("Total Sessions:"), 0, 0)
        self.total_sessions_label = QLabel("0")
        self.total_sessions_label.setStyleSheet("QLabel { font-weight: bold; color: #333; }")
        stats_layout.addWidget(self.total_sessions_label, 0, 1)
        
        stats_layout.addWidget(QLabel("Total Practice Time:"), 0, 2)
        self.total_time_label = QLabel("0.0 hours")
        self.total_time_label.setStyleSheet("QLabel { font-weight: bold; color: #333; }")
        stats_layout.addWidget(self.total_time_label, 0, 3)
        
        stats_layout.addWidget(QLabel("Best Accuracy:"), 1, 0)
        self.best_accuracy_label = QLabel("0%")
        self.best_accuracy_label.setStyleSheet("QLabel { font-weight: bold; color: #4CAF50; }")
        stats_layout.addWidget(self.best_accuracy_label, 1, 1)
        
        stats_layout.addWidget(QLabel("Recent Average:"), 1, 2)
        self.recent_avg_label = QLabel("0%")
        self.recent_avg_label.setStyleSheet("QLabel { font-weight: bold; color: #2196F3; }")
        stats_layout.addWidget(self.recent_avg_label, 1, 3)
        
        stats_group.setLayout(stats_layout)
        
        # Recent sessions table
        sessions_group = QGroupBox("Recent Sessions")
        sessions_layout = QVBoxLayout()
        
        self.sessions_table = QTableWidget()
        self.sessions_table.setColumnCount(6)
        self.sessions_table.setHorizontalHeaderLabels([
            "Date", "Duration", "BPM", "Strikes", "Accuracy", "Avg Offset"
        ])
        
        # Make table headers resize to content
        header = self.sessions_table.horizontalHeader()
        header.setSectionResizeMode(QHeaderView.Stretch)
        
        self.sessions_table.setMaximumHeight(200)
        self.sessions_table.setAlternatingRowColors(True)
        
        sessions_layout.addWidget(self.sessions_table)
        sessions_group.setLayout(sessions_layout)
        
        # Daily summary table
        daily_group = QGroupBox("Daily Summary (Last 7 Days)")
        daily_layout = QVBoxLayout()
        
        self.daily_table = QTableWidget()
        self.daily_table.setColumnCount(5)
        self.daily_table.setHorizontalHeaderLabels([
            "Date", "Sessions", "Practice Time", "Best Accuracy", "Avg Accuracy"
        ])
        
        # Make table headers resize to content
        daily_header = self.daily_table.horizontalHeader()
        daily_header.setSectionResizeMode(QHeaderView.Stretch)
        
        self.daily_table.setMaximumHeight(200)
        self.daily_table.setAlternatingRowColors(True)
        
        daily_layout.addWidget(self.daily_table)
        daily_group.setLayout(daily_layout)
        
        # Refresh button
        refresh_btn = QPushButton("Refresh Data")
        refresh_btn.clicked.connect(self.refresh_data)
        
        layout.addWidget(stats_group)
        layout.addWidget(sessions_group)
        layout.addWidget(daily_group)
        layout.addWidget(refresh_btn)
        layout.addStretch()
        
        self.setLayout(layout)
    
    def refresh_data(self):
        """Refresh all progress data from database"""
        try:
            # Update overall progress stats
            progress_stats = self.database.get_progress_stats()
            
            self.total_sessions_label.setText(str(progress_stats['total_sessions']))
            self.total_time_label.setText(f"{progress_stats['total_time_hours']:.1f} hours")
            self.best_accuracy_label.setText(f"{progress_stats['best_accuracy']:.1f}%")
            self.recent_avg_label.setText(f"{progress_stats['recent_avg_accuracy']:.1f}%")
            
            # Update recent sessions table
            recent_sessions = self.database.get_recent_sessions(10)
            self.sessions_table.setRowCount(len(recent_sessions))
            
            for i, session in enumerate(recent_sessions):
                # session format: (id, start_time, end_time, duration_seconds, bpm, 
                #                  total_strikes, accurate_strikes, early_strikes, late_strikes, 
                #                  accuracy_percentage, avg_timing_offset, notes, created_at)
                start_time = datetime.fromisoformat(session[1]).strftime("%m/%d %H:%M")
                duration_str = f"{session[3]//60}:{session[3]%60:02d}"
                
                self.sessions_table.setItem(i, 0, QTableWidgetItem(start_time))
                self.sessions_table.setItem(i, 1, QTableWidgetItem(duration_str))
                self.sessions_table.setItem(i, 2, QTableWidgetItem(str(session[4])))
                self.sessions_table.setItem(i, 3, QTableWidgetItem(str(session[5])))
                self.sessions_table.setItem(i, 4, QTableWidgetItem(f"{session[9]:.1f}%"))
                self.sessions_table.setItem(i, 5, QTableWidgetItem(f"{session[10]}ms"))
            
            # Update daily summary table
            daily_summaries = self.database.get_daily_summaries(7)
            self.daily_table.setRowCount(len(daily_summaries))
            
            for i, day_summary in enumerate(daily_summaries):
                # daily_summary format: (date, total_practice_time, total_sessions, 
                #                        best_accuracy, avg_accuracy, total_strikes, improvement_score)
                date_str = datetime.strptime(day_summary[0], "%Y-%m-%d").strftime("%m/%d")
                time_str = f"{day_summary[1]//60}:{day_summary[1]%60:02d}"
                
                self.daily_table.setItem(i, 0, QTableWidgetItem(date_str))
                self.daily_table.setItem(i, 1, QTableWidgetItem(str(day_summary[2])))
                self.daily_table.setItem(i, 2, QTableWidgetItem(time_str))
                self.daily_table.setItem(i, 3, QTableWidgetItem(f"{day_summary[3]:.1f}%"))
                self.daily_table.setItem(i, 4, QTableWidgetItem(f"{day_summary[4]:.1f}%"))
                
        except Exception as e:
            print(f"Error refreshing progress data: {e}")


class LogWidget(QWidget):
    """Widget for displaying communication log"""
    
    def __init__(self, serial_controller):
        super().__init__()
        self.serial_controller = serial_controller
        self.init_ui()
        
        # Connect signals
        self.serial_controller.status_updated.connect(self.add_log_entry)
        self.serial_controller.response_received.connect(self.add_log_entry)
        
    def init_ui(self):
        layout = QVBoxLayout()
        
        header_layout = QHBoxLayout()
        header_layout.addWidget(QLabel("Communication Log"))
        
        self.clear_btn = QPushButton("Clear")
        self.clear_btn.clicked.connect(self.clear_log)
        header_layout.addWidget(self.clear_btn)
        header_layout.addStretch()
        
        self.log_text = QTextEdit()
        self.log_text.setReadOnly(True)
        self.log_text.setMaximumHeight(200)
        self.log_text.setFont(QFont("Courier", 9))
        
        layout.addLayout(header_layout)
        layout.addWidget(self.log_text)
        
        self.setLayout(layout)
    
    @pyqtSlot(str)
    def add_log_entry(self, message):
        timestamp = time.strftime("%H:%M:%S")
        self.log_text.append(f"[{timestamp}] {message}")
        
        # Auto-scroll to bottom
        scrollbar = self.log_text.verticalScrollBar()
        scrollbar.setValue(scrollbar.maximum())
    
    def clear_log(self):
        self.log_text.clear()


class MainWindow(QMainWindow):
    """Main application window"""
    
    def __init__(self):
        super().__init__()
        self.serial_controller = SerialController()
        self.database = PracticeDatabase()
        self.init_ui()
        
    def init_ui(self):
        self.setWindowTitle("Smart Practice Pad Interface")
        self.setGeometry(100, 100, 800, 600)
        
        # Central widget
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        
        # Main layout
        main_layout = QVBoxLayout()
        
        # Connection widget
        self.connection_widget = ConnectionWidget(self.serial_controller)
        main_layout.addWidget(self.connection_widget)
        
        # Add separator
        separator = QFrame()
        separator.setFrameShape(QFrame.HLine)
        separator.setFrameShadow(QFrame.Sunken)
        main_layout.addWidget(separator)
        
        # Metronome widget
        self.metronome_widget = MetronomeWidget(self.serial_controller)
        
        # Practice widget
        self.practice_widget = PracticeWidget(self.serial_controller, self.database)
        
        # Create tab widget for main content
        tab_widget = QTabWidget()
        
        # Main practice tab
        practice_tab = QWidget()
        practice_layout = QVBoxLayout()
        practice_layout.addWidget(self.metronome_widget)
        practice_layout.addWidget(self.practice_widget)
        practice_tab.setLayout(practice_layout)
        tab_widget.addTab(practice_tab, "Practice")
        
        # Progress tracking tab
        self.progress_widget = ProgressWidget(self.database)
        tab_widget.addTab(self.progress_widget, "Progress")
        
        # Log widget
        self.log_widget = LogWidget(self.serial_controller)
        tab_widget.addTab(self.log_widget, "Communication Log")
        
        main_layout.addWidget(tab_widget)
        
        central_widget.setLayout(main_layout)
        
        # Set application style
        self.setStyleSheet("""
            QGroupBox {
                font-weight: bold;
                border: 2px solid #cccccc;
                border-radius: 5px;
                margin-top: 1ex;
                padding-top: 10px;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 5px 0 5px;
            }
            QPushButton {
                padding: 8px 16px;
                border-radius: 4px;
                border: 1px solid #ccc;
                background-color: #f8f8f8;
            }
            QPushButton:hover {
                background-color: #e8e8e8;
            }
            QPushButton:pressed {
                background-color: #d8d8d8;
            }
        """)


def main():
    app = QApplication(sys.argv)
    app.setApplicationName("Smart Practice Pad")
    app.setApplicationVersion("1.0")
    
    # Set application style
    app.setStyle('Fusion')
    
    window = MainWindow()
    window.show()
    
    sys.exit(app.exec_())


if __name__ == "__main__":
    main()