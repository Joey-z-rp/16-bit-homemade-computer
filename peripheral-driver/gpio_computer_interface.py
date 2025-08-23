#!/usr/bin/env python3
"""
GPIO Computer Interface for 16-bit Home Computer
Orchestrates the keyboard (8-bit ASCII) and UI (16-bit data, 2-bit address)
interfaces, which are implemented in separate modules.
"""

import time
import RPi.GPIO as GPIO

from keyboard_interface import KeyboardInterface
from ui_interface import UIInterface


class GPIOComputerInterface:
    def __init__(self) -> None:
        # Pin configuration
        self.KEYBOARD_DATA_PINS = [27, 17, 15, 4, 2, 18, 14, 3]  # LSB -> MSB
        self.UI_DATA_PINS = [20, 19, 12, 5, 8, 25, 10, 23, 16, 13, 6, 7, 11, 9, 24, 22]  # LSB -> MSB
        self.UI_ADDRESS_PINS = [21, 26]

        # Instantiate submodules
        self.keyboard = KeyboardInterface(self.KEYBOARD_DATA_PINS)
        self.ui = UIInterface(self.UI_DATA_PINS, self.UI_ADDRESS_PINS)

        self.running = False

        print("GPIO Computer Interface initialized")

    def start(self) -> None:
        self.running = True
        self.keyboard.start()
        self.ui.start()

        print("\nGPIO Computer Interface Running!")
        print("Press Ctrl+C to stop...")

        try:
            # If the UI renderer requires foreground rendering (e.g., kmsdrm/fbcon),
            # run its loop on the main thread. For background-capable drivers this is a no-op.
            ui_renderer = getattr(self.ui, "_built_in_processor", None)
            if ui_renderer is not None:
                ui_renderer.run_forever_if_foreground()

            # For background-capable drivers, keep the main thread alive.
            while self.running:
                time.sleep(0.1)
        except KeyboardInterrupt:
            print("\nStopping GPIO Computer Interface...")
            self.stop()

    def stop(self) -> None:
        self.running = False
        self.keyboard.stop()
        self.ui.stop()
        self.keyboard.join(timeout=1)
        self.ui.join(timeout=1)

    def cleanup(self) -> None:
        GPIO.cleanup()
        print("GPIO cleanup completed")


def main():
    print("GPIO Computer Interface for 16-bit Home Computer")
    print("===============================================")

    # Check if running as root (needed for GPIO access)
    import os
    if os.geteuid() != 0:
        print("Warning: This script may need to run as root for GPIO access.")
        print(
            "If you encounter permission errors, try running with: sudo python3 gpio_computer_interface.py"
        )
        print()

    interface = GPIOComputerInterface()

    try:
        interface.start()
    finally:
        interface.cleanup()


if __name__ == "__main__":
    main()