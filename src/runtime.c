#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "hardware.h"
#include "memory.h" // For trace functions
#include "runtime.h"
#include "compiler.h"
#include "processor.h"
#include "parser.h"

#define MAXOUTLEN 101
// Debug (console) output function.
OutputFunc consoleOut = NULL;

static void displayTrace(void);
static void displayError(Error rval);

#define FLAGTOCHAR(x) x == 1 ? 'X' : '_'

/** Initialize the runtime with the program
 *
 * @retval ERR_OpeningFile	Source file could not be opened
 * @retval ERR_ReadingFile	Error getting line from file
 * @retval ERR_OutOfMemory	Malloc failed
 */
Error rntInit(char filename[], FuncNumInp numInp, FuncNumOut numOut, OutputFunc output)
{
	Error	rval;
	FILE	*source = NULL;
	Memory	*mem = NULL;

	// Set debug (console) output function
	consoleOut = output;

	// Open file
	source = fopen(filename, "r");
	if(!source)
	{
		return ERR_OpeningFile;
	}

	// Compile file
	rval = compile(source, &mem, consoleOut);
	fclose(source);
	if(rval != ERR_None)
	{
		return rval;
	}

	// Initialize processer with the compiled 'memory'
	rval = InitProcessor(mem, numInp, numOut);
	if(rval != ERR_None)
	{
		return rval;
	}

	consoleOut("Runtime initialized!\n");
	rntDisplayStatus();

	return ERR_None;
}

void rntDeInit(void)
{
	DeInitProcessor();
	consoleOut = NULL;
}

/** Display registers, flags and the next instruction in the console */
void rntDisplayStatus(void)
{
	Error		rval = ERR_None;
	ProcInfo	info;
	char		buff[MAXOUTLEN];
	char		nextInstr[21];

	rval = instToStr(getNextInstr(), nextInstr);
	if(rval != ERR_None)
	{
		strcpy(nextInstr, "???");
	}

	info = getStatus();
	sprintf(buff,
		"  Registers: A: %-10d B: %-10d PC: %d\n"
		"  Flags:     Z: %c   O: %c   N: %c\n"
		"  => %s\n",
		info.regA, info.regB, info.progCounter,
		FLAGTOCHAR(info.flagZ), FLAGTOCHAR(info.flagO), FLAGTOCHAR(info.flagN),
		nextInstr);

	consoleOut(buff);
}

/** Step the next instruction
 *
 * See executeNextInstr for possible error codes
 */
Error rntStep(void)
{
	Error		rval = ERR_None;
	MemCell		value;
	unsigned int	address;
	char		buff[101];

	rval = executeNextInstr();
	if(rval != ERR_None && rval != ERR_Breakpoint)
	{
		displayError(rval);
		return rval;
	}

	if(memoryChanged(&address, &value) != ERR_InvalidState)
	{
		sprintf(buff, "  [Memory] %010u:\t%d\n", address, value.getal);
		consoleOut(buff);
	}

	rntDisplayStatus();

	return ERR_None;
}

Error rntRun(void)
{
	Error		rval = ERR_None;

	enableTrace();

	do
	{
		rval = executeNextInstr();
	} while(rval == ERR_None);

	// The only thing that can interrupt a running program is a breakpoint.
	// Or the "error" ERR_EndOfProgram. Other values are REAL errors.
	if(rval != ERR_Breakpoint)
	{
		displayError(rval);
	}
	else
	{
		consoleOut("==> A breakpoint has been hit!\n");
	}

	displayTrace();
	disableTrace();

	rntDisplayStatus();

	return rval; // forward return value
}

static void displayError(Error rval)
{
	char		buff[MAXOUTLEN];
	ProcInfo	info;

	switch(rval)
	{
	case ERR_DivideZero:
		info = getStatus();
		sprintf(buff, "Error: Division by zero at address %d\n", info.progCounter);
		consoleOut(buff);
		break;
	case ERR_UnknownInstr:
		info = getStatus();
		sprintf(buff, "Unknown instruction at address %d\n", info.progCounter);
		consoleOut(buff);
		break;
	case ERR_EndOfProgram:
		consoleOut("==> Program successfully executed.\n");
		break;
	case ERR_None:
		assert(0); // Why are you displaying an error when there is none ???
		break;
	case ERR_OutOfMemory:
		consoleOut("CRITICAL: PseudoAsm out of memory!\n");
		break;
	default:
		consoleOut("Unknown error\n");
		break;
	}
}

static void displayTrace(void)
{
	NumberList	**trace = NULL;
	int			address;
	MemCell		data;
	char		buff[201];

	trace = getTrace();
	while(popNumber(trace, &address) == ERR_None)
	{
		data = readMemory(address);
		sprintf(buff, "  [Memory] %010u:\t%d\n", address, data.getal);
		consoleOut(buff);
	}
}

Error rntFlyExec(char *cmd)
{
	MemCell		memcell,
			value;
	Error		rval;
	unsigned int	address;
	char		buff[101];

	rval = parseAsmInstr(cmd, &memcell);
	if(rval != ERR_None)
	{
		consoleOut("Error parsing asm instruction\n");
		return rval;
	}

	rval = executeInstr(memcell.instructie, 1);
	if(rval != ERR_None)
	{
		consoleOut("Error executing asm instruction\n");
	}

	if(memoryChanged(&address, &value) != ERR_InvalidState)
	{
		sprintf(buff, "  [Memory] %010u:\t%d\n", address, value.getal);
		consoleOut(buff);
	}
	rntDisplayStatus();

	return ERR_None;
}

Error rntFlyAsm(unsigned int address, char *cmd)
{
	MemCell		memcell,
			value;
	Error		rval;
	char		buff[111];

	rval = parseAsmInstr(cmd, &memcell);
	if(rval != ERR_None)
	{
		consoleOut("Error parsing asm instruction\n");
		return rval;
	}

	rval = writeMemory(address, memcell);
	if(rval != ERR_None)
	{
		consoleOut("Error writing to memory\n");
		return rval;
	}
	else
	{
		ProcInfo info;

		if(memoryChanged(&address, &value) != ERR_InvalidState)
		{
			sprintf(buff, "  [Memory] %010u:\t%d (%s)\n", address, value.getal, cmd);
			consoleOut(buff);
		}

		// Next instruction could have changed
		info = getStatus();
		if(info.progCounter == address)
		{
			rntDisplayStatus();
		}
	}

	return ERR_None;
}

//
// BREAKPOINTS
//

void rntSetBp(int address)
{
	char buff[56];

	switch(setBreakpoint(address))
	{
	case ERR_OutOfMemory:
		consoleOut("CRITICAL: PseudoAsm out of memory!\n");
		break;
	case ERR_None:
		sprintf(buff, "Breakpoint set at address %d\n", address);
		consoleOut(buff);
		break;
	default:
		//assert(0);
		consoleOut("Unknown error\n");
		break;
	}
}

void rntListBp(void)
{
	NumberList *l = getBreakpoints();

	if(l == NULL)
	{
		consoleOut("No breakpoints have been set\n");
	}	
	else
	{
		char buff[51];

		consoleOut("Breakpoints:\n");
		while(l != NULL)
		{
			sprintf(buff, "  Address %d\n", l->number);
			consoleOut(buff);
			l = l->next;
		}
	}
}

void rntDelBp(int address)
{	
	switch(delBreakpoint(address))
	{
	case ERR_NotFound:
		printf("There was no breakpoint set at %d!\n", address);
		break;
	case ERR_None:
		printf("Breakpoint at address %d removed\n", address);
		break;
	default:
		//assert(0);
		printf("Unknown error\n");
		break;
	}
}

void rntSetStack(int pointer)
{
	setStackPointer(pointer);
}

int rntGetStack(void)
{
	return getStackPointer();
}

void rntStackTrace(int shouldTrace)
{
	traceStack(shouldTrace);
}

