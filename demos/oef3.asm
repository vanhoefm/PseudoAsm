NOP
LDA #1000000
LDB #1000000
MUL
JOF 6
HLT
LDA #1		; We should print 1 !
OUT
HLT