import sys
import serial
import numpy as np
from PyQt5.QtWidgets import QApplication, QMainWindow, QVBoxLayout, QWidget, QPushButton
from PyQt5.QtCore import QTimer
import matplotlib.pyplot as plt
from matplotlib.backends.backend_qt5agg import FigureCanvasQTAgg as FigureCanvas


from datetime import datetime

def get_current_datetime():
    # Get the current date and time
    now = datetime.now()
    # Format the date and time as a string
    formatted_datetime = now.strftime("%Y%m%d_%H%M%S")
    return formatted_datetime

def extract_line(buffer):
    # Find the position of the first newline character
    newline_pos = buffer.find('\n')
    
    if newline_pos == -1:
        # If no newline is found, return empty string
        line = ''
    else:
        # Extract the line up to the newline character
        line = buffer[:newline_pos]
        # Update the buffer to remove the extracted line and the newline character
        buffer = buffer[newline_pos + 1:]
    
    return line, buffer

class SerialPlotter(QMainWindow):
    def __init__(self, n_channels=5):
        super().__init__()

        self.n_channels = n_channels  # Number of channels to read from the serial port

        # Serial port configuration
        self.serial_port = '/dev/ttyACM0'  # Change this to your serial port
        self.baud_rate = 230400             # Change this to your baud rate
        self.timeout = 0                # Timeout in seconds
        self.ser = serial.Serial(self.serial_port, self.baud_rate, timeout=self.timeout)

        # Data storage
        self.x_data = []
        self.y_data = [[] for _ in range(self.n_channels)]

        # Set up the main window
        self.setWindowTitle("Real-time Serial Data Plotter")
        self.setGeometry(100, 100, 800, 600)

        # Create a central widget and layout
        self.central_widget = QWidget(self)
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout(self.central_widget)

        # Create a Matplotlib figure and canvas
        self.figure, self.ax = plt.subplots()
        self.canvas = FigureCanvas(self.figure)
        self.layout.addWidget(self.canvas)

        # Create a button to start/stop plotting
        self.start_button = QPushButton("Start Plotting", self)
        self.start_button.clicked.connect(self.toggle_plotting)
        self.layout.addWidget(self.start_button)

        # Timer for updating the plot
        self.timer = QTimer()
        self.timer.timeout.connect(self.update_plot)
        self.is_plotting = False
        
        self.counter = 0
        self.buffer = ''
        self.ser.flushInput()  # Clear the input buffer

    def toggle_plotting(self):
        if self.is_plotting:
            self.timer.stop()
            self.start_button.setText("Start Plotting")
            # Close the file if it was opened
            if hasattr(self, 'file'):
                self.file.close()
                del self.file
            # Clear the data lists 
            self.x_data.clear()
            for i in range(self.n_channels):
                self.y_data[i].clear()
        else:
            #open file to save data
            filename = f"../../piezo_data_{get_current_datetime()}.csv"
            try:
                #self.file = open(filename, 'w')
                #header = "timestamp,top,bottom,left,right,force\n"
                #self.file.write(header)
                #self.file.flush()
                a=0
                
            except Exception as e:
                print(f"Error opening file: {e}")
                return
            # Write header to file
            #header = "Timestamp," + ",".join([f"Channel {i+1}" for i in range(self.n_channels)]) + "\n"
            
            # Start the timer to update the plot
            self.timer.start(100)  # Update every 100 ms
            self.start_button.setText("Stop Plotting")
        self.is_plotting = not self.is_plotting

    def update_plot(self):
        try:
            # Read a line from the serial port
            self.buffer += self.ser.read(self.ser.inWaiting() or 1).decode('utf-8')#.strip()
            line_data, self.buffer = extract_line(self.buffer)
            
            #read all available lines
            while line_data:
                print(f"Received data: {line_data}")
                line_data = line_data.split(':')
                if len(line_data) < 2:
                    print("Invalid data format")
                    continue

                # Extract timestamp and channel values
                try:
                    timestamp = int(line_data[0])
                except ValueError:
                    print(f"Invalid value: {line_data[0]}")
                    continue
                # Append the timestamp to x_data
                self.x_data.append(timestamp)
                
                channel_values = line_data[1].split(' ')
                if len(channel_values) != self.n_channels:
                    print("Invalid number of channel values")
                    continue

                # Convert channel values to int and append to data lists
                for i, value in enumerate(channel_values):
                    try:
                        y_value = int(value)
                        self.y_data[i].append(y_value)
                    except ValueError:
                        print(f"Invalid value: {value}")
                        continue

                # Write the data to the file
                if hasattr(self, 'file'):
                    csv_line = f"{timestamp}," + ",".join([str(y) for y in channel_values]) + "\n"
                    self.file.write(csv_line)
                    self.file.flush()

                # Limit the x_data and y_data to the last 100 points
                if len(self.x_data) > 100:
                    self.x_data.pop(0)
                    for i in range(self.n_channels):
                        if len(self.y_data[i]) > 100:
                            self.y_data[i].pop(0)

                line_data, self.buffer = extract_line(self.buffer)

            # Clear the axis and plot the new data
            self.ax.clear()
            # Plot each channel's data
            for i in range(self.n_channels):
                if len(self.y_data[i]) > 0:
                    self.ax.plot(self.x_data, self.y_data[i], lw=2, label=f'Channel {i+1}')
            # Add legend
            #self.ax.legend(loc='upper right', bbox_to_anchor=(1, 1))    
            # Put a legend below current axis
            self.ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05),
                fancybox=True, shadow=True, ncol=5) 
            
            # Set x-axis limits to always show the last 100 points
            #self.ax.set_xlim(max(0, self.counter - 100), self.counter)
            self.ax.set_xlim(self.x_data[0], self.x_data[-1]) 
            self.ax.set_ylim(-50, 130.0) 
            self.ax.set_xlabel('Time')
            self.ax.set_ylabel('Value')
            self.ax.set_title('Real-time Serial Data Plot') 

            # Draw the updated plot
            self.canvas.draw()
        except ValueError as e:
            print(f"ValueError: {e}")  # Debugging statement for conversion errors
        except Exception as e:
            print(f"Error: {e}")  # General error handling

    def closeEvent(self, event):
        self.ser.close()  # Close the serial port when the window is closed
        event.accept()

if __name__ == "__main__":
    app = QApplication(sys.argv)
    plotter = SerialPlotter()
    plotter.show()
    sys.exit(app.exec_())


