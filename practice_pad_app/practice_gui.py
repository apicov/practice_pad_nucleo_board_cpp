#!/usr/bin/env python3
"""
Stick Control Practice Pad - Qt GUI Application
Real-time practice interface with visual timing feedback
"""

import sys
import argparse
from collections import deque
import numpy as np
from serial.tools import list_ports

from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
    QPushButton, QLabel, QSpinBox, QComboBox, QGroupBox, QProgressBar,
    QTextEdit, QGridLayout, QStatusBar, QMessageBox
)
from PyQt5.QtCore import QTimer, Qt, pyqtSignal
from PyQt5.QtGui import QFont, QPalette, QColor

import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.figure import Figure

from serial_comm.stm32_interface import STM32Interface


class TimingPlot(FigureCanvas):
    """Real-time timing accuracy plot"""

    def __init__(self, parent=None, width=8, height=3, dpi=100):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        self.axes = self.fig.add_subplot(111)
        super().__init__(self.fig)
        self.setParent(parent)

        # Data storage (last 80 strikes)
        self.timing_data = deque(maxlen=80)
        self.strike_numbers = deque(maxlen=80)
        self.strike_counter = 0

        # Setup plot
        self.setup_plot()

    def setup_plot(self):
        """Initialize plot appearance"""
        self.axes.clear()
        self.axes.set_xlabel('Strike Number', fontsize=10)
        self.axes.set_ylabel('Timing Error (ms)', fontsize=10)
        self.axes.set_title('Strike Timing Accuracy', fontsize=12, fontweight='bold')
        self.axes.grid(True, alpha=0.3)
        self.axes.axhline(y=0, color='green', linestyle='--', linewidth=2, alpha=0.5, label='Perfect')
        self.axes.axhline(y=60, color='orange', linestyle='--', linewidth=1, alpha=0.3)
        self.axes.axhline(y=-60, color='orange', linestyle='--', linewidth=1, alpha=0.3)
        self.axes.set_ylim(-100, 100)
        self.axes.legend(loc='upper right', fontsize=8)
        self.fig.tight_layout()

    def add_strike(self, timing_ms):
        """Add a new strike to the plot"""
        self.strike_counter += 1
        self.timing_data.append(timing_ms)
        self.strike_numbers.append(self.strike_counter)
        self.update_plot()

    def update_plot(self):
        """Redraw the plot with current data"""
        self.axes.clear()
        self.setup_plot()

        if len(self.timing_data) > 0:
            # Color code based on accuracy
            colors = []
            for timing in self.timing_data:
                abs_timing = abs(timing)
                if abs_timing <= 20:
                    colors.append('green')
                elif abs_timing <= 40:
                    colors.append('lightgreen')
                elif abs_timing <= 60:
                    colors.append('orange')
                else:
                    colors.append('red')

            self.axes.scatter(list(self.strike_numbers), list(self.timing_data),
                              c=colors, s=50, alpha=0.7, edgecolors='black', linewidth=0.5)

            # Set x-axis limits dynamically
            if len(self.strike_numbers) > 0:
                x_min = max(0, min(self.strike_numbers) - 5)
                x_max = max(self.strike_numbers) + 5
                self.axes.set_xlim(x_min, x_max)

        self.draw()

    def clear_data(self):
        """Clear all strike data"""
        self.timing_data.clear()
        self.strike_numbers.clear()
        self.strike_counter = 0
        self.setup_plot()
        self.draw()


class PracticePadGUI(QMainWindow):
    """Main application window"""

    # Signals
    strike_received = pyqtSignal(int)  # timing_ms
    rep_updated = pyqtSignal(str)      # rep_str (e.g., "5/20")

    def __init__(self, port='/dev/ttyACM0'):
        super().__init__()

        # STM32 interface (will be initialized when connecting)
        self.stm32 = None
        self.default_port = port
        self.connected = False

        # Session state
        self.session_active = False
        self.current_exercise = None
        self.current_rep = 0

        # Setup UI
        self.init_ui()

        # Connect signals
        self.strike_received.connect(self.on_strike_signal)
        self.rep_updated.connect(self.on_rep_signal)

        # Status update timer
        self.status_timer = QTimer()
        self.status_timer.timeout.connect(self.update_status)
        self.status_timer.start(1000)  # Update every second

    def init_ui(self):
        """Initialize the user interface"""
        self.setWindowTitle('Stick Control Practice Pad')
        self.setGeometry(100, 100, 1200, 800)

        # Central widget and main layout
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QHBoxLayout(central_widget)

        # Left panel (controls)
        left_panel = self.create_left_panel()
        main_layout.addWidget(left_panel, stretch=1)

        # Right panel (visualization)
        right_panel = self.create_right_panel()
        main_layout.addWidget(right_panel, stretch=2)

        # Status bar
        self.statusBar = QStatusBar()
        self.setStatusBar(self.statusBar)
        self.statusBar.showMessage('Not connected')

    def create_left_panel(self):
        """Create left control panel"""
        panel = QWidget()
        layout = QVBoxLayout(panel)

        # Connection group
        conn_group = QGroupBox("Connection")
        conn_layout = QVBoxLayout()

        # Port selection
        port_layout = QHBoxLayout()
        port_layout.addWidget(QLabel("Port:"))
        self.port_combo = QComboBox()
        self.port_combo.setMinimumWidth(150)
        port_layout.addWidget(self.port_combo)

        self.refresh_ports_btn = QPushButton('⟳')
        self.refresh_ports_btn.setMaximumWidth(30)
        self.refresh_ports_btn.setToolTip('Refresh port list')
        self.refresh_ports_btn.clicked.connect(self.refresh_ports)
        port_layout.addWidget(self.refresh_ports_btn)

        conn_layout.addLayout(port_layout)

        self.connect_btn = QPushButton('Connect')
        self.connect_btn.clicked.connect(self.toggle_connection)
        self.connect_btn.setStyleSheet("QPushButton { font-weight: bold; padding: 10px; }")
        conn_layout.addWidget(self.connect_btn)

        # Populate ports on startup (after connect_btn is created)
        self.refresh_ports()

        self.connection_status = QLabel('⭕ Disconnected')
        self.connection_status.setAlignment(Qt.AlignCenter)
        font = QFont()
        font.setPointSize(12)
        font.setBold(True)
        self.connection_status.setFont(font)
        conn_layout.addWidget(self.connection_status)

        conn_group.setLayout(conn_layout)
        layout.addWidget(conn_group)

        # Exercise selection group
        exercise_group = QGroupBox("Exercise Selection")
        exercise_layout = QGridLayout()

        exercise_layout.addWidget(QLabel("Exercise ID:"), 0, 0)
        self.exercise_spin = QSpinBox()
        self.exercise_spin.setRange(1, 320)
        self.exercise_spin.setValue(1)
        self.exercise_spin.valueChanged.connect(self.on_exercise_changed)
        exercise_layout.addWidget(self.exercise_spin, 0, 1)

        exercise_layout.addWidget(QLabel("Tempo (BPM):"), 1, 0)
        self.tempo_spin = QSpinBox()
        self.tempo_spin.setRange(40, 200)
        self.tempo_spin.setValue(60)
        self.tempo_spin.setSingleStep(10)
        exercise_layout.addWidget(self.tempo_spin, 1, 1)

        self.load_btn = QPushButton('Load Exercise')
        self.load_btn.clicked.connect(self.load_exercise)
        self.load_btn.setEnabled(False)
        exercise_layout.addWidget(self.load_btn, 2, 0, 1, 2)

        exercise_group.setLayout(exercise_layout)
        layout.addWidget(exercise_group)

        # Exercise info group
        info_group = QGroupBox("Current Exercise")
        info_layout = QVBoxLayout()

        self.exercise_name_label = QLabel("No exercise loaded")
        self.exercise_name_label.setWordWrap(True)
        self.exercise_name_label.setStyleSheet("font-size: 14px; font-weight: bold;")
        info_layout.addWidget(self.exercise_name_label)

        self.pattern_label = QLabel("Pattern: -")
        self.pattern_label.setWordWrap(True)
        self.pattern_label.setStyleSheet("font-family: monospace; font-size: 16px;")
        info_layout.addWidget(self.pattern_label)

        self.tempo_label = QLabel("Tempo: - BPM")
        self.tempo_label.setStyleSheet("font-size: 12px;")
        info_layout.addWidget(self.tempo_label)

        info_group.setLayout(info_layout)
        layout.addWidget(info_group)

        # Session control group
        session_group = QGroupBox("Session Control")
        session_layout = QVBoxLayout()

        self.start_btn = QPushButton('▶ Start Session')
        self.start_btn.clicked.connect(self.start_session)
        self.start_btn.setEnabled(False)
        self.start_btn.setStyleSheet("QPushButton { font-weight: bold; padding: 15px; font-size: 14px; }")
        session_layout.addWidget(self.start_btn)

        self.pause_btn = QPushButton('⏸ Pause')
        self.pause_btn.clicked.connect(self.pause_session)
        self.pause_btn.setEnabled(False)
        session_layout.addWidget(self.pause_btn)

        self.reset_btn = QPushButton('🔄 Reset to 0/20')
        self.reset_btn.clicked.connect(self.reset_session)
        self.reset_btn.setEnabled(False)
        session_layout.addWidget(self.reset_btn)

        self.abort_btn = QPushButton('⏹ Stop Session')
        self.abort_btn.clicked.connect(self.abort_session)
        self.abort_btn.setEnabled(False)
        session_layout.addWidget(self.abort_btn)

        session_group.setLayout(session_layout)
        layout.addWidget(session_group)

        # Progress group
        progress_group = QGroupBox("Progress")
        progress_layout = QVBoxLayout()

        self.rep_label = QLabel("Rep: 0/20")
        self.rep_label.setStyleSheet("font-size: 24px; font-weight: bold;")
        self.rep_label.setAlignment(Qt.AlignCenter)
        progress_layout.addWidget(self.rep_label)

        self.progress_bar = QProgressBar()
        self.progress_bar.setRange(0, 20)
        self.progress_bar.setValue(0)
        self.progress_bar.setTextVisible(True)
        self.progress_bar.setFormat("%v/20")
        self.progress_bar.setStyleSheet("""
            QProgressBar {
                border: 2px solid grey;
                border-radius: 5px;
                text-align: center;
                font-size: 14px;
                font-weight: bold;
            }
            QProgressBar::chunk {
                background-color: #4CAF50;
            }
        """)
        progress_layout.addWidget(self.progress_bar)

        self.attempt_label = QLabel("Attempts: 0")
        self.attempt_label.setAlignment(Qt.AlignCenter)
        progress_layout.addWidget(self.attempt_label)

        progress_group.setLayout(progress_layout)
        layout.addWidget(progress_group)

        # Stretch to push everything to top
        layout.addStretch()

        return panel

    def create_right_panel(self):
        """Create right visualization panel"""
        panel = QWidget()
        layout = QVBoxLayout(panel)

        # Title
        title = QLabel("Real-Time Strike Analysis")
        title.setStyleSheet("font-size: 18px; font-weight: bold;")
        title.setAlignment(Qt.AlignCenter)
        layout.addWidget(title)

        # Timing plot
        self.timing_plot = TimingPlot(self, width=8, height=4)
        layout.addWidget(self.timing_plot)

        # Legend
        legend_group = QGroupBox("Timing Guide")
        legend_layout = QHBoxLayout()
        legend_layout.addWidget(QLabel("🎯 Perfect (±20ms)"))
        legend_layout.addWidget(QLabel("✓ Good (±40ms)"))
        legend_layout.addWidget(QLabel("⚠ OK (±60ms)"))
        legend_layout.addWidget(QLabel("✗ Poor (>60ms)"))
        legend_group.setLayout(legend_layout)
        layout.addWidget(legend_group)

        # Event log
        log_label = QLabel("Event Log")
        log_label.setStyleSheet("font-weight: bold;")
        layout.addWidget(log_label)

        self.event_log = QTextEdit()
        self.event_log.setReadOnly(True)
        self.event_log.setMaximumHeight(200)
        self.event_log.setStyleSheet("font-family: monospace; font-size: 10px;")
        layout.addWidget(self.event_log)

        return panel

    def refresh_ports(self):
        """Refresh the list of available serial ports"""
        self.port_combo.clear()

        # Get available ports
        ports = list_ports.comports()
        port_list = [port.device for port in ports]

        if not port_list:
            self.port_combo.addItem("No ports found")
            self.port_combo.setEnabled(False)
            self.connect_btn.setEnabled(False)
        else:
            self.port_combo.setEnabled(True)
            self.connect_btn.setEnabled(True)

            # Add all available ports
            for port in port_list:
                self.port_combo.addItem(port)

            # Try to select the default port if it exists
            if self.default_port in port_list:
                index = port_list.index(self.default_port)
                self.port_combo.setCurrentIndex(index)

            self.log_event(f"Found {len(port_list)} port(s): {', '.join(port_list)}")

    def toggle_connection(self):
        """Connect or disconnect from STM32"""
        if not self.connected:
            selected_port = self.port_combo.currentText()

            if selected_port == "No ports found":
                QMessageBox.warning(self, "No Ports", "No serial ports available.\nCheck USB connection and click refresh.")
                return

            self.log_event(f"Connecting to {selected_port}...")

            # Create STM32 interface with selected port
            self.stm32 = STM32Interface(port=selected_port)

            if self.stm32.connect():
                if self.stm32.ping():
                    self.connected = True
                    self.connect_btn.setText('Disconnect')
                    self.connection_status.setText('🟢 Connected')
                    self.connection_status.setStyleSheet("color: green;")
                    self.load_btn.setEnabled(True)
                    self.port_combo.setEnabled(False)
                    self.refresh_ports_btn.setEnabled(False)
                    self.statusBar.showMessage(f'Connected to {selected_port}')
                    self.log_event("✓ Connected successfully!")

                    # Register callbacks
                    self.stm32.register_event_callback("STRIKE", self.on_strike)
                    self.stm32.register_event_callback("REP", self.on_rep)
                else:
                    self.stm32.disconnect()
                    self.stm32 = None
                    self.log_event("✗ Device not responding")
                    QMessageBox.warning(self, "Connection Failed", "Device not responding")
            else:
                self.stm32 = None
                self.log_event("✗ Connection failed")
                QMessageBox.warning(self, "Connection Failed",
                                    f"Could not connect to {selected_port}.\nCheck USB connection and permissions.")
        else:
            self.stm32.disconnect()
            self.stm32 = None
            self.connected = False
            self.connect_btn.setText('Connect')
            self.connection_status.setText('⭕ Disconnected')
            self.connection_status.setStyleSheet("color: gray;")
            self.load_btn.setEnabled(False)
            self.start_btn.setEnabled(False)
            self.port_combo.setEnabled(True)
            self.refresh_ports_btn.setEnabled(True)
            self.statusBar.showMessage('Disconnected')
            self.log_event("Disconnected")

    def on_exercise_changed(self, value):
        """Update exercise info label when spinner changes"""
        level = "Unknown"
        if 1 <= value <= 72:
            level = "Level 1: Single Beats"
        elif 73 <= value <= 120:
            level = "Level 2: Triplets"
        elif 121 <= value <= 220:
            level = "Level 3: Rolls"
        elif 221 <= value <= 320:
            level = "Level 4: Flams"

        self.statusBar.showMessage(f"Exercise {value} - {level}")

    def load_exercise(self):
        """Load selected exercise"""
        if not self.stm32:
            return

        exercise_id = self.exercise_spin.value()
        tempo = self.tempo_spin.value()

        self.log_event(f"Loading Exercise {exercise_id} at {tempo} BPM...")

        ex_info = self.stm32.load_exercise(exercise_id, tempo)

        if ex_info:
            self.current_exercise = ex_info
            name = ex_info.get('NAME', 'Unknown').replace('_', ' ')
            pattern = ex_info.get('PATTERN', 'Unknown')

            self.exercise_name_label.setText(f"Exercise {exercise_id}: {name}")
            self.pattern_label.setText(f"Pattern: {pattern}")
            self.tempo_label.setText(f"Tempo: {tempo} BPM")

            self.start_btn.setEnabled(True)
            self.stm32.set_bpm(tempo)

            self.log_event(f"✓ Loaded: {name}")
            self.log_event(f"  Pattern: {pattern}")
            self.log_event(f"  Tempo: {tempo} BPM")

            # Clear plot
            self.timing_plot.clear_data()
            self.current_rep = 0
            self.progress_bar.setValue(0)
            self.rep_label.setText("Rep: 0/20")
        else:
            self.log_event("✗ Failed to load exercise")
            QMessageBox.warning(self, "Load Failed", "Could not load exercise")

    def start_session(self):
        """Start practice session"""
        if not self.stm32:
            return

        if self.stm32.start_session():
            self.session_active = True
            self.start_btn.setEnabled(False)
            self.pause_btn.setEnabled(True)
            self.reset_btn.setEnabled(True)
            self.abort_btn.setEnabled(True)
            self.load_btn.setEnabled(False)

            self.log_event("=" * 40)
            self.log_event("🎯 SESSION STARTED - GO!")
            self.log_event("Play the pattern 20 times perfectly!")
            self.log_event("=" * 40)

            self.statusBar.showMessage('Session active - Play now!')
        else:
            self.log_event("✗ Failed to start session")

    def pause_session(self):
        """Pause session"""
        if not self.stm32:
            return

        if self.stm32.pause_session():
            self.pause_btn.setText('▶ Resume')
            self.pause_btn.clicked.disconnect()
            self.pause_btn.clicked.connect(self.resume_session)
            self.log_event("⏸ Session paused")
            self.statusBar.showMessage('Session paused')

    def resume_session(self):
        """Resume session"""
        if not self.stm32:
            return

        if self.stm32.resume_session():
            self.pause_btn.setText('⏸ Pause')
            self.pause_btn.clicked.disconnect()
            self.pause_btn.clicked.connect(self.pause_session)
            self.log_event("▶ Session resumed")
            self.statusBar.showMessage('Session active')

    def reset_session(self):
        """Reset session to 0/20"""
        if not self.stm32:
            return

        reply = QMessageBox.question(self, 'Reset Session',
                                      'Reset to 0/20?',
                                      QMessageBox.Yes | QMessageBox.No)

        if reply == QMessageBox.Yes:
            if self.stm32.reset_session():
                self.current_rep = 0
                self.progress_bar.setValue(0)
                self.rep_label.setText("Rep: 0/20")
                self.timing_plot.clear_data()
                self.log_event("🔄 Session reset to 0/20")

    def abort_session(self):
        """Stop session"""
        if not self.stm32:
            return

        reply = QMessageBox.question(self, 'Stop Session',
                                      'Stop the current session?',
                                      QMessageBox.Yes | QMessageBox.No)

        if reply == QMessageBox.Yes:
            if self.stm32.abort_session():
                self.session_active = False
                self.start_btn.setEnabled(True)
                self.pause_btn.setEnabled(False)
                self.reset_btn.setEnabled(False)
                self.abort_btn.setEnabled(False)
                self.load_btn.setEnabled(True)

                self.log_event("⏹ Session aborted")
                self.statusBar.showMessage('Session stopped')

    def on_strike(self, data):
        """Callback for strike event from STM32"""
        if data:
            try:
                timing_ms = int(data[0])
                self.strike_received.emit(timing_ms)
            except:
                pass

    def on_strike_signal(self, timing_ms):
        """Handle strike signal in GUI thread"""
        # Add to plot
        self.timing_plot.add_strike(timing_ms)

        # Log with indicator
        abs_timing = abs(timing_ms)
        if abs_timing <= 20:
            indicator = "🎯"
        elif abs_timing <= 40:
            indicator = "✓"
        elif abs_timing <= 60:
            indicator = "⚠"
        else:
            indicator = "✗"

        self.log_event(f"  {indicator} Strike: {timing_ms:+4d} ms")

    def on_rep(self, data):
        """Callback for rep update from STM32"""
        if data:
            try:
                rep_str = data[0]
                self.rep_updated.emit(rep_str)
            except:
                pass

    def on_rep_signal(self, rep_str):
        """Handle rep update signal in GUI thread"""
        try:
            current, total = rep_str.split('/')
            current = int(current)

            self.current_rep = current
            self.progress_bar.setValue(current)
            self.rep_label.setText(f"Rep: {current}/{total}")

            self.log_event(f"\n📊 Progress: {rep_str}")

            # Check for completion
            if current >= 20:
                self.on_session_complete()
            elif current == 1 and self.current_rep > 1:
                # Reset detected
                self.log_event("⚠ RESET TO 0/20! Keep trying...")
                self.timing_plot.clear_data()

        except:
            pass

    def on_session_complete(self):
        """Handle session completion"""
        self.session_active = False
        self.start_btn.setEnabled(True)
        self.pause_btn.setEnabled(False)
        self.reset_btn.setEnabled(False)
        self.abort_btn.setEnabled(False)
        self.load_btn.setEnabled(True)

        self.log_event("")
        self.log_event("=" * 40)
        self.log_event("🎉 SUCCESS! EXERCISE COMPLETED!")
        self.log_event("=" * 40)

        QMessageBox.information(self, "Success!",
                                "🎉 Exercise completed!\n\n" +
                                "You've mastered this exercise at this tempo!\n\n" +
                                "Next steps:\n" +
                                "• Try a faster tempo\n" +
                                "• Move to the next exercise")

    def update_status(self):
        """Periodic status update"""
        if self.connected and self.session_active and self.stm32:
            state = self.stm32.get_session_state()
            if state:
                attempts = state.get('ATTEMPTS', '0')
                self.attempt_label.setText(f"Attempts: {attempts}")

    def log_event(self, message):
        """Add message to event log"""
        if hasattr(self, 'event_log'):
            self.event_log.append(message)
            # Auto-scroll to bottom
            scrollbar = self.event_log.verticalScrollBar()
            scrollbar.setValue(scrollbar.maximum())

    def closeEvent(self, event):
        """Handle window close"""
        if self.connected and self.stm32:
            self.stm32.disconnect()
        event.accept()


def main():
    """Entry point"""
    parser = argparse.ArgumentParser(description='Stick Control Practice Pad GUI')
    parser.add_argument('--port', default='/dev/ttyACM0',
                        help='Serial port (default: /dev/ttyACM0)')
    args = parser.parse_args()

    app = QApplication(sys.argv)

    # Set application style
    app.setStyle('Fusion')

    gui = PracticePadGUI(port=args.port)
    gui.show()

    sys.exit(app.exec_())


if __name__ == "__main__":
    main()
