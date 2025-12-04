#!/usr/bin/env python3
"""
Simple Practice Pad Application
Terminal-based interface for Stick Control practice
"""

import sys
import time
import argparse
from serial_comm.stm32_interface import STM32Interface


class SimplePracticeApp:
    """Simple terminal-based practice application"""

    def __init__(self, port: str = '/dev/ttyACM0'):
        self.stm32 = STM32Interface(port=port)
        self.current_rep = 0
        self.current_exercise = None
        self.session_active = False

    def run(self):
        """Main application loop"""
        print("=" * 60)
        print("  STICK CONTROL PRACTICE PAD")
        print("  Based on George Lawrence Stone's Method")
        print("=" * 60)
        print()

        # Connect to STM32
        print("Connecting to practice pad...")
        if not self.stm32.connect():
            print("\n✗ Failed to connect!")
            print("  Check that:")
            print("  1. STM32 is connected via USB")
            print("  2. Port is correct (try --port /dev/ttyACM0 or COM3)")
            print("  3. No other program is using the port")
            return 1

        # Test connection
        if not self.stm32.ping():
            print("✗ Device not responding")
            self.stm32.disconnect()
            return 1

        print("✓ Connected successfully!\n")

        # Register event callbacks
        self.stm32.register_event_callback("STRIKE", self.on_strike)
        self.stm32.register_event_callback("REP", self.on_rep_update)

        # Main menu
        while True:
            choice = self.show_menu()
            if choice == 'q':
                break
            self.handle_choice(choice)

        self.stm32.disconnect()
        print("\nGoodbye!")
        return 0

    def show_menu(self):
        """Show main menu and get user choice"""
        print("\n" + "=" * 60)
        print("MAIN MENU")
        print("=" * 60)
        print("1. Start Practice Session")
        print("2. Test Metronome")
        print("3. Check Connection")
        print("q. Quit")
        print()
        return input("Choice: ").strip().lower()

    def handle_choice(self, choice: str):
        """Handle menu selection"""
        if choice == '1':
            self.practice_session()
        elif choice == '2':
            self.test_metronome()
        elif choice == '3':
            self.check_connection()
        else:
            print("Invalid choice")

    def practice_session(self):
        """Start a practice session"""
        print("\n" + "=" * 60)
        print("PRACTICE SESSION")
        print("=" * 60)

        # Get exercise ID
        print("\nAvailable exercises:")
        print("  Level 1 (Single Beats): 1-72")
        print("  Level 2 (Triplets): 73-120")
        print("  Level 3 (Rolls): 121-220")
        print("  Level 4 (Flams): 221-320")
        print()

        try:
            exercise_id = int(input("Exercise ID (1-320): "))
            if exercise_id < 1 or exercise_id > 320:
                print("Invalid exercise ID")
                return
        except ValueError:
            print("Invalid input")
            return

        # Get tempo
        try:
            tempo = int(input("Tempo (40-200 BPM): "))
            if tempo < 40 or tempo > 200:
                print("Invalid tempo")
                return
        except ValueError:
            print("Invalid input")
            return

        # Load exercise
        print(f"\nLoading Exercise {exercise_id} at {tempo} BPM...")
        ex_info = self.stm32.load_exercise(exercise_id, tempo)

        if not ex_info:
            print("✗ Failed to load exercise")
            return

        self.current_exercise = ex_info
        print(f"✓ Loaded: {ex_info.get('NAME', 'Unknown')}")
        print(f"  Pattern: {ex_info.get('PATTERN', 'Unknown')}")
        print(f"  Tempo: {tempo} BPM")

        # Set BPM
        self.stm32.set_bpm(tempo)

        print("\n" + "-" * 60)
        print("STONE'S METHOD:")
        print("  • Play pattern 20 times WITHOUT stopping")
        print("  • If you fail on rep 15, you start over from 0!")
        print("  • Only perfection counts")
        print("-" * 60)

        input("\nPress ENTER to start...")

        # Start session
        print("\n🎯 STARTING SESSION...")
        print("Play the pattern now!\n")

        if not self.stm32.start_session():
            print("✗ Failed to start session")
            return

        self.session_active = True
        self.current_rep = 0

        # Wait for session to complete or user abort
        try:
            while self.session_active:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\n\n⚠ Session aborted by user")
            self.stm32.abort_session()

    def test_metronome(self):
        """Test metronome functionality"""
        print("\n" + "=" * 60)
        print("METRONOME TEST")
        print("=" * 60)

        try:
            tempo = int(input("Tempo (40-200 BPM): "))
            if tempo < 40 or tempo > 200:
                print("Invalid tempo")
                return
        except ValueError:
            print("Invalid input")
            return

        print(f"\nSetting tempo to {tempo} BPM...")
        if not self.stm32.set_bpm(tempo):
            print("✗ Failed to set tempo")
            return

        print("✓ Tempo set")
        print("\nStarting metronome...")

        if not self.stm32.start_metronome():
            print("✗ Failed to start metronome")
            return

        print("✓ Metronome running")
        print("  (Watch the LED on your STM32 board)")
        print("\nPress ENTER to stop...")
        input()

        self.stm32.stop_metronome()
        print("✓ Metronome stopped")

    def check_connection(self):
        """Check connection status"""
        print("\n" + "=" * 60)
        print("CONNECTION TEST")
        print("=" * 60)

        print("\nTesting connection...")
        if self.stm32.ping():
            print("✓ Connection OK")

            status = self.stm32.get_status()
            if status:
                print(f"\nMetronome Status:")
                print(f"  BPM: {status.get('BPM', 'Unknown')}")
                print(f"  Running: {status.get('RUNNING', 'Unknown')}")

            state = self.stm32.get_session_state()
            if state:
                print(f"\nSession State:")
                print(f"  State: {state.get('STATE', 'Unknown')}")
                print(f"  Rep: {state.get('REP', 'Unknown')}")
                print(f"  Attempts: {state.get('ATTEMPTS', 'Unknown')}")
        else:
            print("✗ Connection failed")

    # ========================================================================
    # Event callbacks
    # ========================================================================

    def on_strike(self, data):
        """Called when a strike is detected"""
        if data:
            try:
                timing_error = int(data[0])
                # Visual feedback based on timing
                if abs(timing_error) < 20:
                    indicator = "🎯"
                elif abs(timing_error) < 40:
                    indicator = "✓"
                elif abs(timing_error) < 60:
                    indicator = "⚠"
                else:
                    indicator = "✗"

                print(f"  {indicator} Strike: {timing_error:+4d} ms", flush=True)
            except:
                pass

    def on_rep_update(self, data):
        """Called when rep counter updates"""
        if data:
            try:
                rep_str = data[0]  # e.g., "5/20"
                print(f"\n📊 Progress: {rep_str}", flush=True)

                # Check if completed
                if "20/20" in rep_str or "20" == rep_str.split('/')[0]:
                    self.session_complete()
                elif rep_str.startswith("1/"):
                    # Reset detected
                    print("⚠ RESET TO 0! Keep trying...\n", flush=True)

            except:
                pass

    def session_complete(self):
        """Handle session completion"""
        self.session_active = False
        print("\n" + "=" * 60)
        print("🎉 SUCCESS! EXERCISE COMPLETED!")
        print("=" * 60)
        print("\nYou've mastered this exercise at this tempo!")
        print("\nNext steps:")
        print("  • Try a faster tempo")
        print("  • Move to the next exercise")
        print("=" * 60)


def main():
    """Entry point"""
    parser = argparse.ArgumentParser(description='Stick Control Practice Pad')
    parser.add_argument('--port', default='/dev/ttyACM0',
                        help='Serial port (default: /dev/ttyACM0)')
    args = parser.parse_args()

    app = SimplePracticeApp(port=args.port)
    return app.run()


if __name__ == "__main__":
    sys.exit(main())
