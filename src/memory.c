#include <stdlib.h>
#include <assert.h>
#include "errors.h"
#include "memory.h"
#include "numberlist.h"

/** Value of uninitialized memory */
#define UNINIT 0xCCCCCCCC

static Memory * findMemCell(Memory *l, unsigned int address);
static Error addMemCell(Memory **l, unsigned int address, MemCell data);

/** WARNING: When using trace, make sure you are only working on ONE linked
 *  memory list! If you have multiple lists, you cannot write to them at the
 *  same time, because the tracer makes no difference between the two lists */
static int shouldTrace = 0;		// Should trace all access?
static int addrWasWritten = 0;		// Was there a memory change?
static int lastWrittenAddr = 0;		// If so, this was the address that changed!
static int ignoreNextTrace = 0;		// Used so stack access can be ignored
static NumberList *memtrace = NULL;	// Complete memory trace

/** Read the memory.
 *
 * @param [in] l	Memory
 * @param [in] address	Address of the memory cell
 * @return MemCell struct representing the cell. "Never" fails.
 */
MemCell readMemCell(Memory ** l, unsigned int address)
{
	Memory *addrloc = NULL;

	assert(l != NULL);

	// Return the MemCell if the address is already in the linked list.
	// If not yet in the list it is 'uninitialized' memory!

	addrloc = findMemCell(*l, address);
	if(addrloc == NULL)
	{
		MemCell emptyMemCell = {UNINIT};

		return emptyMemCell;
	}
	else
	{
		return addrloc->cell;
	}
}

/** Write to the memory
 *
 * @param [in,out] l		Memory
 * @param [in] address		Address of where the save the memory cell
 * @param [in] data		Data to be safed
 * @retval ERR_OutOfMemory	Malloc Failed
 */
Error writeMemCell(Memory ** l, unsigned int address, MemCell data)
{
	Memory	*mem = NULL;
	Error	rval = ERR_None;

	assert(l != NULL);

	// If the memory cell is not yet in the linked list, add it to the list.
	// If already in the list, update the value.

	mem = findMemCell(*l, address);
	if(mem == NULL)
	{
		rval = addMemCell(l, address, data);
		if(rval != ERR_None)
		{
			return rval;
		}
	}
	else
	{
		mem->cell = data;
	}

	// Always save the last address written.
	// Only save the trace list if tracing is on.
	if(!ignoreNextTrace)
	{
		lastWrittenAddr = address;
		addrWasWritten = 1;
		if(shouldTrace)
		{
			return addNumber(&memtrace, address);
		}
	}
	else
	{
		ignoreNextTrace = 0;
	}

	return ERR_None;
}

/** Free the complete memory list */
void freeMemList(Memory *l)
{
	Memory *toDel = l;

	while(l != NULL)
	{
		toDel = l;
		l = l->next;
		free(toDel);
	}
}

/** TRUE if a memory address was changed.
 *  See getLastWrittenAddr to get the address. */
int wasAddrWritten(void)
{
	return addrWasWritten;
}

/** Get the last address that was modified.
 *  Resets wasAddrWritten! */
int getLastWrittenAddr(void)
{
	addrWasWritten = 0;
	return lastWrittenAddr;
}

/** Enable memory access trace (only writes are saved) */
void enableTrace(void)
{
	shouldTrace = 1;
}

/** Disable memory access trace */
void disableTrace(void)
{
	shouldTrace = 0;
	freeNumberList(&memtrace);
}

void ignoreNextWriteInTrace(void)
{
	ignoreNextTrace = 1;
}

/** Get the memory trace
 *
 * @return Linked list of numbers representing the changed memory addresses.
 */
NumberList ** getTrace(void)
{
	return &memtrace;
}

/** Private function: find a memory cell in the linked list
 *
 * @return Returns NULL if address is not yet in the list. Else is returns
 *  the node including the memory cell.
 */
static Memory * findMemCell(Memory *l, unsigned int address)
{
	// Find the address. This is a sorted list! Return NULL if address not found.

	while(l != NULL && l->address < address)
	{
		l = l->next;
	}

	if(l != NULL && l->address == address)
	{
		return l;
	}
	else
	{
		return NULL;
	}
}

/** Private function: add a memory cell to the linked list.
 *  Does nothing if a memory cell with this address already exists.
 *
 * @retval ERR_OutOfMemory	Malloc failed
 */
static Error addMemCell(Memory **l, unsigned int address, MemCell data)
{
	Memory	*toAdd = NULL;

	assert(l != NULL);

	if(*l == NULL || (*l)->address > address)
	{	// Add new MemCell to the beginning of the list
		toAdd  = (Memory*) malloc(sizeof(Memory));
		if(toAdd == NULL)
		{
			return ERR_OutOfMemory;
		}
		toAdd->address = address;
		toAdd->cell = data;
		toAdd->next = *l;
		*l = toAdd;
	}
	else
	{	// Add new MemCell in the middle or at the end of the list
		Memory	*p = *l,
				*q = NULL;

		// Find where to add it. This is a sorted list!
		while(p != NULL && p->address < address)
		{
			q = p;
			p = p->next;
		}

		// Add MemCell if the addres is not already in use.
		if(p == NULL || p->address != address)
		{
			toAdd  = (Memory*) malloc(sizeof(Memory));
			if(toAdd == NULL)
			{
				return ERR_OutOfMemory;
			}
			toAdd->address = address;
			toAdd->cell = data;
			toAdd->next = p;
			q->next = toAdd;
		}
	}

	return ERR_None;
}
