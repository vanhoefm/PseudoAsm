NOP
LDA #109   
STA 100    ; (100) = 109
LDA #33    
STA 103    ; (103) = 103
LDA #106   
STA 104    ; (104) = 106
LDA #6     
STA 106    ; (106) = 6
LDA #112   
STA 107    ; (107) = 112
LDA #9     
STA 109    ; (109) = 9
LDA #103   
STA 110    ; (110) = 103
LDA #12    
STA 112    ; (112) = 12
LDA #0     
STA 113    ; (113) = 0
NOP        
LDA 100    ; A = 109
LDB #1     ; B = 1
SUB        ; A = 108
JSN 60     ; NO JUPM (NO print 97 en 98)
LDA (100)  ; LDA 109 -> A = 9
STA 97     ; (97) = 9
STA 98     ; (98) = 9
LDA 100    ; A = 109
ADD        ; A = 110
STA 99     ; (99) = 110
LDA (99)   ; LDA 110 -> A = 103
STA 99     ; (99) = 110
NOP        
NOP        
NOP        
LDA 99     ; A = 110
LDB #1     ; B = 1
SUB        ; A = 109
JSN 60     ; NO JMP
LDA (99)   ; A = 110
NOP        
LDA 97     ; A = 109
LDB (99)   ; B = 1
SUB        ; A = 108
JSN 46     ; NO JMP
STB 97     ; (97) = 1
LDA 98     
SUB        
JSP 50     
STB 98     
LDA 99     
LDB #1     
ADD        
STA 99     
LDA (99)   
STA 99     
JMP 35     
NOP        
NOP        
NOP        
LDA 97     
OUT        
LDA 98     
OUT        
HLT        

