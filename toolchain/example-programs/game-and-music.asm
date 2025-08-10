// The number of loops to delay
@delay_loop_number
M=0
// The address to jump to after the delay
@delay_return_address
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
// Send the character to UI_CMD_1
@UI_CMD_1
M=D
// Add delay after sending command
@100
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
// Decrement the remaining length
@print_string_length
M=M-1
D=M
// If length > 0, continue printing
@PRINT_STRING
D;JGT
// If length = 0, we're done - return
@print_string_return_address
A=M
0;JMP
