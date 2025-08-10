
// 100: total count
// 200: outer i
// 300: inner j

@0
D=A
@100
M=D         // total_count = 0

@1
D=A  
@200
M=D         // outer_i = 1

(OUTER_LOOP)
    @200
    D=M
    @100
    D=D-A
    @END_OUTER
    D;JGT       // Exit when outer_i > 25
    
    @1
    D=A
    @300
    M=D         // inner_j = 1
    
    (INNER_LOOP)
        @300
        D=M
        @90
        D=D-A
        @END_INNER
        D;JGT   // Exit when inner_j > 30
        
        @100
        M=M+1   // total_count++
        
        @300
        M=M+1   // inner_j++
        
        @INNER_LOOP
        0;JMP
    
    (END_INNER)
        @200
        M=M+1   // outer_i++
        
        @OUTER_LOOP
        0;JMP

(END_OUTER)
    // Store result at address 100
    @100
    D=M
    @END
    M=D

(END)
@END
0;JMP