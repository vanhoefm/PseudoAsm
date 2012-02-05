INP
STA 100
LDB 100
INP
JSB 10
OUT
HLT
NOP
NOP
NOP
STA 100
STB 101
LDA 101
JIZ 46			; GOTO _return_a_
JSP 32			; GOTO _int_add(a++, n--)_
NOP
NOP
LDA 100			; _int_add(a--, n++)_:
LDB 101
STA 100			
LDB #1
SUB
STA 100
LDA 101
ADD
STA 101
LDA 100
LDB 101
JSB 10			; recursive call
JMP	46			; GOTO _return_a_
NOP
NOP
LDA 100			; _int_add(a++, n--)_:
LDB 101
STA 100			
LDB #1
ADD
STA 100
LDA 101
SUB
STA 101
LDA 100
LDB 101
JSB 10			; recursive call
NOP
NOP
LDA 100			; _return_a_:
RTS				
