@0      // sum = 0
M=0

@1      // counter = 1  
M=1

(LOOP)
@1      // Load counter
D=M
@5      // Compare with 5
D=D-A
@END    // If counter > 5, jump to end
D;JGT

@1      // Load counter
D=M
@0      // Add to sum
M=D+M

@1      // counter = counter + 1
M=M+1

@LOOP   // Jump back to loop
0;JMP

(END)
@END    // Infinite loop at end
0;JMP