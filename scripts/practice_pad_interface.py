#!/usr/bin/env python3
"""
Smart Practice Pad Interface
A PyQt5 application for controlling the STM32-based drum practice pad
"""

import sys
import time
import threading
from PyQt5.QtWidgets import (QApplication, QMainWindow, QVBoxLayout, QHBoxLayout, 
                             QWidget, QPushButton, QLabel, QSpinBox, QComboBox,
                             QTextEdit, QGroupBox, QGridLayout, QProgressBar, QFrame)
from PyQt5.QtCore import QTimer, pyqtSignal, QObject, pyqtSlot
from PyQt5.QtGui import QFont, QPalette, QColor
import serial
import serial.tools.list_ports


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
            # Send command
            self.serial_port.write(f"{command}\n".encode())
            self.serial_port.flush()
            
            # Read response with timeout and error handling
            response = ""
            start_time = time.time()
            while time.time() - start_time < 2.0:  # 2 second timeout
                if self.serial_port.in_waiting > 0:
                    try:
                        raw_response = self.serial_port.readline()
                        line = raw_response.decode('utf-8', errors='replace').strip()
                        # Filter out non-printable characters except newlines
                        line = ''.join(char for char in line if char.isprintable() or char in '\r\n').strip()
                        
                        # Only accept lines that start with CMD_ (command responses)
                        if line.startswith('CMD_'):
                            response = line
                            break
                        # Ignore other debug output and continue reading
                        
                    except UnicodeDecodeError as e:
                        self.status_updated.emit(f"Decode error: {str(e)}")
                        continue
                time.sleep(0.01)
            
            if response:
                self.response_received.emit(f"CMD: {command} -> {response}")
                return response
            else:
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
        main_layout.addWidget(self.metronome_widget)
        
        # Log widget
        self.log_widget = LogWidget(self.serial_controller)
        main_layout.addWidget(self.log_widget)
        
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