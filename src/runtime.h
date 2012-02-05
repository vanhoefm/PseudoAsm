#ifndef _PSEUDOASM_INC_RUNTIME_H_
#define _PSEUDOASM_INC_RUNTIME_H_

#include "errors.h"
#include "hardware.h"

/* Initialise the runtime */
Error rntInit(char filename[], FuncNumInp numInp, FuncNumOut numOut, OutputFunc output);

/* Deinit */
void rntDeInit(void);

/* Execute the next instruction */
Error rntStep(void);

/* Run the program untill HLT or a breakpoint */
Error rntRun(void);

/* Execute an instruction without changing the program counter */
Error rntFlyExec(char *cmd);

/* Display status of the runtime */
void rntDisplayStatus(void);


/* Set a breakpoint */
void rntSetBp(int address);

/* List current breakpoints */
void rntListBp(void);

/* Delete a breakpoint */
void rntDelBp(int address);

/* Parse an instruction and save it to memory */
Error rntFlyAsm(unsigned int address, char *cmd);


/* Set the stack pointer */
void rntSetStack(int pointer);

/* Get the stack pointer */
int rntGetStack(void);

/* Should changes to the stack be traced? */
void rntStackTrace(int shouldTrace);

#endif // _PSEUDOASM_INC_UTIL_H_
