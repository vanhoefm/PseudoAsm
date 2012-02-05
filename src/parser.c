#include <stdio.h>
#include <string.h>
#include "util.h"
#include "parser.h"
#include "errors.h"

/** Internally used: in parsing table */
typedef enum AddrFlags
{
	FLAG_Onmiddelijk	= 0x01,
	FLAG_Direct		= 0x02,
	FLAG_Indirect		= 0x04,
	FLAG_Geindexeerd	= 0x08,
	FLAG_NoArgs		= 0x10
} AddrFlags;

static Error parseInstr(char *instr, AsmInstr asmInstr, AddrFlags addrFlags, MemCell *cell);

/** Used to create a parse table */
typedef struct ParseInfo
{
	/** String: Asm instrunction */
	char		instruction[4];
	/** Allowed arguments of the instruction */
	AddrFlags	addrFlags;
	/** Opcode of the operator */
	AsmInstr	asmInstr;
} ParseInfo;

/** Link between the assembly instruction, allowed arguments of the instruction
	and the opcode for the operator */
ParseInfo parseTable[] =
{
	{"lda", FLAG_Onmiddelijk | FLAG_Direct | FLAG_Indirect, A_LDA},
	{"ldb", FLAG_Onmiddelijk | FLAG_Direct | FLAG_Indirect, A_LDB},
	{"sta", FLAG_Direct | FLAG_Indirect, A_STA},
	{"stb", FLAG_Direct | FLAG_Indirect, A_STB},
	{"add", FLAG_NoArgs, A_ADD},
	{"sub", FLAG_NoArgs, A_SUB},
	{"mul", FLAG_NoArgs, A_MUL},
	{"div", FLAG_NoArgs, A_DIV},
	{"rts", FLAG_NoArgs, A_RTS},
	{"nop", FLAG_NoArgs, A_NOP},
	{"inp", FLAG_NoArgs, A_INP},
	{"out", FLAG_NoArgs, A_OUT},
	{"hlt", FLAG_NoArgs, A_HLT},
	{"jsb", FLAG_Direct, A_JSB},
	{"jmp", FLAG_Direct, A_JMP},
	{"jsp", FLAG_Direct, A_JSP},
	{"jsn", FLAG_Direct, A_JSN},
	{"jiz", FLAG_Direct, A_JIZ},
	{"jof", FLAG_Direct, A_JOF}//,
	//{"sst", FLAG_Onmiddelijk, A_SST}
};

/** Parses an instruction (string) to it's binary representation (MemCell).
 *  This function is internal and is designed to be used with the parse table.
 *
 * @retval ERR_InvalidInstr		Invalid instruction detected
 */
static Error parseInstr(char *instr, AsmInstr asmInstr, AddrFlags addrFlags, MemCell *cell)
{
	char end[2];
	int arg;

	cell->instructie.operator = asmInstr;	

	// Note 0: The line should include NO COMMENTS. Remove them before calling
	// this function.
	// Note 1: the return value of sscanf, "%1s" and the end[2] array are used to
	// check that the instruction ends there and does not include any garbage
	// after the actual instruction.
	// Note 2: "%*3s" is used because ALL the instruction have length 3 :)
	// Note 3: strchr(instr, '-') == NULL is used to check that the number
	// is posotive.

	if((addrFlags & FLAG_Onmiddelijk) == FLAG_Onmiddelijk
		&& sscanf(instr, "%*3s #%d %1s", &arg, end) == 1)
	{
		cell->instructie.adressering = ONMIDDELIJK;
		cell->instructie.operand = arg;
	}
	else if((addrFlags & FLAG_Direct) == FLAG_Direct
		&& sscanf(instr, "%*3s %d %1s", &arg, end) == 1
		&& strchr(instr, '-') == NULL)
	{
		cell->instructie.adressering = DIRECT;
		cell->instructie.operand = arg;
	}
	else if((addrFlags & FLAG_Indirect) == FLAG_Indirect
		&& sscanf(instr, "%*3s (%d) %1s", &arg, end) == 1
		&& strchr(instr, '-') == NULL)
	{
		cell->instructie.adressering = INDIRECT;
		cell->instructie.operand = arg;
	}
	else if(addrFlags == FLAG_NoArgs
		&& sscanf(instr, "%*3s %1s", end) == EOF)
	{
		cell->instructie.adressering = 0;
		cell->instructie.operand = /*(unsigned int)*/-1;
	}
	else
	{
		return ERR_InvalidInstr;
	}

	return ERR_None;
}

/** Parse an assembly instruction.
 *
 * @retval ERR_InvalidInstr	Invalid instruction (incorrect arguments?)
 * @retval ERR_UnknownInstr	Unknown instruction
 */
Error parseAsmInstr(char *instr, MemCell *cell)
{
	int i;

	strtolower(instr);

	// See if instruction is in the table. If it is, call his parse function
	// and return the result. Return ERR_UnknownInstr if instruction is not
	// found in the table.

	for(i = 0; i < sizeof(parseTable) / sizeof(ParseInfo); i++)
	{
		if(!strncmp(instr, parseTable[i].instruction, 3))
		{
			return parseInstr(instr, parseTable[i].asmInstr, parseTable[i].addrFlags, cell);
		}
	}

	return ERR_UnknownInstr;
}

/** Converts an opcode to its mnemonic
 *
 * @retval ERR_InvalidInstr	The specific arguments are not allowed for the instruction
 * @retval ERR_UnknownInstr	Unknown instruction
 */
Error instToStr(Instruction instr, char *string)
{
	int i = 0;

	for(i = 0; i < sizeof(parseTable) / sizeof(ParseInfo); i++)
	{
		if(instr.operator == (unsigned int)parseTable[i].asmInstr)
		{
			ParseInfo info = parseTable[i];

			// Check the arguments of the instruction (so the addressing method)
			// and return the string representation of the instruction.

			if(info.addrFlags == FLAG_NoArgs)
			{
				// Addressing method and operand are ignored.
				strcpy(string, parseTable[i].instruction);
				return ERR_None;
			}
			else if(instr.adressering == ONMIDDELIJK
				&& (info.addrFlags & FLAG_Onmiddelijk) == FLAG_Onmiddelijk)
			{
				sprintf(string, "%s #%d", info.instruction, instr.operand);
				return ERR_None;
			}
			else if(instr.adressering == DIRECT
				&& (info.addrFlags & FLAG_Direct) == FLAG_Direct)
			{
				sprintf(string, "%s %d", info.instruction, instr.operand);
				return ERR_None;
			}
			else if(instr.adressering == INDIRECT
				&& (info.addrFlags & FLAG_Indirect) == FLAG_Indirect)
			{
				sprintf(string, "%s (%d)", info.instruction, instr.operand);
				return ERR_None;
			}
			
			// If we got here, the arguments or addressing method of the instruction
			// is invalid!
			return ERR_InvalidInstr;
		}
	}

	return ERR_UnknownInstr;
}
