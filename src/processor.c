/* Note: If not initialized, it will try to execute uninitialized memory */
#include <stdlib.h>
#include <assert.h>
#include "numberlist.h"
#include "hardware.h"
#include "memory.h"
#include "errors.h"
#include "processor.h"

#define TRUE 1
#define FALSE 0

// Registers
static int regA;
static int regB;
static unsigned int progCounter;

// Stack pointer
static unsigned int stackPointer = 900000;
static int shouldTraceStack = 0;

// Flags
static int flagZ;
static int flagO;
static int flagN;

// Memory
static Memory *memory = NULL;

/** Input method used */
static FuncNumInp numberinp = NULL;

/** Output method used */
static FuncNumOut numberout = NULL;

// List of breakpoints
NumberList *breakpoints = NULL;

typedef Error (*funcHandleInstr)(Instruction inst);

typedef struct InstrInfo
{
	AsmInstr instr;
	funcHandleInstr handler;
} InstrInfo;

/*
 * Begin of private functions: Used to handle certain assembly instructions
 */

static Error instrNop(Instruction instr)
{
	assert(instr.operator == A_NOP);

	progCounter++;
	return ERR_None;
}

static Error instrLoad(Instruction instr)
{
	int value = 0;
	int pointer = 0;

	//
	// Step 1 - Get value that will be loaded into the register
	//
	switch(instr.adressering)
	{
	case ONMIDDELIJK:
		// Copy unsigned 24 bit value to signed 32 bit variable.
		// => No conversion will be done.
		value = instr.operand;

		// If H.O. bit of the 24 bit number was set, sign extend value
		if(instr.operand & 0x800000)
		{
			value |= 0xFF000000;
		}
		break;
	case DIRECT:
		value = readMemCell(&memory, instr.operand).getal;
		break;
	case INDIRECT:
		pointer = readMemCell(&memory, instr.operand).getal;
		value = readMemCell(&memory, pointer).getal;
		break;
	default:
		return ERR_InvalidInstr;
		break;
	}

	//
	// Step 2 - Load the value in the correct register 
	//
	switch(instr.operator)
	{
	case A_LDA:
		regA = value;
		flagN = regA < 0;
		flagZ = regA == 0;
		flagO = 0;
		break;
	case A_LDB:
		regB = value;
		break;
	default:
		return ERR_InvalidInstr;
		break;
	}

	progCounter++;
	return ERR_None;
}

/**
 * @retval ERR_OutOfMemory	Malloc failed
 */
static Error instrStore(Instruction instr)
{
	int		addr = 0;
	Error	rval = ERR_None;
	MemCell memCell;

	// Step 1 - get the address where to store the value
	switch(instr.adressering)
	{
	case DIRECT:
		addr = instr.operand;
		break;
	case INDIRECT:
		addr = readMemCell(&memory, instr.operand).getal;
		break;
	default:
		return ERR_InvalidInstr;
		break;
	}

	// Step 2 - Store the value
	switch(instr.operator)
	{
	case A_STA:
		memCell.getal = regA;
		break;
	case A_STB:
		memCell.getal = regB;
		break;
	default:
		return ERR_InvalidInstr;
		break;
	}

	// Store the value!
	rval = writeMemCell(&memory, addr, memCell);
	if(rval != ERR_None)
	{
		return rval;
	}

	progCounter++;
	return ERR_None;
}

/**
 * @retval ERR_DivideByZero		Attempt to divide by zero
 */
static Error instrMath(Instruction instr)
{
	double	dvalA = regA,
			dvalB = regB;

	switch(instr.operator)
	{
	case A_ADD:
		regA += regB;
		dvalA += dvalB;
		break;
	case A_SUB:
		regA -= regB;
		dvalA -= dvalB;
		break;
	case A_MUL:
		regA *= regB;
		dvalA *= dvalB;
		break;
	case A_DIV:
		if(regB == 0)
		{
			return ERR_DivideZero;
		}
		regA /= regB;
		dvalA /= dvalB;
		break;
	default:
		assert(0); // Mistake in parse table
		return ERR_InvalidInstr;
		break;
	}

	flagN = regA < 0;
	flagZ = regA == 0;
	flagO = dvalA != (double)regA;

	progCounter++;
	return ERR_None;
}

static Error instrInput(Instruction instr)
{
	assert(instr.operator == A_INP);
	assert(numberinp != NULL);

	regA = numberinp();

	flagN = regA < 0;
	flagZ = regA == 0;
	flagO = 0;

	progCounter++;
	return ERR_None;
}

static Error instrOutput(Instruction instr)
{
	assert(instr.operator == A_OUT);
	assert(numberout != NULL);

	numberout(regA);

	progCounter++;
	return ERR_None;
}

static Error instrJump(Instruction instr)
{
	int shouldJump = 0;
	
	switch(instr.operator)
	{
	case A_JMP:
		shouldJump = 1;
		break;
	case A_JSP:
		shouldJump = !flagZ && !flagN;
		break;
	case A_JSN:
		shouldJump = flagN;
		break;
	case A_JIZ:
		shouldJump = flagZ;
		break;
	case A_JOF:
		shouldJump = flagO;
		break;
	default:
		assert(0); // Mistake in parse table
		return ERR_InvalidInstr;
		break;
	}

	if(shouldJump)
	{
		progCounter = instr.operand;
	}
	else
	{
		progCounter++;
	}

	return ERR_None;
}

static Error instrCall(Instruction instr)
{
	MemCell memCell;

	assert(instr.operator == A_JSB);

	// Push program counter on the stack
	if(!shouldTraceStack)
	{
		ignoreNextWriteInTrace();
	}
	memCell.getal = progCounter + 1;
	writeMemCell(&memory, --stackPointer, memCell);

	// Jump to subroutine
	progCounter = instr.operand;

	return ERR_None;
}

static Error instrReturn(Instruction instr)
{
	MemCell memCell;

	assert(instr.operator == A_RTS);

	// Pop old program counter
	memCell = readMemCell(&memory, stackPointer++);

	// Set program counter back
	progCounter = memCell.getal;

	return ERR_None;
}

/** @retval ERR_EndOfProgram	Halt instruction reached! */
static Error instrHalt(Instruction instr)
{
	assert(instr.operator == A_HLT);

	return ERR_EndOfProgram;
}

// static Error instrSetStack(Instruction instr)
// {
// 	assert(instr.operator == A_SST);
// 
// 	stackPointer = instr.operand;
// 
// 	progCounter++;
// 
// 	return ERR_None;
// }

/**
 * Instruction table: Connection between the assembly instruction and the
 * private function that handles this instruction
 */
static InstrInfo instrTable[] = 
{
	{A_NOP, instrNop},
	{A_LDA, instrLoad},
	{A_LDB, instrLoad},
	{A_STA, instrStore},
	{A_STB, instrStore},
	{A_INP, instrInput},
	{A_OUT, instrOutput},
	{A_ADD, instrMath},
	{A_SUB, instrMath},
	{A_MUL, instrMath},
	{A_DIV, instrMath},
	{A_JMP, instrJump},
	{A_JSP, instrJump},
	{A_JSN, instrJump},
	{A_JIZ, instrJump},
	{A_JOF, instrJump},
	{A_JSB, instrCall},
	{A_RTS, instrReturn},
	{A_HLT, instrHalt}//,
	//{A_SST, instrSetStack}
};

/** Public function: Initialize processor.
 *
 * @param meminit	Load programming and set memory
 * @param inp		Input method of instruction INP
 * @param out		Output method of instruction OUT
 * return "Never" fails and always returns ERR_None
 */
Error InitProcessor(Memory *meminit, FuncNumInp inp, FuncNumOut out)
{
	memory = meminit;
	numberinp = inp;
	numberout = out;

	breakpoints = NULL;

	// Reset processor registers and falgs
	regA = 0;
	regB = 0;
	progCounter = 0;

	stackPointer = 900000;
	shouldTraceStack = 0;

	flagZ = 0;
	flagO = 0;
	flagN = 0;	

	// Reset last written address (written by compiler)
	getLastWrittenAddr(); // this will reset it :)

	return ERR_None;
}

void DeInitProcessor(void)
{
	disableTrace();
	freeMemList(memory);
	memory = NULL;
	freeNumberList(&breakpoints);

	numberout = NULL;
	numberinp = NULL;
}

/**
 * Execute the given instruction.
 *
 * @param [in] instr			Instruction to execute
 * @param [in] saveProgCount		If true, the program counter will remain unchanged.
 *					Used for debugging purposes.
 * @retval ERR_UnknownInstr		Instruction was not found in the parse table
 * @retval ERR_OutOfMemory		Malloc failed
 * @retval ERR_DivideByZero		Attempt to divide by zero
 * @retval ERR_EndOfProgram		Halt instruction reached
 */
Error executeInstr(Instruction instr, int saveProgCount)
{
	int	i = 0,
		oldProgCounter = progCounter;
	Error	rval = ERR_None;

	for(i = 0; i < sizeof(instrTable) / sizeof(InstrInfo); i++)
	{
		if((unsigned int)instrTable[i].instr == instr.operator)
		{
			rval = instrTable[i].handler(instr);
			if(saveProgCount)
			{
				progCounter = oldProgCounter;
			}
			return rval;
		}
	}

	return ERR_UnknownInstr;
}

/** Let the processor execute the next insruction
 *
 * See executeInstr for return values.
 * If instruction successfully executed, it could return ERR_Breakpoint if there is
 * a breakpoint on the next instruction.
 */
Error executeNextInstr(void)
{
	Error rval = ERR_None;
	MemCell instr = readMemCell(&memory, progCounter);

	rval = executeInstr(instr.instructie, FALSE);
	if(rval == ERR_None && hasNumber(breakpoints, progCounter))
	{
		return ERR_Breakpoint;
	}
	else
	{
		return rval;
	}
}

/** Get the next instruction that will be executed */
Instruction getNextInstr(void)
{
	MemCell instr = readMemCell(&memory, progCounter);

	return instr.instructie;
}

/**
 * Get information about the processor:
 *  - Registers
 *  - Flags
 *  - Program counter
 */
ProcInfo getStatus(void)
{
	ProcInfo info;

	info.regA = regA;
	info.regB = regB;
	info.flagZ = flagZ;
	info.flagO = flagO;
	info.flagN = flagN;
	info.progCounter = progCounter;

	return info;
}

/**
 * Set information about the processor:
 *  - Registers
 *  - Flags
 *  - Program counter
 */
void setStatus(ProcInfo info)
{
	regA = info.regA;
	regB = info.regB;
	flagZ = info.flagZ;
	flagO = info.flagO;
	flagN = info.flagN;
	progCounter = info.progCounter;
}

MemCell readMemory(unsigned int address)
{
	return readMemCell(&memory, address);
}

Error writeMemory(unsigned int address, MemCell data)
{
	return writeMemCell(&memory, address, data);
}

/** Return the address and value of the last memory change after the previous
 *  call of this fucntion. So after calling this function, the 'last written address'
 *  is reset!
 *
 * Returns ERR_InvalidState if no memory cell was changed after the previous call
 * of the function.
 */
Error memoryChanged(unsigned int *address, MemCell *value)
{
	if(!wasAddrWritten())
	{
		return ERR_InvalidState;
	}
	else
	{
		*address = getLastWrittenAddr();
		*value = readMemCell(&memory, *address);
		return ERR_None;
	}
}

/* Set a breakpoint somewhere */
Error setBreakpoint(unsigned int address)
{
	return addNumber(&breakpoints, address);
}

/* Remove a breakpoint */
Error delBreakpoint(unsigned int address)
{
	return delNumber(&breakpoints, address);
}

NumberList *getBreakpoints(void)
{
	return breakpoints;
}

int getStackPointer(void)
{
	return stackPointer;
}

void setStackPointer(int pointer)
{
	stackPointer = pointer;
}

void traceStack(int shouldTrace)
{
	shouldTraceStack = shouldTrace;
}

