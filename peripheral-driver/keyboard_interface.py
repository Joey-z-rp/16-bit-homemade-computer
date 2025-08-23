#!/usr/bin/env python3
"""
Keyboard interface: reads USB keyboard via evdev and continuously outputs the
current 8-bit ASCII code to the configured GPIO data pins.
"""

import threading
import time
from typing import List, Optional

import RPi.GPIO as GPIO
import evdev


class KeyboardInterface:
    def __init__(self, keyboard_data_pins: List[int]):
        self.keyboard_data_pins: List[int] = keyboard_data_pins

        # Ensure GPIO is in BCM mode and warnings are disabled (idempotent)
        GPIO.setmode(GPIO.BCM)
        GPIO.setwarnings(False)

        for pin in self.keyboard_data_pins:
            GPIO.setup(pin, GPIO.OUT)
            GPIO.output(pin, GPIO.LOW)

        self.keyboard_device: Optional[evdev.InputDevice] = self._find_keyboard_device()
        if not self.keyboard_device:
            print("No USB keyboard found!")
            raise SystemExit(1)

        self.running: bool = False
        self.current_ascii_code: int = 0
        self.key_pressed: bool = False
        self.key_lock = threading.Lock()

        self._monitor_thread: Optional[threading.Thread] = None
        self._output_thread: Optional[threading.Thread] = None

        print(f"Found keyboard: {self.keyboard_device.name}")

    def _find_keyboard_device(self) -> Optional[evdev.InputDevice]:
        for device_path in evdev.list_devices():
            try:
                device = evdev.InputDevice(device_path)
                device_name = device.name.lower()
                if (
                    evdev.ecodes.EV_KEY in device.capabilities()
                    and "virtual" not in device_name
                    and "ssh" not in device_name
                    and "mouse" not in device_name
                ):
                    capabilities = device.capabilities()
                    if evdev.ecodes.EV_KEY in capabilities:
                        key_codes = capabilities[evdev.ecodes.EV_KEY]
                        keyboard_keys = [evdev.ecodes.KEY_A, evdev.ecodes.KEY_SPACE]
                        if any(key in key_codes for key in keyboard_keys):
                            return device
            except (OSError, PermissionError):
                continue
        return None

    def _get_key_ascii(self, event) -> Optional[int]:
        if event.type == evdev.ecodes.EV_KEY:
            key_code = event.code
            key_state = event.value
            if key_state == 1:
                ascii_map = {
                    evdev.ecodes.KEY_A: ord("A"),
                    evdev.ecodes.KEY_B: ord("B"),
                    evdev.ecodes.KEY_C: ord("C"),
                    evdev.ecodes.KEY_D: ord("D"),
                    evdev.ecodes.KEY_E: ord("E"),
                    evdev.ecodes.KEY_F: ord("F"),
                    evdev.ecodes.KEY_G: ord("G"),
                    evdev.ecodes.KEY_H: ord("H"),
                    evdev.ecodes.KEY_I: ord("I"),
                    evdev.ecodes.KEY_J: ord("J"),
                    evdev.ecodes.KEY_K: ord("K"),
                    evdev.ecodes.KEY_L: ord("L"),
                    evdev.ecodes.KEY_M: ord("M"),
                    evdev.ecodes.KEY_N: ord("N"),
                    evdev.ecodes.KEY_O: ord("O"),
                    evdev.ecodes.KEY_P: ord("P"),
                    evdev.ecodes.KEY_Q: ord("Q"),
                    evdev.ecodes.KEY_R: ord("R"),
                    evdev.ecodes.KEY_S: ord("S"),
                    evdev.ecodes.KEY_T: ord("T"),
                    evdev.ecodes.KEY_U: ord("U"),
                    evdev.ecodes.KEY_V: ord("V"),
                    evdev.ecodes.KEY_W: ord("W"),
                    evdev.ecodes.KEY_X: ord("X"),
                    evdev.ecodes.KEY_Y: ord("Y"),
                    evdev.ecodes.KEY_Z: ord("Z"),
                    evdev.ecodes.KEY_SPACE: ord(" "),
                    evdev.ecodes.KEY_ENTER: ord("\n"),
                    evdev.ecodes.KEY_BACKSPACE: 8,
                    evdev.ecodes.KEY_0: ord("0"),
                    evdev.ecodes.KEY_1: ord("1"),
                    evdev.ecodes.KEY_2: ord("2"),
                    evdev.ecodes.KEY_3: ord("3"),
                    evdev.ecodes.KEY_4: ord("4"),
                    evdev.ecodes.KEY_5: ord("5"),
                    evdev.ecodes.KEY_6: ord("6"),
                    evdev.ecodes.KEY_7: ord("7"),
                    evdev.ecodes.KEY_8: ord("8"),
                    evdev.ecodes.KEY_9: ord("9"),
                }
                return ascii_map.get(key_code, 0)
            elif key_state == 0:
                return 0
        return None

    def _send_keyboard_data(self, ascii_code: int) -> None:
        for bit_index, pin in enumerate(self.keyboard_data_pins):
            bit_value = (ascii_code >> bit_index) & 1
            GPIO.output(pin, GPIO.HIGH if bit_value else GPIO.LOW)

    def _monitor_loop(self) -> None:
        print("Keyboard monitoring started...")
        try:
            for event in self.keyboard_device.read_loop():
                if not self.running:
                    break
                ascii_code = self._get_key_ascii(event)
                if ascii_code is not None:
                    with self.key_lock:
                        self.current_ascii_code = ascii_code
                        self.key_pressed = ascii_code != 0
                    if ascii_code != 0:
                        try:
                            print(f"Key pressed: {chr(ascii_code)} (ASCII: {ascii_code})")
                        except Exception:
                            print(f"Key pressed (ASCII: {ascii_code})")
        except (OSError, PermissionError) as error:
            print(f"Error reading keyboard: {error}")

    def _output_loop(self) -> None:
        print("GPIO output started...")
        while self.running:
            try:
                with self.key_lock:
                    ascii_code = self.current_ascii_code
                self._send_keyboard_data(ascii_code)
                time.sleep(0.001)
            except Exception as error:
                print(f"Error sending GPIO data: {error}")
                time.sleep(0.1)

    def start(self) -> None:
        if self.running:
            return
        self.running = True

        self._monitor_thread = threading.Thread(target=self._monitor_loop, daemon=True)
        self._output_thread = threading.Thread(target=self._output_loop, daemon=True)
        self._monitor_thread.start()
        self._output_thread.start()

    def stop(self) -> None:
        self.running = False

    def join(self, timeout: Optional[float] = 1.0) -> None:
        if self._monitor_thread is not None:
            self._monitor_thread.join(timeout=timeout)
        if self._output_thread is not None:
            self._output_thread.join(timeout=timeout)

    def get_current_ascii(self) -> int:
        with self.key_lock:
            return self.current_ascii_code

