// The number of loops to delay
@delay_loop_number
M=0
// The address to jump to after the delay, initialized to MAIN
@MAIN
D=A
@delay_return_address
M=D

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


(MAIN)

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
@83 // S
D=A
@UI_CMD_1
M=D
@WAIT_MUSIC_PROMPT_EXECUTED
0;JMP

// Music prompt
(MUSIC_PROMPT)
@77 // M
D=A
@UI_CMD_1
M=D

// delay for the command to be executed
(WAIT_MUSIC_PROMPT_EXECUTED)
@50
D=A
@delay_loop_number
M=D
@SET_MODE_PROMPT_PRINTED
D=A
@delay_return_address
M=D
@DELAY
0;JMP

(SET_MODE_PROMPT_PRINTED)
@is_mode_prompt_printed
M=1

(KEYBOARD_CHECK)
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
// TODO: Implement the select action logic
@MAIN
0;JMP

(MUSIC)
// TODO: Implement the music logic
@MAIN
0;JMP