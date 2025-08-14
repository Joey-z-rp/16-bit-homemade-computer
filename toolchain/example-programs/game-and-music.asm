// Temporary variable
@temp
M=0

// The number of loops to delay
@delay_loop_number
M=0
// The address to jump to after the delay
@delay_return_address
M=0
// The delay counter
@delay_counter
M=0

// The string array address
@print_string_address
M=0
// The number of characters in the string
@print_string_length
M=0
// Print string return address
@print_string_return_address
M=0
// Current character index for positioning (x-coordinate)
@print_string_index
M=0
// Store the print string command before sending
@print_string_cmd_1
M=0
@print_string_cmd_2
M=0

// Return address for clear screen function
@clear_screen_return_address
M=0

// Return address for toggle MSB flag function
@toggle_msb_return
M=0

// Return address for play bounce sound function
@play_bounce_sound_return
M=0

// The last key pressed
@last_key
M=0

// 0: selecting action
// 1: music
// 2: game
@mode
M=0

@is_mode_prompt_printed
M=0

// Buffer for storing user input (max 30 characters)
@input_buffer_start
M=0
@input_buffer_length
M=0
@input_buffer_index
M=0

// Temporary address for buffer operations
@current_buffer_addr
M=0

// Current musical note character for display
@current_note
M=0

// Random number generator
@random_number_counter
M=0
@random_number
M=0

// Game state variables
@10
D=A
@bat_x
M=D
@50
D=A
@bat_y
M=D
@100
D=A
@ball_x
M=D
@50
D=A
@ball_y
M=D

// Game speed control - only update game every N main loops
@game_speed_counter
M=0
@10
D=A
@game_speed_divisor
M=D  // Update game every 10 main loops (adjust for speed)

@boosted_ball_speed
M=0

// Ball velocity (positive = right/down, negative = left/up)
@0
D=A
@2
D=D-A
@ball_vel_x
M=D    // Start moving left
@ball_vel_y
M=1    // Start moving down

// Bat movement speed
@2
D=A
@bat_speed
M=D    // Bat moves 1 pixel per input

@bat_variant
M=0

// Bat collided with ball flag
@bat_collided_with_ball
M=0
// Bat upgraded flag
@bat_upgraded
M=0

@2
D=A
@collision_position_offset
M=D

// Game update commands
@game_update_command_1
M=0
@game_update_command_2
M=0
@game_update_command_3
M=0
@game_update_command_4
M=0

// Game boundaries
@210
D=A
@screen_width
M=D
@128
D=A
@screen_height
M=D
@12
D=A
@bat_half_width
M=D
@13
D=A
@bat_half_height
M=D
@5
D=A
@ball_half_width
M=D
@10
D=A
@ball_half_height
M=D

// Command synchronization: MSB toggle flag (0 = MSB=0, 32768 = MSB=1)
@cmd_sync_flag
M=0

// Input buffer storage area (max 30 characters) starting at 340
@340
D=A
@input_buffer_start
M=D

// Select action prompt stored at 300: "SELECT ACTION"
@300
D=A
@select_action_prompt_address
M=D
// The number of characters in the select action prompt
@14
D=A
@select_action_prompt_length
M=D
// "S"
@83
D=A
@300
M=D
// "E"
@69
D=A
@301
M=D
// "L"
@76
D=A
@302
M=D
// "E"
@69
D=A
@303
M=D
// "C"
@67
D=A
@304
M=D
// "T"
@84
D=A
@305
M=D
// " "
@32
D=A
@306
M=D
// "A"
@65
D=A
@307
M=D
// "C"
@67
D=A
@308
M=D
// "T"
@84
D=A
@309
M=D
// "I"
@73
D=A
@310
M=D
// "O"
@79
D=A
@311
M=D
// "N"
@78
D=A
@312
M=D
// ":"
@58
D=A
@313
M=D

// Music prompt stored at 314: "PRESS NUMBER"
@314
D=A
@music_prompt_address
M=D
// The number of characters in the music prompt
@12
D=A
@music_prompt_length
M=D
// "P"
@80
D=A
@314
M=D
// "R"
@82
D=A
@315
M=D
// "E"
@69
D=A
@316
M=D
// "S"
@83
D=A
@317
M=D
// "S"
@83
D=A
@318
M=D
// " "
@32
D=A
@319
M=D
// "N"
@78
D=A
@320
M=D
// "U"
@85
D=A
@321
M=D
// "M"
@77
D=A
@322
M=D
// "B"
@66
D=A
@323
M=D
// "E"
@69
D=A
@324
M=D
// "R"
@82
D=A
@325
M=D

// "INVALID ACTION" message stored at 326
@326
D=A
@invalid_action_address
M=D
// The number of characters in the invalid action message
@14
D=A
@invalid_action_length
M=D
// "I"
@73
D=A
@326
M=D
// "N"
@78
D=A
@327
M=D
// "V"
@86
D=A
@328
M=D
// "A"
@65
D=A
@329
M=D
// "L"
@76
D=A
@330
M=D
// "I"
@73
D=A
@331
M=D
// "D"
@68
D=A
@332
M=D
// " "
@32
D=A
@333
M=D
// "A"
@65
D=A
@334
M=D
// "C"
@67
D=A
@335
M=D
// "T"
@84
D=A
@336
M=D
// "I"
@73
D=A
@337
M=D
// "O"
@79
D=A
@338
M=D
// "N"
@78
D=A
@339
M=D

// Send boot command
@6144
D=A
@UI_CMD_1
M=D
@15000
D=A
@delay_loop_number
M=D
@MAIN
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(MAIN)
// Generate random number
@random_number_counter
M=M+1
D=M
@3
D=D&A
@random_number // 0, 1 or 2
M=D

// Delay between each main loop
@100
D=A
@delay_loop_number
M=D
@MODE_PROMPT
D=A
@delay_return_address // jump to MODE_PROMPT after delay
M=D
@DELAY
0;JMP

(MODE_PROMPT)
// only print the prompt once
@is_mode_prompt_printed
D=M
@KEYBOARD_CHECK
D;JGT
@mode
D=M
@MUSIC_PROMPT
D;JGT

// Select action prompt
// Clear the screen first
@CLEAR_SCREEN_THEN_PRINT_SELECT_PROMPT
D=A
@clear_screen_return_address
M=D
@CLEAR_SCREEN
0;JMP

(CLEAR_SCREEN_THEN_PRINT_SELECT_PROMPT)
@select_action_prompt_address
D=M
@print_string_address
M=D
@select_action_prompt_length
D=M
@print_string_length
M=D
@SET_MODE_PROMPT_PRINTED
D=A
@print_string_return_address
M=D
@PRINT_STRING
0;JMP

// Music prompt
(MUSIC_PROMPT)
// Clear the screen first
@CLEAR_SCREEN_THEN_PRINT_MUSIC_PROMPT
D=A
@clear_screen_return_address
M=D
@CLEAR_SCREEN
0;JMP

(CLEAR_SCREEN_THEN_PRINT_MUSIC_PROMPT)
@music_prompt_address
D=M
@print_string_address
M=D
@music_prompt_length
D=M
@print_string_length
M=D
@SET_MODE_PROMPT_PRINTED
D=A
@print_string_return_address
M=D
@PRINT_STRING
0;JMP

(SET_MODE_PROMPT_PRINTED)
@is_mode_prompt_printed
M=1

(KEYBOARD_CHECK)
// If it's game mode, jump to game update
@mode
D=M
@2
D=D-A
@GAME
D;JEQ
// If the last key is the same as the current key, skip the rest of the program
@KEYBOARD
D=M
@last_key
D=D-M
@MAIN
D;JEQ
@KEYBOARD
D=M
@last_key
M=D
// Process based on the mode
@mode
D=M
@SELECT_ACTION
D;JEQ
@MUSIC
0;JMP

(SELECT_ACTION)
// Check if no key is pressed (keyboard reading is 0)
@KEYBOARD
D=M
@MAIN
D;JEQ

// Check if Enter key is pressed (ASCII 10)
@KEYBOARD
D=M
@10
D=D-A
@CHECK_ENTER
D;JEQ

// Check if buffer is full (max 30 characters)
@input_buffer_length
D=M
@30
D=D-A
@MAIN
D;JGE

// Store the character in the buffer
@input_buffer_start
D=M
@input_buffer_index
D=D+M
@current_buffer_addr
M=D
@KEYBOARD
D=M
@current_buffer_addr
A=M
M=D

// Print the character to the screen at the current position
@KEYBOARD
D=M
// Add MSB flag to the character
@cmd_sync_flag
D=D+M
@UI_CMD_1
M=D
@input_buffer_index
D=M
// Print with Y=1
@256
D=D+A
// Add MSB flag to the position
@cmd_sync_flag
D=D+M
@UI_CMD_2
M=D

// Toggle the command sync flag for next command
@TOGGLE_MSB_RETURN_SELECT_ACTION
D=A
@toggle_msb_return
M=D
@TOGGLE_MSB_FLAG
0;JMP

(TOGGLE_MSB_RETURN_SELECT_ACTION)
// Increment buffer index and length
@input_buffer_index
M=M+1
@input_buffer_length
M=M+1

@MAIN
0;JMP

(CHECK_ENTER)
// Enter key pressed - check if input equals "music"
@input_buffer_length
D=M
@5
D=D-A
@CHECK_MUSIC_COMMAND
D;JEQ
@input_buffer_length
D=M
@4
D=D-A
@CHECK_GAME_COMMAND
D;JEQ
@INVALID_ACTION
0;JMP

(CHECK_MUSIC_COMMAND)
// Check each character of "MUSIC"
@input_buffer_start
A=M
D=M
@77 // 'M'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@1
D=D+A
A=D
D=M
@85 // 'U'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@2
D=D+A
A=D
D=M
@83 // 'S'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@3
D=D+A
A=D
D=M
@73 // 'I'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@4
D=D+A
A=D
D=M
@67 // 'C'
D=D-A
@INVALID_ACTION
D;JNE

// Clear the input buffer
@input_buffer_length
M=0
@input_buffer_index
M=0
// Enter music mode
@1
D=A
@mode
M=D
@is_mode_prompt_printed
M=0
@MAIN
0;JMP

(CHECK_GAME_COMMAND)
@input_buffer_start
A=M
D=M
@71 // 'G'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@1
D=D+A
A=D
D=M
@65 // 'A'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@2
D=D+A
A=D
D=M
@77 // 'M'
D=D-A
@INVALID_ACTION
D;JNE

@input_buffer_start
D=M
@3
D=D+A
A=D
D=M
@69 // 'E'
D=D-A
@INVALID_ACTION
D;JNE

// Clear the input buffer
@input_buffer_length
M=0
@input_buffer_index
M=0
// Enter game mode
@2
D=A
@mode
M=D
@MAIN
0;JMP

(INVALID_ACTION)
// Print "INVALID ACTION" message
// Clear the screen first
@CLEAR_SCREEN_THEN_PRINT_INVALID_ACTION
D=A
@clear_screen_return_address
M=D
@CLEAR_SCREEN
0;JMP

(CLEAR_SCREEN_THEN_PRINT_INVALID_ACTION)
@invalid_action_address
D=M
@print_string_address
M=D
@invalid_action_length
D=M
@print_string_length
M=D
@INVALID_ACTION_PRINTED
D=A
@print_string_return_address
M=D
@PRINT_STRING
0;JMP

(INVALID_ACTION_PRINTED)
// Clear the input buffer
@input_buffer_length
M=0
@input_buffer_index
M=0
@MAIN
0;JMP

(MUSIC)
// Check if no key is pressed (keyboard reading is 0)
@KEYBOARD
D=M
@MAIN
D;JEQ

// Check if key is 1-6 for music notes
@KEYBOARD
D=M
@49 // ASCII '1'
D=D-A
@CHECK_KEY_1
D;JEQ

@KEYBOARD
D=M
@50 // ASCII '2'
D=D-A
@CHECK_KEY_2
D;JEQ

@KEYBOARD
D=M
@51 // ASCII '3'
D=D-A
@CHECK_KEY_3
D;JEQ

@KEYBOARD
D=M
@52 // ASCII '4'
D=D-A
@CHECK_KEY_4
D;JEQ

@KEYBOARD
D=M
@53 // ASCII '5'
D=D-A
@CHECK_KEY_5
D;JEQ

@KEYBOARD
D=M
@54 // ASCII '6'
D=D-A
@CHECK_KEY_6
D;JEQ

// If key is not 1-6, ignore it
@MAIN
0;JMP

// Handle key 1 (C note)
(CHECK_KEY_1)
@67 // ASCII 'C'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Handle key 2 (D note)
(CHECK_KEY_2)
@68 // ASCII 'D'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Handle key 3 (E note)
(CHECK_KEY_3)
@69 // ASCII 'E'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Handle key 4 (F note)
(CHECK_KEY_4)
@70 // ASCII 'F'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Handle key 5 (G note)
(CHECK_KEY_5)
@71 // ASCII 'G'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Handle key 6 (A note)
(CHECK_KEY_6)
@65 // ASCII 'A'
D=A
@SEND_PLAY_SAMPLE_CMD
0;JMP

// Display the musical note and send command
(SEND_PLAY_SAMPLE_CMD)
// Store the note character for display
@current_note
M=D

// Send play sample command
@3840 // 0 0001 111 00000000
D=A
@KEYBOARD
D=D+M // Add the ASCII code of the pressed key
@UI_CMD_1
M=D

// Add small delay after displaying note
@200
D=A
@delay_loop_number
M=D
@MUSIC_NOTE_DISPLAY
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(MUSIC_NOTE_DISPLAY)
// Display the note character at X=0, Y=1
@current_note
D=M
// Add MSB flag to the character
@cmd_sync_flag
D=D+M
@UI_CMD_1
M=D
// Position at X=0, Y=1 (Y=1 means add 256 to X coordinate)
@256
D=A
// Add MSB flag to the position
@cmd_sync_flag
D=D+M
@UI_CMD_2
M=D

// Toggle the command sync flag for next command
@TOGGLE_MSB_RETURN_MUSIC
D=A
@toggle_msb_return
M=D
@TOGGLE_MSB_FLAG
0;JMP

(TOGGLE_MSB_RETURN_MUSIC)
@MAIN
0;JMP

(GAME)
// Increment game speed counter
@game_speed_counter
M=M+1

// Check if it's time to update the game
@game_speed_counter
D=M
@game_speed_divisor
D=D-M
@MAIN
D;JLT  // If counter < divisor, skip game update

// Reset counter and update game
@game_speed_counter
M=0

// Update bat position
@KEYBOARD
D=M
@87 // 'W'
D=D-A
@MOVE_BAT_UP
D;JEQ

@KEYBOARD
D=M
@83 // 'S'
D=D-A
@MOVE_BAT_DOWN
D;JEQ

@KEYBOARD
D=M
@84 // 'T'
D=D-A
@UPGRADE_BAT
D;JEQ

@UPDATE_BALL_POSITION
0;JMP

(MOVE_BAT_UP)
// Check if bat is at the top of the screen
@bat_y
D=M
@bat_half_height
D=D-M
@UPDATE_BALL_POSITION
D;JLT

@bat_y
D=M
@bat_speed
D=D-M
@bat_y
M=D
@UPDATE_BALL_POSITION
0;JMP

(MOVE_BAT_DOWN)
// Check if bat is at the bottom of the screen
@bat_y
D=M
@bat_half_height
D=D+M
@screen_height
D=D-M
@UPDATE_BALL_POSITION
D;JGT

@bat_y
D=M
@bat_speed
D=D+M
@bat_y
M=D
@UPDATE_BALL_POSITION
0;JMP

(UPGRADE_BAT)
@4
D=A
@bat_speed
M=D
@5
D=A
@boosted_ball_speed
M=D

@bat_upgraded
D=M
D=D-1
@UPDATE_BALL_POSITION
D;JEQ

@bat_upgraded
M=1
// Play game background music
@3076 // 0 0001 100 00000100
D=A
@UI_CMD_1
M=D
// Delay to play the sound
@100
D=A
@delay_loop_number
M=D
@PLAY_UPGRADE_BAT_SOUND
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(PLAY_UPGRADE_BAT_SOUND)
// Play upgrade bat sound
@3843 // 0 0001 111 00000011
D=A
@UI_CMD_1
M=D
// Delay to play the sound
@100
D=A
@delay_loop_number
M=D
@UPDATE_BALL_POSITION
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(UPDATE_BALL_POSITION)
// Update ball position
@ball_x
D=M
@ball_vel_x
D=D+M
@ball_x
M=D

@ball_y
D=M
@ball_vel_y
D=D+M
@ball_y
M=D

// Reset bat collided with ball flag
@bat_collided_with_ball
M=0

// COLLISION DETECTION
// Check left wall collision (ball_x - ball_half_width < 0)
@ball_x
D=M
@ball_half_width
D=D-M
@CHECK_RIGHT_WALL
D;JGE

// Left wall hit
@2
D=A
@boosted_ball_speed
D=D+M
@random_number
D=D+M
@ball_vel_x
M=D
@boosted_ball_speed
M=0
// Set ball_x to ball_half_width to prevent it from going off-screen
@ball_half_width
D=M
@collision_position_offset
D=D+M
@ball_x
M=D
// Play bounce sound
@CHECK_TOP_WALL
D=A
@play_bounce_sound_return
M=D
@PLAY_BOUNCE_SOUND
0;JMP

(CHECK_RIGHT_WALL)
// Check right wall collision (ball_x + ball_half_width > screen_width)
@ball_x
D=M
@ball_half_width
D=D+M
@screen_width
D=D-M
@CHECK_TOP_WALL
D;JLE

// Right wall hit
@2
D=A
@boosted_ball_speed
D=D+M
@random_number
D=D+M
@0
D=A-D
@ball_vel_x
M=D
// Set ball_x to screen_width - ball_half_width to prevent it from going off-screen
@screen_width
D=M
@ball_half_width
D=D-M
@collision_position_offset
D=D-M
@ball_x
M=D
// Play bounce sound
@CHECK_TOP_WALL
D=A
@play_bounce_sound_return
M=D
@PLAY_BOUNCE_SOUND
0;JMP

(CHECK_TOP_WALL)
// Check top wall collision (ball_y - ball_half_height < 0)
@ball_y
D=M
@ball_half_height
D=D-M
@CHECK_BOTTOM_WALL
D;JGE

// Top wall hit
@2
D=A
@boosted_ball_speed
D=D+M
@random_number
D=D+M
@ball_vel_y
M=D
// Set ball_y to ball_half_height to prevent it from going off-screen
@ball_half_height
D=M
@collision_position_offset
D=D+M
@ball_y
M=D
// Play bounce sound
@CHECK_BAT_COLLISION
D=A
@play_bounce_sound_return
M=D
@PLAY_BOUNCE_SOUND
0;JMP

(CHECK_BOTTOM_WALL)
// Check bottom wall collision (ball_y + ball_half_height > screen_height)
@ball_y
D=M
@ball_half_height
D=D+M
@screen_height
D=D-M
@CHECK_BAT_COLLISION
D;JLE

// Bottom wall hit
@2
D=A
@boosted_ball_speed
D=D+M
@random_number
D=D+M
@0
D=A-D
@ball_vel_y
M=D
// Set ball_y to screen_height - ball_half_height to prevent it from going off-screen
@screen_height
D=M
@ball_half_height
D=D-M
@collision_position_offset
D=D-M
@ball_y
M=D
// Play bounce sound
@CHECK_BAT_COLLISION
D=A
@play_bounce_sound_return
M=D
@PLAY_BOUNCE_SOUND
0;JMP

(CHECK_BAT_COLLISION)
// Check bat collision
// First check if ball center is in the same X range as bat center
@ball_x
D=M
@bat_x
D=D-M
@bat_half_width
D=D-M
@ball_half_width
D=D-M
@RENDER_GAME
D;JGT

// Ball is in bat's X range, now check Y collision
@ball_y
D=M
@bat_y
D=D-M
// Ball's Y position is the same as bat's Y position
@BAT_COLLISION
D;JEQ
// Ball's Y is less than bat's Y
@BALL_ABOVE_BAT
D;JLT

// Ball's Y is greater than bat's Y
// Ball is below bat
@ball_y
D=M
@bat_y
D=D-M
@bat_half_height
D=D-M
@ball_half_height
D=D-M
@RENDER_GAME
D;JGE
@BAT_COLLISION
0;JMP

(BALL_ABOVE_BAT)
@bat_y
D=M
@ball_y
D=D-M
@bat_half_height
D=D-M
@ball_half_height
D=D-M
@RENDER_GAME
D;JGT

(BAT_COLLISION)
// BAT COLLISION DETECTED!

// Play bat collision sound
@bat_upgraded
D=M-1
@PLAY_UPGRADED_BAT_COLLISION_SOUND
D;JEQ
@3840 // 0 0001 111 00000000
D=A
@UI_CMD_1
M=D
// Delay to play the sound
@100
D=A
@delay_loop_number
M=D
@HANDLE_BAT_COLLISION
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(PLAY_UPGRADED_BAT_COLLISION_SOUND)
@3841 // 0 0001 111 00000001
D=A
@UI_CMD_1
M=D
// Delay to play the sound
@100
D=A
@delay_loop_number
M=D
@HANDLE_BAT_COLLISION
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(HANDLE_BAT_COLLISION)
@bat_collided_with_ball
M=1
// Bounce the ball
@2
D=A
@boosted_ball_speed
D=D+M
@random_number
D=D+M
@ball_vel_x
M=D

@bat_x
D=M
@bat_half_width
D=D+M
@ball_half_width
D=D+M
@collision_position_offset
D=D+M
@ball_x
M=D

(RENDER_GAME)

// Render the game after updating positions and collision detection
// First word: bits 10..7 contain sprite ID (0-15), bits 6..3 contain variant (0-15)
// Second word: position for first sprite (210 * 128 coordinate space)
// Third word: bits 10..7 contain sprite ID (0-15), bits 6..3 contain variant (0-15)
// Fourth word: position for second sprite (210 * 128 coordinate space)
// One command moves 2 sprites simultaneously

// The ball sprite
@8192
D=A
// Sprite id 1 variant 0
@128
D=D+A
@cmd_sync_flag
D=D+M
@game_update_command_1
M=D
// Position for first sprite (210 * 128 coordinate space)
@ball_y
D=M
// shift to the left by 8 bits
@temp
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
@ball_x
D=D+M
@cmd_sync_flag
D=D+M
@game_update_command_2
M=D

// The bat sprite
@bat_upgraded
D=M
D=D-1
@RENDER_UPGRADED_BAT
D;JEQ

@bat_collided_with_ball
D=M
D=D-1
@RENDER_REGULAR_BAT_COLLISION
D;JEQ
@bat_variant
M=0
@SET_RENDER_BAT_COMMAND
0;JMP

(RENDER_REGULAR_BAT_COLLISION)
@8 // regular bat collision variant
D=A
@bat_variant
M=D
@SET_RENDER_BAT_COMMAND
0;JMP

(RENDER_UPGRADED_BAT)
@bat_collided_with_ball
D=M
D=D-1
@RENDER_UPGRADED_BAT_COLLISION
D;JEQ
@16 // upgraded bat variant
D=A
@bat_variant
M=D
@SET_RENDER_BAT_COMMAND
0;JMP

(RENDER_UPGRADED_BAT_COLLISION)
@24 // upgraded bat collision variant
D=A
@bat_variant
M=D

(SET_RENDER_BAT_COMMAND)
@bat_variant
D=M
@8192 // Sprite id 0 base
D=D+A
@cmd_sync_flag
D=D+M
@game_update_command_3
M=D

// Position for second sprite (210 * 128 coordinate space)
@bat_y
D=M
// shift to the left by 8 bits
@temp
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
M=D
D=D+M
@bat_x
D=D+M
@cmd_sync_flag
D=D+M
@game_update_command_4
M=D

// Send the commands
@game_update_command_1
D=M
@UI_CMD_1
M=D
@game_update_command_2
D=M
@UI_CMD_2
M=D
@game_update_command_3
D=M
@UI_CMD_3
M=D
@game_update_command_4
D=M
@UI_CMD_4
M=D

@MAIN
0;JMP

(DELAY)
@delay_counter
M=0
(DELAY_LOOP)
@delay_counter
D=M
@delay_loop_number // delay 0.02ms per loop at 500kHz
D=D-M
@delay_return_address
A=M
D;JGT
@delay_counter
M=M+1
@DELAY_LOOP
0;JMP

(PRINT_STRING)
// Load the current character address
@print_string_address
A=M
D=M
// Add MSB flag to the character
@cmd_sync_flag
D=D+M
@print_string_cmd_1
M=D
@print_string_index
D=M
@cmd_sync_flag
D=D+M
@print_string_cmd_2
M=D
// Send the commands
@print_string_cmd_1
D=M
@UI_CMD_1
M=D
@print_string_cmd_2
D=M
@UI_CMD_2
M=D

// Toggle the command sync flag for next command
@TOGGLE_MSB_RETURN_PRINT_STRING
D=A
@toggle_msb_return
M=D
@TOGGLE_MSB_FLAG
0;JMP

(TOGGLE_MSB_RETURN_PRINT_STRING)

// Add delay after sending command
@200
D=A
@delay_loop_number
M=D
@PRINT_STRING_CONTINUE
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(PRINT_STRING_CONTINUE)
// Increment the string address for next character
@print_string_address
M=M+1
// Increment the x-coordinate for next character position
@print_string_index
M=M+1
// Decrement the remaining length
@print_string_length
M=M-1
D=M
// If length > 0, continue printing
@PRINT_STRING
D;JGT
// If length = 0, we're done - return
@print_string_index
M=0
@print_string_return_address
A=M
0;JMP

(CLEAR_SCREEN)
// Send clear screen command
// Single word command does not need MSB flag
@4096
D=A
@UI_CMD_1
M=D
// Add small delay after clear screen
@100
D=A
@delay_loop_number
M=D
@CLEAR_SCREEN_DELAY
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(CLEAR_SCREEN_DELAY)
// Return to caller
@clear_screen_return_address
A=M
0;JMP

// Helper function to toggle the MSB flag
(TOGGLE_MSB_FLAG)
@cmd_sync_flag
D=M
@0
D=D-A
@SET_MSB_32768
D;JEQ
// If flag was 32768, set it to 0
@cmd_sync_flag
M=0
@TOGGLE_MSB_DONE
0;JMP
(SET_MSB_32768)
// If flag was 0, set it to 32768
@32767
D=A
D=D+1
@cmd_sync_flag
M=D
(TOGGLE_MSB_DONE)
// Return to caller
@toggle_msb_return
A=M
0;JMP

(PLAY_BOUNCE_SOUND)
@3842 // 0 0001 111 00000010
D=A
@UI_CMD_1
M=D
// Delay to play the sound
@100
D=A
@delay_loop_number
M=D
@play_bounce_sound_return
D=M
@delay_return_address
M=D
@DELAY
0;JMP
