#ifndef _PSEUDOASM_INC_COMPILER_H_
#define _PSEUDOASM_INC_COMPILER_H_

#include <stdio.h>    // FILE typedef
#include "errors.h"   // Return value
#include "memory.h"   // Memory typedef
#include "hardware.h" // OutputFunc decleration

/** Reads a file and converts each assembler instruction to it's
 *  binary representation. 'Compiled' program is saved in the linked
 *  list of memory cell. 'output' is the function that will be used
 *  to print error messages */
Error compile(FILE *fp, Memory **memory, OutputFunc output);

#endif // _PSEUDOASM_INC_COMPILER_H_

