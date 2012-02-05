#ifndef _PSEUDOASM_INC_MEMORY_H_
#define _PSEUDOASM_INC_MEMORY_H_

#include "hardware.h"
#include "errors.h"
#include "numberlist.h"

typedef struct Memory
{
	unsigned int address;
	MemCell cell;
	struct Memory *next;
} Memory;

/* Read data from an address */
MemCell readMemCell(Memory ** l, unsigned int address);

/* Write data to an address */
Error writeMemCell(Memory ** l, unsigned int address, MemCell data);

/* Free the memory list */
void freeMemList(Memory *l);

/* Get the last address that was written to.
   Resets "address was written", see wasAddrWritten. */
int getLastWrittenAddr(void);

/* Did we write to an address. */
int wasAddrWritten(void);

/* Saves a list of all the address where we wrote some data to */
void enableTrace(void);

/* Disable trace */
void disableTrace(void);

/* Get the current trace */
NumberList ** getTrace(void);

/* Do NOT trace the next call to writeMemCell */
void ignoreNextWriteInTrace(void);

#endif // _PSEUDOASM_INC_MEMORY_H_
