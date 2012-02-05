NOP
INP		; Loop until strict positive number
JSP 5
JMP 1
NOP
JSB 13		; Call Subroutine:  
HLT
NOP
STA 100		; Write 10d to address 100
LDB 100		; Load 10d to B
STB (100)	; Write 10d = NOP to address 10 = this address
JMP 16
NOP
LDA #10		; Subroutine
OUT		; Print 10
JMP 8
NOP		; We get back here
RTS
HLT

