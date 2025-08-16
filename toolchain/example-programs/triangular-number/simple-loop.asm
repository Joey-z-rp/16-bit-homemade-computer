@0      // sum = 0
M=0
@1      // counter = 1  
M=1
(LOOP)
@1      // Load counter
D=M
@200     // Compare with 200
D=D-A
@BEFORE_END    // If counter > 200, jump to before end
D;JGT
@1      // Load counter
D=M
@0      // Add to sum
M=D+M
@1      // counter = counter + 1
M=M+1
@LOOP   // Jump back to loop
0;JMP
(BEFORE_END)
@0      // Load sum
D=M
@END 
M=D    // Store sum in END so we can see the result in RAM output
(END)
@END    // Infinite loop at end
0;JMP
// Result: 20100 - 0100111010000100
