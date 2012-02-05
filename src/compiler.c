#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "hardware.h"
#include "parser.h"
#include "memory.h"
#include "compiler.h"

#define MAXLEN	151

static void prepareLine(char *line);

/** Compile a file of assembly instructions. The first line will have number 0.
 *  Compiled instructions are saved in the memory. Error messages are displayed
 *  with the output funtion.
 *
 * @param [in] fp		File to compile
 * @param [in,out] memory	Memory where the compiled program will be saved
 * @retval ERR_ReadingFile	Error getting line from file
 * @retval ERR_OutOfMemory	Malloc failed
 */
Error compile(FILE *fp, Memory **memory, OutputFunc output)
{
	char line[MAXLEN];
	char buff[MAXLEN + 16];
	MemCell instr;
	unsigned int instrCounter = 0;

	assert(memory != NULL);

	while(!feof(fp))
	{
		Error	rval = ERR_None;

		// Get line from the file
		if(fgets(line, MAXLEN, fp) == NULL && !feof(fp))
		{
			return ERR_ReadingFile;
		}

		// Remove comments and newline
		prepareLine(line);

		// Handle empty lines
		if(line[0] == '\0')
		{
			sprintf(buff,
				"  WARNING: Empty line (%d), replaing with NOP instruction.\n",
				instrCounter);
			output(buff);
			instr.getal = 0;
		}
		else
		{
			// Parse the line and handle errors if any
			rval = parseAsmInstr(line, &instr); 
			if(rval != ERR_None)
			{
				switch(rval)
				{
				case ERR_UnknownInstr:
					sprintf(buff, "  ERROR: Unknown instruction at line %d: %s\n",
						instrCounter, line);
					break;
				case ERR_InvalidInstr:
					sprintf(buff, "  ERROR: Invalid use of instruction at line %d: %s\n",
						instrCounter, line);
					break;
				default:
					sprintf(buff, "  ERROR: Line %d: %s\n",
						instrCounter, line);
					break;
				}
				output(buff);
				output("  > WARNING: Replacing with NOP instruction!\n");
				instr.getal = 0;
			}
		}

		// Write the parsed instruction to memory
		rval = writeMemCell(memory, instrCounter, instr);
		if(rval != ERR_None)
		{
			return rval;
		}

		instrCounter++;
	}

	output("  Compilation complete.\n");

	return ERR_None;
}

/** Prepares a line for parsing: removes comments and the newline */
static void prepareLine(char *line)
{
	int	i, length;

	length = (int)strlen(line);
	for(i = 0; i < length; i++)
	{
		if(line[i] == ';')
		{
			line[i] = '\0';
			break;
		}
		else if(line[i] == '\n')
		{
			line[i] = '\0';
			break;
		}
	}
}



