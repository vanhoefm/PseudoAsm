#ifndef _PSEUDOASM_INC_PROCESSOR_H_
#define _PSEUDOASM_INC_PROCESSOR_H_

#include "memory.h"
#include "error.h"

typedef struct ProcInfo
{
	int regA;
	int regB;
	int flagZ;
	int flagO;
	int flagN;
	int progCounter;
} ProcInfo;

/* Initialise processor */
Error InitProcessor(Memory *meminit, FuncNumInp inp, FuncNumOut out);

/* Deinitialise processor */
void DeInitProcessor(void);

/* Get current registers, etc */
ProcInfo getStatus(void);

/* Execute an instruction. If the Program Counter should not be updated,
 * set saveProgCount to 1. For normal behaviour, set it to 0. */
Error executeInstr(Instruction instr, int saveProgCount);

/* Execute the next instruction */
Error executeNextInstr(void);

/* Get the next instruction that will be executed */
Instruction getNextInstr(void);

/* Modify the registers, program counter, etc */
void setStatus(ProcInfo info);


/* Set a breakpoint */
Error setBreakpoint(unsigned int address);

/* Remove a breakpoint */
Error delBreakpoint(unsigned int address);

/* Get list of breakpoints */
NumberList *getBreakpoints(void);


/* Returns the address and value of a memory cell changed.
 * If no cell was changed since the last call of this function,
 * it returns ERR_InvalidState */
Error memoryChanged(unsigned int *address, MemCell *value);

/* Read a memory cell */
MemCell readMemory(unsigned int address);

/* Write to a memory cell */
Error writeMemory(unsigned int address, MemCell data);


/* Get the stack pointer */
int getStackPointer(void);

/* Set the stack pointer */
void setStackPointer(int pointer);

/* Should the stack be traced like normal memory? */
void traceStack(int shouldTrace);

#endif // _PSEUDOASM_INC_PROCESSOR_H_
