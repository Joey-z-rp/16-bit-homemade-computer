#!/usr/bin/env python3
"""
UI interface: polls 16-bit data across four 2-bit-addressed pages from GPIO,
buffers commands, and invokes a processor for each complete command.
"""

import threading
import time
from collections import deque
from typing import Callable, Deque, List, Optional

try:
    from .ui_command_processor import UICommandProcessor  # type: ignore
except Exception:
    # Fallback for running as a script or if relative import fails
    try:
        from ui_command_processor import UICommandProcessor  # type: ignore
    except Exception:
        UICommandProcessor = None  # type: ignore

import RPi.GPIO as GPIO


class UIInterface:
    def __init__(
        self,
        ui_data_pins: List[int],
        ui_address_pins: List[int],
        on_command: Optional[Callable[[List[int]], None]] = None,
    ) -> None:
        self.ui_data_pins: List[int] = ui_data_pins
        self.ui_address_pins: List[int] = ui_address_pins
        self.on_command = on_command
        self._built_in_processor = None

        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)

        for pin in self.ui_data_pins:
            GPIO.setup(pin, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

        for pin in self.ui_address_pins:
            GPIO.setup(pin, GPIO.OUT)
            GPIO.output(pin, GPIO.LOW)

        self.running: bool = False
        self._monitor_thread: Optional[threading.Thread] = None

        self.ui_commands: Deque[List[int]] = deque(maxlen=100)
        self.ui_lock = threading.Lock()

    def _read_ui_data(self) -> int:
        value = 0
        for bit_index, pin in enumerate(self.ui_data_pins):
            if GPIO.input(pin) == GPIO.HIGH:
                value |= 1 << bit_index
        return value

    def _set_ui_address(self, address: int) -> None:
        for bit_index, pin in enumerate(self.ui_address_pins):
            bit_value = (address >> bit_index) & 1
            GPIO.output(pin, GPIO.HIGH if bit_value else GPIO.LOW)

    def _default_process(self, command_buffer: List[int]) -> None:
        print("Default process", command_buffer)

    def _is_command_complete(self, command_buffer: List[int]) -> bool:
        """Check if a command is complete based on MSB pairing and command type."""
        if not command_buffer or command_buffer[0] == -1:
            return False
        
        # Extract command type from first word (now 4 bits in bits 14..11)
        command_type = (command_buffer[0] >> 11) & 0b1111
        
        if command_type == 0b0000:  # Print character command
            # Need at least 2 words with matching MSBs
            if command_buffer[1] == -1:
                return False
            # Check if MSBs match (bit 15 of both words should be the same)
            msb_0 = (command_buffer[0] >> 15) & 1
            msb_1 = (command_buffer[1] >> 15) & 1
            return msb_0 == msb_1
        elif command_type == 0b0001:  # Play sound command
            # Only need 1 word: sound ID
            return command_buffer[0] != -1
        elif command_type == 0b0010:  # Clear screen command
            # Only need 1 word: no parameters
            return command_buffer[0] != -1
        elif command_type == 0b0011:  # Boot command
            # Only need 1 word: no parameters
            return command_buffer[0] != -1
        elif command_type == 0b0100:  # Move sprite command
            # Need all 4 words with matching MSBs
            if (command_buffer[1] == -1 or command_buffer[2] == -1 or command_buffer[3] == -1):
                return False
            # Check if all MSBs match (bit 15 of all words should be the same)
            msb_0 = (command_buffer[0] >> 15) & 1
            msb_1 = (command_buffer[1] >> 15) & 1
            msb_2 = (command_buffer[2] >> 15) & 1
            msb_3 = (command_buffer[3] >> 15) & 1
            return msb_0 == msb_1 == msb_2 == msb_3
        elif command_type == 0b0101:  # Debug command
            if (command_buffer[1] == -1 or command_buffer[2] == -1 or command_buffer[3] == -1):
                return False
            return True
        else:
            return False

    def _monitor_loop(self) -> None:
        print("UI monitoring started...")
        current_address = 0
        last_data = [-1, -1, -1, -1]
        command_buffer = [-1, -1, -1, -1]
        
        # Command reconstruction state
        command_timeout = 0.05  # Time to wait for missing words (seconds) - reduced for responsiveness
        last_command_time = time.time()

        while self.running:
            try:
                self._set_ui_address(current_address)
                time.sleep(0.000001)
                data = self._read_ui_data()
                
                if data != last_data[current_address]:
                    command_buffer[current_address] = data
                    last_data[current_address] = data
                    last_command_time = time.time()
                    

                current_address = (current_address + 1) % 4

                # Check for complete commands in the current buffer
                if self._is_command_complete(command_buffer):
                    # Only process if this is a new command (different from last processed)
                    command_key = tuple(command_buffer)
                    if not hasattr(self, '_last_processed_command') or self._last_processed_command != command_key:
                        self._last_processed_command = command_key
                        
                        # Process the complete command immediately
                        with self.ui_lock:
                            self.ui_commands.append(command_buffer.copy())
                        
                        # Initialize built-in processor lazily if no external handler is provided
                        if self.on_command is None and self._built_in_processor is None and UICommandProcessor is not None:
                            self._built_in_processor = UICommandProcessor()

                        if any(value != -1 for value in command_buffer):
                            print(command_buffer)
                        
                        if self.on_command is not None:
                            self.on_command(command_buffer)
                        elif self._built_in_processor is not None:
                            self._built_in_processor.enqueue_command(command_buffer)
                        else:
                            self._default_process(command_buffer)
                    
                    # Reset buffer after processing (or after determining it's a duplicate)
                    command_buffer = [-1, -1, -1, -1]
                    # Don't reset last_data here - it should persist to avoid processing stale data
                
                # Check for timeout on incomplete commands
                current_time = time.time()
                if current_time - last_command_time > command_timeout and any(value != -1 for value in command_buffer):
                    # Ignore incomplete commands on timeout - reset everything to clear stale state
                    command_buffer = [-1, -1, -1, -1]
                    last_data = [-1, -1, -1, -1]  # Reset last_data only on timeout to clear stale state

                time.sleep(0.0001)
            except Exception as error:
                print(f"Error reading UI data: {error}")
                time.sleep(0.1)

    def start(self) -> None:
        if self.running:
            return
        self.running = True
        # Ensure the built-in UI renderer is initialized so the screen appears
        # even before any GPIO UI commands are received.
        try:
            if self.on_command is None and self._built_in_processor is None and UICommandProcessor is not None:
                self._built_in_processor = UICommandProcessor()
        except Exception as _error:
            print(f"Failed to initialize built-in UI renderer: {_error}")
        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._monitor_thread.start()

    def stop(self) -> None:
        self.running = False
        # Stop built-in processor if we created one
        try:
            if self._built_in_processor is not None:
                self._built_in_processor.stop()
        except Exception:
            pass

    def join(self, timeout: Optional[float] = 1.0) -> None:
        if self._monitor_thread is not None:
            self._monitor_thread.join(timeout=timeout)

