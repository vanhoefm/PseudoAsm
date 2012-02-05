#ifndef _PSEUDOASM_INC_COMPILE_H_
#define _PSEUDOASM_INC_COMPILE_H_

#include "hardware.h"
#include "errors.h"

/* Convert an instruction to its binary representation */
Error parseAsmInstr(char *instr, MemCell *cell);

/* Convert the binary representation of an instruction to its 'string' form */
Error instToStr(Instruction instr, char *string);

#endif // _PSEUDOASM_INC_COMPILE_H_
