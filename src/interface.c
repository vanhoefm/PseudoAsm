#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "numberlist.h"
#include "editor.h"
#include "input.h"
#include "runtime.h"
#include "util.h"

// Menu options
void menuOpenProg(void);
void menuNieuwProg(void);
void openProgram(char *filename);

// Interface functions that handle certain commands
Error cmdRun(char *cmd);
Error cmdStep(char *cmd);
Error cmdExit(char *cmd);
Error cmdExec(char *cmd);
Error cmdSetBp(char *cmd);
Error cmdListBp(char *cmd);
Error cmdDelBp(char *cmd);
Error cmdAsm(char *cmd);
Error cmdStatus(char *cmd);
Error cmdStack(char *cmd);
Error cmdHelp(char *cmd);

//
// COMMAND TABLE INFO AND DECLERATIONS
//
#define NUMCOMMANDS sizeof(commandList) / sizeof(UICommand)

typedef Error (*HandleCommand)(char *cmd);

typedef struct UICommand
{
	const char *command;
	HandleCommand handler;
	const char *help;
} UICommand;

/** List of commands that the interface accepts, and a short description.
 *
 *  For synonyms, the first one needs the description, the others
 *  a NULL pointer (see help command).
 */
UICommand commandList[] = 
{
	{"i", cmdStatus, "Display registers and next instruction"},
	{"status", cmdStatus, NULL},
	{"r", cmdRun, "Run the program"},
	{"run", cmdRun, NULL},
	{"s", cmdStep, "Execute the next instruction"},
	{"step", cmdStep, NULL},
	{"exec", cmdExec, "Execute an instruction: exec instruction"},
	{"e", cmdExec, NULL},
	{"bp", cmdSetBp, "Set a breakpoint: bp address"},
	{"bpl", cmdListBp, "List all breakpoints"},
	{"bpd", cmdDelBp, "Delete a breakpoint: bpd address"},
	{"a", cmdAsm, "Assemble an instruction and save it to memory: a address instruction"},
	{"asm", cmdAsm, NULL},
	{"stack", cmdStack, "Manipulate the stack: stack, stack address, stack trace on/off"},
	{"exit", cmdExit, "Exit the assembler program"},
	{"quit", cmdExit, NULL},
	{"help", cmdHelp, "Display all commands"},
	{"h", cmdHelp, NULL}
};

// Method that will be used to output numbers by our assembly program
void numberoutput(int number)
{
	printf("Output: %d\n", number);
}

// Output generated by the compiler, runtime, etc.
void consoleoutput(char *output)
{
	printf("%s",output);
}

void menuMain(void)
{
	int optie = 0;
	int exit = 0;

	do
	{
		exit = 0;

		CLEARSCREEN;

		printf(
		"\n"
		"        **************************************************************\n"
		"        *                                                            *\n"
		"        *      ,---.                   |         ,---.               *\n"
		"        *      |---',---.,---..   .,---|,---.    |---|,---.,-.-.     *\n"
		"        *      |    `---.|---'|   ||   ||   |    |   |`---.| | |     *\n"
		"        *      `    `---'`---'`---'`---'`---'    `   '`---'` ' '     *\n"
		"        *                                                            *\n"
		"        **************************************************************\n"
		"\n"
		"                        > Press CTRL+C to force exit <\n"
		"\n");	

		printf("Pick an option:\n"
			"---------------\n"
			"1) Open Program\n"
			"2) New/Edit Program\n"
			"3) Exit\n");
	
		do
		{
			printf(":> ");
			optie = optioninput();
			if(optie < '1' || optie > '3')
			{
				printf("Invalid menu selection.\n");
			}
		} while(optie < '1' || optie > '3');
	
		switch(optie)
		{
		case '1':
			menuOpenProg();
			break;
		case '2':
			menuNieuwProg();
			break;
		case '3':
			exit = 1;
			printf("\nThanks for using PseudoAsm.\n\n"
				"\tMade by Mathy Vanhoef\n\n");
			break;
 		}
	} while(!exit);
}

void menuOpenProg(void)
{
	char buff[151];
	int valid = 0;
	FILE *prog;

	do
	{
		printf("Give the name and location of the program:\n"
		":> ");

		if(fgets(buff, 151, stdin) == NULL)
		{
			printf("Unexpected error getting user input\n");
			continue;
		}

		buff[strlen(buff) - 1] = '\0';

		// Check if file exists
		prog = fopen(buff, "r+");
		if(!prog)
		{
			perror("Error opening file");
			valid = 0;
		}
		else
		{
			fclose(prog);
			valid = 1;
			openProgram(buff);
		}
	} while(!valid);
}

void openProgram(char *filename)
{
	Error	rval = ERR_None;
	char	buff[151];
	char	cmd[51];

	CLEARSCREEN;

	printf("Initializing runtime ...\n");

	rval = rntInit(filename, numberinput, numberoutput, consoleoutput);
	if(rval != ERR_None)
	{
		printf("Error initializing runtime (%d)\n", rval);
	}

	do
	{
		int i = 0;

		// Get command
		printf(":> ");
		if(getline(buff, 151, stdin) != ERR_None)
		{
			printf("Unexpected error getting user input\n");
			continue;
		}

		strtolower(buff);
		sscanf(buff, "%s", cmd);

		// Go through command list and exute if valid command
		for(i = 0; i < NUMCOMMANDS; i++)
		{
			if(strcmp(commandList[i].command, cmd) == 0)
			{
				rval = commandList[i].handler(buff);
				break;
			}
		}
		
		if(i == NUMCOMMANDS)
		{
			printf("Invalid command. Type \"help\" for a list of commands.\n");
			rval = ERR_None; // Let the user try again ...
		}
	} while(rval == ERR_None);

	rntDeInit();

	printf("Press enter to return to main menu ..");
	readendline(stdin);
}

Error cmdStatus(char *cmd)
{
	rntDisplayStatus();

	return ERR_None;
}

Error cmdRun(char *cmd)
{
	Error rval = ERR_None;

	rval = rntRun();
	if(rval == ERR_Breakpoint)
	{
		return ERR_None;
	}
	else
	{
		return rval;
	}
}

Error cmdStep(char *cmd)
{
	// Makes it look better
	printf("\n");

	return rntStep();
}

Error cmdExit(char *cmd)
{
	return ERR_EndOfProgram;
}

// e of exec
Error cmdExec(char *cmd)
{
	if(strncmp(cmd, "e ", 2) == 0)
	{
		cmd += 2;
	}
	else if(strncmp(cmd, "exec ", 5) == 0)
	{
		cmd += 5;
	}
	else
	{
		printf("Usage: exec instruction\n");
		return ERR_None;
	}

	rntFlyExec(cmd);

	return ERR_None;
}

Error cmdSetBp(char *cmd)
{
	int address;
	char end[2];

	if(sscanf(cmd, "bp %d %1s", &address, end) == 1)
	{
		rntSetBp(address);
	}
	else
	{
		printf("Usage: bp address\n");
	}

	return ERR_None;
}

Error cmdListBp(char *cmd)
{
	rntListBp();

	return ERR_None;
}

Error cmdDelBp(char *cmd)
{	
	int address;
	char end[2];

	if(sscanf(cmd, "bpd %d %1s", &address, end) == 1)
	{
		rntDelBp(address);
	}
	else
	{
		printf("Usage: bpd address\n");
	}

	return ERR_None;
}

Error cmdAsm(char *cmd)
{
	unsigned int address;
	char instr[31];
	char end[2];

	if(sscanf(cmd, "a %u %30[^\n] %1s", &address, instr, end) == 2)
	{
		rntFlyAsm(address, instr);
	}
	else if(sscanf(cmd, "asm %u %30[^\n] %1s", &address, instr, end) == 2)
	{
		rntFlyAsm(address, instr);
	}
	else
	{
		printf("Usage: asm address instruction\n");
	}

	return ERR_None;
}

void menuNieuwProg(void)
{
	printf("Opening edit window, close edit window to return to console ...\n");
	// Make sure it get's displayed before we start the gtk main loop!
	fflush(stdout);

	// Launch gtk editor
	openEditWindow();
}

Error cmdStack(char *cmd)
{
	unsigned int pointer;
	char end[2];

	if(sscanf(cmd, "stack %d %1s", &pointer, end) == 1)
	{
		rntSetStack(pointer);
		printf("Stack Pointer: %d\n", pointer);
	}
	else if(sscanf(cmd, "stack %1s", end) == EOF)
	{
		printf("Stack Pointer: %d\n", rntGetStack());
	}
	else if(sscanf(cmd, "stack trace on %1s", end) == EOF)
	{
		rntStackTrace(1);
		printf("Stack changes are now traced\n");
	}
	else if(sscanf(cmd, "stack trace off %1s", end) == EOF)
	{
		rntStackTrace(0);
		printf("Stack trace is disabled\n");
	}
	else
	{
		printf("Invalid use of command\n");
	}

	return ERR_None;
}

/* Display a _very_ simple help: list all the commands */
Error cmdHelp(char *cmd)
{
	int i = 0, orgCmdHelp;

	printf("  Command List:\n");

	i = 0;
	while(i < NUMCOMMANDS)
	{
		orgCmdHelp = i;
		printf("%s", commandList[i++].command);
		while(i < NUMCOMMANDS && commandList[i].help == NULL)
		{
			printf(", %s", commandList[i++].command);
		}
		printf(": %s\n", commandList[orgCmdHelp].help);
	}

	return ERR_None;
}