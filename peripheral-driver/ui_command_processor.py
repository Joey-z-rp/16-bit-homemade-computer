#!/usr/bin/env python3
"""
UI command processor: decodes 16-bit UI commands and renders using pygame.

Protocol
--------
- Each command is composed of up to 4 words (16-bit each).
- The first word encodes the command type in bits 14..11 (4 bits).
- Bit 15 (MSB) of each word is used to indicate word pairing:
  - Words with matching MSBs belong to the same command
  - Words with different MSBs are from different commands
- The remaining bits of the first word (bits 10..0) carry parameters per type.

Implemented commands
--------------------
- Type 0b0000 (0): Print character
  - First word: bits 7..0 contain the 8-bit ASCII code
  - Second word (required): cell position on the text grid (computed from font metrics)
      - bits 14..8: row index (0..rows-1) [bit 15 is MSB for pairing]
      - bits 7..0: column index (0..cols-1)
  - Both words must have matching MSBs to be considered a complete command.

- Type 0b0001 (1): Play sound
  - First word: bits 7..0 contain the sound sample ID, bits 10..8 contain volume level (0-7, where 0=mute, 7=full volume)
  - Musical notes (ASCII codes): 49='1'=c, 50='2'=d, 51='3'=e, 52='4'=f, 53='5'=g, 54='6'=a
  - Game sounds: 0=hit-1, 1=hit-2, 2=bounce, 3=transform, 4=game-background-music
  - Maps to sound files in sound-samples folder

- Type 0b0010 (2): Clear screen
  - First word: no parameters used
  - Clears the entire screen to the default background color

- Type 0b0011 (3): Boot
  - First word: no parameters used
  - Plays the boot.mp3 sound file and displays the boot-screen.png image
  - The boot screen is centered and scaled to fit the logical surface

- Type 0b0100 (4): Move sprite
  - First word: bits 10..7 contain sprite ID (0-15), bits 6..3 contain variant (0-15)
  - Second word: position for first sprite (210 * 128 coordinate space)
  - Third word: bits 10..7 contain sprite ID (0-15), bits 6..3 contain variant (0-15)
  - Fourth word: position for second sprite (210 * 128 coordinate space)
  - One command moves 2 sprites simultaneously
  - Sprites are loaded from images/{id}-{variant}.png files and rendered with proper rotation

- Type 0b0101 (5): Debug
  - First word: bits 10..0 contain debug data (11 bits)
  - Second word: debug data (16 bits)
  - Third word: debug data (16 bits)
  - Fourth word: debug data (16 bits)
  - All 4 words must have matching MSBs to be considered a complete command
  - Prints debug information to console; no UI changes

Rendering
---------
- Maintains a logical 210x128 surface (grid space). The window is resizable and
  starts as large as the current display. Each frame, the logical surface is
  scaled to the window size while preserving aspect ratio and centered.
- Draws white text on a dark gray background on the logical surface; scaling to
  window happens automatically.
- Colors are configurable through class constants: BACKGROUND_COLOR, TEXT_COLOR, and BLACK_COLOR.

Text grid size depends on the fixed font. Use get_text_grid_size() to query
(cols, rows). Each cell is mapped to pixel space using the font's width/height.
"""

from __future__ import annotations

import threading
import queue
import os
from typing import List, Optional, Tuple


class UICommandProcessor:
    SCREEN_WIDTH: int = 210
    SCREEN_HEIGHT: int = 128
    BACKGROUND_COLOR: Tuple[int, int, int] = (40, 40, 40)  # Dark gray background
    TEXT_COLOR: Tuple[int, int, int] = (255, 255, 255)  # White text
    BLACK_COLOR: Tuple[int, int, int] = (0, 0, 0)

    # Command type constants
    CMD_PRINT_CHAR: int = 0b0000
    CMD_PLAY_SOUND: int = 0b0001
    CMD_CLEAR_SCREEN: int = 0b0010
    CMD_BOOT: int = 0b0011
    CMD_MOVE_SPRITE: int = 0b0100
    CMD_DEBUG: int = 0b0101

    # Sprite size mapping (hardcoded sizes for each sprite ID)
    SPRITE_SIZES: dict[int, Tuple[int, int]] = {
        0: (25, 30), 
        1: (10, 20),
    }

    def __init__(
        self,
        run_in_background: Optional[bool] = None,
    ) -> None:
        # Lazy import pygame to avoid import errors on systems without display
        import pygame  # type: ignore

        self._pygame = pygame
        self._screen = None
        self._logical_surface = None
        self._font = None
        self._clock = None
        self._resizable_window = False
        # Fixed font configuration
        self._font_name: Optional[str] = "monospace"
        self._font_size: int = 12
        self._font_bold: bool = False
        # Derived text grid metrics (computed after font init)
        self._cell_width: int = 0
        self._cell_height: int = 0
        self._num_cols: int = 0
        self._num_rows: int = 0
        # Adjustable extra spacing between cells (can be negative)
        self._cell_spacing_x: int = -0.5
        self._cell_spacing_y: int = 0

        self._command_queue: "queue.Queue[List[int]]" = queue.Queue(maxsize=256)
        self._running: bool = False
        self._thread: Optional[threading.Thread] = None
        self._run_in_background: bool = True
        
        # Sprite management
        self._sprites: dict = {}  # sprite_id -> {variant, rotation, x, y}
        self._sprite_images: dict = {}  # (sprite_id, variant) -> pygame.Surface

        # Sound system
        self._sounds: dict[int, pygame.mixer.Sound] = {}
        # Map sound IDs to sound files - combining original musical notes and new game sounds
        self._sound_mapping = {
            # Original musical notes (ASCII codes)
            49: 'c', 50: 'd', 51: 'e', 52: 'f', 53: 'g', 54: 'a',
            # New game sounds
            0: 'hit-1', 1: 'hit-2', 2: 'bounce', 3: 'transform', 4: 'game-background-music'
        }

        # Initialize pygame subsystems (no window/context yet)
        self._pygame.display.init()
        self._pygame.font.init()
        self._pygame.mixer.init()
        self._pygame.mixer.music.set_volume(1)

        # Decide threading model based on backend if not specified
        if run_in_background is None:
            driver = self._pygame.display.get_driver().lower()
            # Some backends are picky about rendering on the main thread
            # (e.g., kmsdrm, fbcon). For those, keep everything foreground.
            self._run_in_background = driver not in ("kmsdrm", "fbcon")
        else:
            self._run_in_background = bool(run_in_background)

        # Create the window and GL/EGL context on the same thread that will render
        if self._run_in_background:
            try:
                print(f"Pygame driver: {driver} | rendering in background thread")
            except Exception:
                pass
            self._start_thread()
        else:
            try:
                print(f"Pygame driver: {driver} | rendering in foreground on main thread")
            except Exception:
                pass
            self._init_display_and_resources()

    def _init_display_and_resources(self) -> None:
        pygame = self._pygame

        driver = pygame.display.get_driver()
        flags = 0
        if driver.lower() in ("kmsdrm", "fbcon"):
            # Fullscreen for direct-to-display backends; no resizable
            flags |= pygame.FULLSCREEN
            self._resizable_window = False
            self._screen = pygame.display.set_mode((0, 0), flags)
        else:
            # Resizable window on desktop environments
            info = pygame.display.Info()
            start_size = (info.current_w, info.current_h)
            flags |= pygame.RESIZABLE
            self._resizable_window = True
            self._screen = pygame.display.set_mode(start_size, flags)

        pygame.display.set_caption("UI Renderer (Logical 210x128)")
        # Hide the mouse cursor
        pygame.mouse.set_visible(False)
        self._clock = pygame.time.Clock()
        # Logical surface where all drawing happens in grid coordinates
        self._logical_surface = pygame.Surface((self.SCREEN_WIDTH, self.SCREEN_HEIGHT)).convert()
        # Configure font; prefer monospace, fall back to default
        try:
            if self._font_name is None:
                self._font = pygame.font.SysFont(None, self._font_size, bold=self._font_bold)
            else:
                self._font = pygame.font.SysFont(self._font_name, self._font_size, bold=self._font_bold)
        except Exception:
            self._font = pygame.font.SysFont(None, self._font_size, bold=self._font_bold)
        # Compute character cell metrics and grid size
        self._recompute_text_grid_metrics()
        # Clear logical and screen to black initially
        self._logical_surface.fill(self.BACKGROUND_COLOR)
        self._screen.fill(self.BACKGROUND_COLOR)
        pygame.display.flip()
        
        # Load sound samples
        self._load_sound_samples()
        
        # Load sprite images
        self._load_sprite_images()
        
        try:
            width, height = self._screen.get_size()
            print(f"UI window created: {width}x{height} (resizable={self._resizable_window})")
        except Exception:
            pass

    def _load_sound_samples(self) -> None:
        """Load sound samples from the sound-samples folder."""
        try:
            for sample_id, sound_name in self._sound_mapping.items():
                # Try different file extensions
                sound_file = None
                for ext in ['.wav', '.mp3', '.ogg']:
                    test_file = os.path.join("sound-samples", f"{sound_name}{ext}")
                    if os.path.exists(test_file):
                        sound_file = test_file
                        break
                
                if sound_file:
                    try:
                        self._sounds[sample_id] = self._pygame.mixer.Sound(sound_file)
                        print(f"Loaded sound sample {sample_id} -> {sound_name}")
                    except Exception as e:
                        print(f"Warning: Could not load sound file {sound_file}: {e}")
                else:
                    print(f"Warning: Sound file for {sound_name} not found (tried .wav, .mp3, .ogg)")
        except Exception as e:
            print(f"Warning: Could not load sound samples: {e}")

    def _load_sprite_images(self) -> None:
        """Load sprite images from the images folder."""
        try:
            for sprite_id in range(16):  # Support sprite IDs 0-15
                for variant in range(16):  # Support variants 0-15
                    sprite_file = os.path.join("images", f"{sprite_id}-{variant}.png")
                    if os.path.exists(sprite_file):
                        try:
                            image = self._pygame.image.load(sprite_file).convert_alpha()
                            self._sprite_images[(sprite_id, variant)] = image
                            print(f"Loaded sprite {sprite_id}-{variant}.png")
                        except Exception as e:
                            print(f"Warning: Could not load sprite {sprite_id}-{variant}.png: {e}")
                    # Don't warn for missing variants - they're optional
        except Exception as e:
            print(f"Warning: Could not load sprite images: {e}")

    def _ensure_initialized(self) -> None:
        """Ensure that the display and render resources are created on this thread."""
        if self._screen is None or self._logical_surface is None or self._clock is None or self._font is None:
            self._init_display_and_resources()

    def _start_thread(self) -> None:
        if self._running:
            return
        self._running = True
        self._thread = threading.Thread(target=self._render_loop, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._running = False
        if self._thread is not None:
            self._thread.join(timeout=1.0)
        # Allow pygame to quit gracefully
        try:
            self._pygame.quit()
        except Exception:
            pass

    def enqueue_command(self, command_words: List[int]) -> None:
        """
        Thread-safe method to queue an incoming UI command for processing.
        Expects a list of up to 4 words (integers 0..65535).
        """
        if not isinstance(command_words, list) or len(command_words) == 0:
            return
        # Copy and trim to max 4 words
        words = [int(w) & 0xFFFF for w in command_words[:4]]
        try:
            self._command_queue.put_nowait(words)
        except queue.Full:
            # Drop command if overwhelmed
            pass

    # === Internal helpers ===

    def _render_loop(self) -> None:
        pygame = self._pygame
        # Create the window/context on this thread to satisfy EGL/GL thread affinity
        self._ensure_initialized()
        screen = self._screen
        clock = self._clock
        assert screen is not None and clock is not None

        while self._running:
            self._process_frame()
            clock.tick(60)

    def _process_frame(self) -> None:
        pygame = self._pygame
        # Process window events so the window stays responsive
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                self._running = False
                break
            if self._resizable_window and event.type == pygame.VIDEORESIZE:
                # Recreate screen with new size
                self._screen = pygame.display.set_mode((event.w, event.h), pygame.RESIZABLE)

        # Drain and process queued commands
        try:
            words = self._command_queue.get_nowait()
            self._process_command(words)
        except queue.Empty:
            pass

        # Render sprites
        self._render_sprites()

        # Present the logical surface scaled to current window size
        self._present()

    def _process_command(self, words: List[int]) -> None:
        if not words:
            return

        first_word = words[0] & 0xFFFF
        # Type is in bits 14..11 (4 bits), with bit 15 being MSB for pairing
        command_type = (first_word >> 11) & 0b1111

        if command_type == self.CMD_PRINT_CHAR:
            self._handle_print_char(words)
        elif command_type == self.CMD_PLAY_SOUND:
            self._handle_play_sound(words)
        elif command_type == self.CMD_CLEAR_SCREEN:
            self._handle_clear_screen(words)
        elif command_type == self.CMD_BOOT:
            self._handle_boot(words)
        elif command_type == self.CMD_MOVE_SPRITE:
            self._handle_move_sprite(words)
        elif command_type == self.CMD_DEBUG:
            self._handle_debug(words)
        else:
            # Unknown command types are currently ignored
            return

    def _handle_print_char(self, words: List[int]) -> None:
        ascii_code = (words[0]) & 0xFF
        
        # Filter out non-displayable characters (control characters, etc.)
        if ascii_code < 32 or ascii_code == 127:  # Control characters and DEL
            return  # Skip non-displayable characters
            
        character = chr(ascii_code)

        # Check if we have the required second word for position
        if len(words) < 2:
            # Ignore incomplete commands
            return

        # Decode coordinates from second word
        second_word = words[1] & 0xFFFF
        # Y in bits 14..8 (bit 15 is MSB for pairing), X in LSB 8 bits
        y_value = (second_word >> 8) & 0x7F  # 7 bits for Y (0-127)
        x_value = second_word & 0xFF
        # Always treat as cell coordinates
        x_position = int(x_value) * max(1, self._cell_width)
        y_position = int(y_value) * max(1, self._cell_height)

        # Clamp to screen bounds based on cell size; ensure top-left stays within bounds
        max_x = self.SCREEN_WIDTH - self._cell_width
        max_y = self.SCREEN_HEIGHT - self._cell_height
        x_position = max(0, min(max_x, x_position))
        y_position = max(0, min(max_y, y_position))

        self._draw_character(character, (x_position, y_position))

    def _handle_play_sound(self, words: List[int]) -> None:
        """Handle play sound command."""
        sound_id = (words[0]) & 0xFF
        volume_level = (words[0] >> 8) & 0b111  # Extract volume from bits 10..8 (0-7)
        
        if sound_id in self._sounds:
            try:
                # Special case for game background music - stop any currently playing background music
                if sound_id == 4:  # game-background-music
                    self._pygame.mixer.stop()
                
                # Convert volume from 0-7 range to 0.0-1.0 range for pygame
                # Special case: if volume is 0, play at full volume (1.0) instead of mute
                if volume_level == 0:
                    volume_normalized = 1.0
                else:
                    volume_normalized = volume_level / 7.0
                self._sounds[sound_id].set_volume(volume_normalized)
                self._sounds[sound_id].play()
                # Show the actual volume being used in the print statement
                actual_volume_display = "0/7 (playing at full volume)" if volume_level == 0 else f"{volume_level}/7"
                print(f"Playing sound sample {sound_id} ({self._sound_mapping.get(sound_id, 'unknown')}) at volume {actual_volume_display} ({volume_normalized:.2f})")
            except Exception as e:
                print(f"Error playing sound {sound_id}: {e}")
        else:
            print(f"Warning: Sound sample {sound_id} not loaded")

    def _handle_clear_screen(self, words: List[int]) -> None:
        """Handle clear screen command."""
        # Clear the logical surface to black
        self._logical_surface.fill(self.BACKGROUND_COLOR)
        # Clear all sprites
        self._sprites.clear()
        # Reset the white background flag so new sprites get fresh white background
        if hasattr(self, '_white_background_set'):
            delattr(self, '_white_background_set')
        # No immediate screen update; presentation happens each frame in _present

    def _handle_boot(self, words: List[int]) -> None:
        """Handle boot command."""
        pygame = self._pygame
        
        # Play boot sound (boot.mp3)
        try:
            boot_sound_path = os.path.join("sound-samples", "boot.mp3")
            if os.path.exists(boot_sound_path):
                pygame.mixer.music.load(boot_sound_path)
                pygame.mixer.music.play()
                print("Playing boot sound: boot.mp3")
            else:
                print(f"Warning: Boot sound file {boot_sound_path} not found")
        except Exception as e:
            print(f"Error playing boot sound: {e}")

        # Display boot screen (boot-screen.png)
        try:
            boot_screen_path = os.path.join("images", "boot-screen.png")
            if os.path.exists(boot_screen_path):
                boot_surface = pygame.image.load(boot_screen_path).convert()
                # Scale boot screen to fit the logical surface while preserving aspect ratio
                scale_factor = min(self.SCREEN_WIDTH / boot_surface.get_width(), 
                                 self.SCREEN_HEIGHT / boot_surface.get_height())
                scaled_boot_surface = pygame.transform.smoothscale(boot_surface, (
                    int(boot_surface.get_width() * scale_factor),
                    int(boot_surface.get_height() * scale_factor)
                ))
                
                # Center the boot screen on the logical surface
                x_offset = (self.SCREEN_WIDTH - scaled_boot_surface.get_width()) // 2
                y_offset = (self.SCREEN_HEIGHT - scaled_boot_surface.get_height()) // 2
                
                # Clear the logical surface and draw the boot screen
                self._logical_surface.fill(self.BACKGROUND_COLOR)
                self._logical_surface.blit(scaled_boot_surface, (x_offset, y_offset))
                
                print("Boot screen displayed on logical surface")
            else:
                print(f"Warning: Boot screen image {boot_screen_path} not found")
        except Exception as e:
            print(f"Error displaying boot screen: {e}")

    def _handle_move_sprite(self, words: List[int]) -> None:
        """Handle move sprite command.
        
        Command format: 4 words
        - Word 1: command type (0100) + sprite_id (bits 10..7) + variant (bits 6..3)
        - Word 2: position for first sprite (210 * 128 coordinate space)
        - Word 3: command type (0100) + sprite_id (bits 10..7) + variant (bits 6..3)
        - Word 4: position for second sprite (210 * 128 coordinate space)
        """
        if len(words) < 4:
            # Need all 4 words for this command
            return
            
        # Decode first sprite from words 1
        first_word = words[0] & 0xFFFF
        sprite_id_1 = (first_word >> 7) & 0b1111  # bits 10..7
        variant_1 = (first_word >> 3) & 0b1111    # bits 6..3
        
        # Decode first sprite position from word 2
        second_word = words[1] & 0xFFFF
        x1 = second_word & 0xFF        # bits 7..0 for X (0-209)
        y1 = (second_word >> 8) & 0x7F # bits 14..8 for Y (0-127)
        
        # Decode second sprite from words 3 and 4
        third_word = words[2] & 0xFFFF
        sprite_id_2 = (third_word >> 7) & 0b1111  # bits 10..7
        variant_2 = (third_word >> 3) & 0b1111    # bits 6..3
        
        # Decode second sprite position from word 4
        fourth_word = words[3] & 0xFFFF
        x2 = fourth_word & 0xFF        # bits 7..0 for X (0-209)
        y2 = (fourth_word >> 8) & 0x7F # bits 14..8 for Y (0-127)
        
        # Store sprite information
        # Clear old sprites with the same ID first to prevent overlap
        old_keys_to_remove = []
        for existing_key in self._sprites.keys():
            existing_sprite_id, existing_variant = existing_key
            # Clear ALL variants of the sprites being moved by this command
            if existing_sprite_id == sprite_id_1 or existing_sprite_id == sprite_id_2:
                old_keys_to_remove.append(existing_key)
        
        # Remove old sprites
        for key in old_keys_to_remove:
            del self._sprites[key]
        
        # Now add the new sprites
        self._sprites[(sprite_id_1, variant_1)] = {
            'variant': variant_1,
            'x': x1,
            'y': y1
        }
        
        self._sprites[(sprite_id_2, variant_2)] = {
            'variant': variant_2,
            'x': x2,
            'y': y2
        }
        
        print(f"Moved sprite {sprite_id_1} (variant {variant_1}) to ({x1}, {y1})")
        print(f"Moved sprite {sprite_id_2} (variant {variant_2}) to ({x2}, {y2})")

    def _handle_debug(self, words: List[int]) -> None:
        """Handle debug command.
        
        Command format: 4 words
        - Word 1: command type (0101) + debug data (bits 10..0)
        - Word 2: debug data (16 bits)
        - Word 3: debug data (16 bits)
        - Word 4: debug data (16 bits)
        """
        if len(words) < 4:
            # Need all 4 words for this command
            return
            
        # Extract debug data from all words
        first_word = words[0] & 0xFFFF
        debug_data_1 = first_word & 0x7FF  # bits 10..0 (excluding command type)
        debug_data_2 = words[1] & 0xFFFF   # bits 15..0
        debug_data_3 = words[2] & 0xFFFF   # bits 15..0
        debug_data_4 = words[3] & 0xFFFF   # bits 15..0
        
        # Print debug information
        print(f"DEBUG: Data1={debug_data_1}, Data2={debug_data_2}, Data3={debug_data_3}, Data4={debug_data_4}")
        
        # No UI changes for debug commands

    def _render_sprites(self) -> None:
        """Render all sprites on the logical surface using real images."""
        if not self._sprites:
            return
            
        pygame = self._pygame
        assert self._logical_surface is not None
        
        # Clear the screen first to remove any old sprites
        self._logical_surface.fill((255, 255, 255))  # White background for sprites
        
        for sprite_key, sprite_data in self._sprites.items():
            sprite_id, variant = sprite_key
            x = sprite_data['x']
            y = sprite_data['y']
            
            
            # Try to get the sprite image
            if sprite_key in self._sprite_images:
                # Use the real sprite image
                sprite_image = self._sprite_images[sprite_key]
                
                # Instead of scaling down to logical size first, let's render at higher quality
                # We'll use the original image size but position it according to logical coordinates
                original_width, original_height = sprite_image.get_size()
                
                # Apply rotation if needed (on the original high-quality image)
                # No rotation logic here, as rotation is removed from command
                rotated_image = sprite_image
                
                # Get the rotated image dimensions
                img_width, img_height = rotated_image.get_size()
                
                # Calculate the logical size this sprite should occupy
                if sprite_id in self.SPRITE_SIZES:
                    logical_width, logical_height = self.SPRITE_SIZES[sprite_id]
                    
                    # Instead of scaling down the image first, let's scale it to fit the logical size
                    # while maintaining the best possible quality
                    scale_x = logical_width / img_width
                    scale_y = logical_height / img_height
                    scale_factor = min(scale_x, scale_y)  # Use the smaller scale to fit within bounds
                    
                    # Calculate the final dimensions that preserve aspect ratio
                    final_width = int(img_width * scale_factor)
                    final_height = int(img_height * scale_factor)
                    
                    # Scale the rotated image to fit within logical bounds
                    # Use smoothscale for better quality since we're scaling from high-res to low-res
                    final_sprite = pygame.transform.smoothscale(rotated_image, (final_width, final_height)).convert_alpha()
                    
                    # Debug: Show physical vs logical dimensions
                    if self._screen is not None:
                        window_width, window_height = self._screen.get_size()
                        scale_x_physical = window_width / self.SCREEN_WIDTH
                        scale_y_physical = window_height / self.SCREEN_HEIGHT
                    else:
                        print(f"  Sprite {sprite_id}-{variant}: rotated 0°, logical size {logical_width}x{logical_height}, final size {final_width}x{final_height}")
                    
                    # Center the sprite at the specified position using logical size for positioning
                    draw_x = x - (logical_width // 2)
                    draw_y = y - (logical_height // 2)
                    
                    # Clamp to logical surface bounds
                    draw_x = max(0, min(self.SCREEN_WIDTH - logical_width, draw_x))
                    draw_y = max(0, min(self.SCREEN_HEIGHT - logical_height, draw_y))
                    
                    # Draw the sprite centered within its logical bounds
                    sprite_x = draw_x + (logical_width - final_width) // 2
                    sprite_y = draw_y + (logical_height - final_height) // 2
                    
                    self._logical_surface.blit(final_sprite, (sprite_x, sprite_y))
                    
                else:
                    # No size specification, use rotated image as-is
                    draw_x = x - (img_width // 2)
                    draw_y = y - (img_height // 2)
                    draw_x = max(0, min(self.SCREEN_WIDTH - img_width, draw_x))
                    draw_y = max(0, min(self.SCREEN_HEIGHT - img_height, draw_y))
                    self._logical_surface.blit(rotated_image, (draw_x, draw_y))
                    print(f"  Using rotated size {img_width}x{img_height} (no size spec)")
                
            else:
                print(f"Warning: Sprite {sprite_id}-{variant} not found")

    def _draw_character(self, character: str, position: Tuple[int, int]) -> None:
        pygame = self._pygame
        assert self._logical_surface is not None and self._font is not None

        # Clear the target cell before drawing new character to avoid overlap
        cell_rect = pygame.Rect(position[0], position[1], self._cell_width, self._cell_height+1)
        self._logical_surface.fill(self.BACKGROUND_COLOR, cell_rect)

        # Render character in white; background cleared per-cell above
        # Antialiasing disabled for crisp pixel glyphs
        text_surface = self._font.render(character, False, self.TEXT_COLOR)
        self._logical_surface.blit(text_surface, position)
        # No immediate screen update; presentation happens each frame in _present

    def _recompute_text_grid_metrics(self) -> None:
        """Compute character cell width/height and the number of columns/rows.

        Uses font metrics so that the text cell layout aligns glyphs cleanly.
        """
        assert self._font is not None
        # Width: use advance of 'M' as representative for monospace fonts
        base_width, _ = self._font.size("M")
        # Height: use tight metrics (ascent + descent) to reduce extra leading
        ascent = self._font.get_ascent()
        descent = self._font.get_descent()
        base_height = max(1, int(ascent + max(0, descent)))
        # Apply user-adjustable spacing (can be negative)
        self._cell_width = max(1, int(base_width + self._cell_spacing_x))
        self._cell_height = max(1, int(base_height + self._cell_spacing_y))
        self._num_cols = max(1, self.SCREEN_WIDTH // self._cell_width)
        self._num_rows = max(1, self.SCREEN_HEIGHT // self._cell_height)


    # === Public helpers ===
    def get_text_grid_size(self) -> Tuple[int, int]:
        """Return (num_columns, num_rows) for the current font.
        """
        return (self._num_cols, self._num_rows)

    def get_max_cell_indices(self) -> Tuple[int, int]:
        """Return (max_x_index, max_y_index) usable in commands.

        This is simply (num_columns - 1, num_rows - 1).
        """
        return (max(0, self._num_cols - 1), max(0, self._num_rows - 1))

    def get_sprites(self) -> dict:
        """Return the current sprite information dictionary.
        
        Returns a dict mapping sprite_id to sprite data:
        {sprite_id: {'variant': int, 'rotation': int, 'x': int, 'y': int}}
        """
        return self._sprites.copy()

    def _present(self) -> None:
        pygame = self._pygame
        assert self._screen is not None and self._logical_surface is not None

        window_width, window_height = self._screen.get_size()
        # Stretch to fill the entire window (ignore aspect ratio)
        scaled_surface = pygame.transform.smoothscale(
            self._logical_surface, (max(1, int(window_width)), max(1, int(window_height)))
        )

        self._screen.fill(self.BLACK_COLOR)
        self._screen.blit(scaled_surface, (0, 0))
        pygame.display.flip()

    # === Public run helpers for foreground mode ===
    def run_for(self, seconds: float) -> None:
        """Run the render loop on the current thread for a duration (seconds)."""
        if not self._running:
            self._running = True
        # Ensure we are not already using the background thread
        if self._thread is not None and self._thread.is_alive():
            # Already running in background; just sleep
            import time as _time
            _time.sleep(max(0.0, float(seconds)))
            return
        import time as _time
        # Ensure display initialized on this thread
        self._ensure_initialized()
        end_time = _time.time() + max(0.0, float(seconds))
        while self._running and _time.time() < end_time:
            self._process_frame()
            assert self._clock is not None
            self._clock.tick(60)

    def run_forever(self) -> None:
        """Run the render loop on the current thread until stop() is called or window is closed."""
        if not self._running:
            self._running = True
        if self._thread is not None and self._thread.is_alive():
            return
        assert self._clock is not None
        # Ensure display initialized on this thread
        self._ensure_initialized()
        while self._running:
            self._process_frame()
            self._clock.tick(60)

    def run_forever_if_foreground(self) -> None:
        """Helper: only run on the current thread if foreground mode is required."""
        if not self._run_in_background:
            self.run_forever()


__all__ = ["UICommandProcessor"]

