/* Mathy Vanhoef - Version 1.2
 *
 * Provides more advanced functions to get user/file input in console mode.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "input.h"
#include "errors.h"

/** Max length of a line */
#define MAXLEN 256

Error getline(char input[], int maxlen, FILE *fp)
{
	int	len = 0;

	assert(input != NULL);
	assert(fp != NULL);

	// Get string
	if(!fgets(input, maxlen, fp))
	{
		return ERREOF;
	}

	len = (int)strlen(input);
	// Check if it was the complete string
	if(len == maxlen - 1 && input[len - 1] != '\n')
	{
		readendline(fp);
		return ERROVERFLOW;
	}

	input[len - 1] = '\0'; // Remove newline

	return ERR_None;
}

char *userinput(int *outLen)
{
	char	buff[MAXLEN],
		*outBuff = NULL;
	int	len = 0;

	while((len = getline(buff, MAXLEN, stdin)) == ERROVERFLOW)
	{
		printf("Invoer is maximaal %d karakters lang.\n"
			"Voor informatie opnieuw in.\n", MAXLEN - 1);
	}

	if(len == ERREOF)
	{
		return NULL;
	}

	outBuff = (char*)malloc(len + 1);
	if(!outBuff)
	{
		printf("Out of memory\n");
		exit(1);
	}
	strcpy(outBuff, buff);

	/* return length if wanted by caller */
	if(outLen)
	{
		*outLen = len;
	}

	return outBuff;
}

int optioninput(void)
{
	int ch = 0;
	
	/* First character must be printable ASCII character (and not a whitespace)
	 * If not the case, ask user for input again */
	while((ch = getchar()) < 33 || ch > 126)
	{
		/* Flush stdin if needed */
		if(ch != '\n')
		{
			readendline(stdin);
		}
		else if (ch == EOF)
		{
			return ERREOF;
		}
		printf("Typ een optie en druk op [enter].\n:> ");
	}

	/* Flush rest of buffer */
	readendline(stdin);

	return ch;
}

int numberinput(void)
{
	char	buff[12];
	int	rval = 0;

	printf("Enter a number: ");

	while((rval = getline(buff, 12, stdin)) == ERROVERFLOW || !isnumber(buff))
	{

		printf("Not a valid number. Try again.\n");
		printf("Enter a number: ");
	}

	return atoi(buff);
}

/** Skips the rest of the line, including newline character */
int readendline(FILE *fp)
{
	fscanf(fp, "%*[^\n]");	/* read untill newline */
	return fgetc(fp);	/* Remove newline */
}

BOOL isnumber(char input[])
{
	int i = 0;
	int signincl = 0;

	// Sign of number
	if(input[0] == '-' || input[0] == '+')
	{
		i++;
		signincl = 1;
	}

	// Number itself
	for(; input[i] != '\0' && isdigit(input[i]); i++)
	{
		/* all ok */
	}

	/* There must be at least one digit (not counting sign)
	   && all characters were digits */
	return (i > signincl && input[i] == '\0');
}

double doubleinput(void)
{
	char	buff[20];
	double	input = 0.0;

	do
	{
		getline(buff, 20, stdin);
	} while(sscanf(buff, "%lf", &input) != 1);

	return input;
}


/** Input functions specificly for our files **/


char *getInfo(const char identifier[], FILE *fp, char output[], int maxlen)
{
	char	*work = NULL;
	char	buff[300];

	assert(fp != NULL);
	assert(identifier != NULL);

	/* Get line from file */
	if(getline(buff, 300, fp) == ERROVERFLOW)
	{
		return NULL;
	}

	/* Check if the line contains identifier */
	work = strstr(buff, identifier);
	if(!work)
	{
		return NULL; /* Error if line doesn't contain identifier */
	}
	
	/* Copy info to buffer if buffer is large enough */
	work += strlen(identifier);
	if((int)strlen(work) > maxlen - 1)
	{
		return NULL;
	}
	strcpy(output, work);

	return output;
}

char *getMallocedInfo(const char identifier[], FILE *fp)
{
	char	buff[300];
	char	*work;
	char	*output;
	int		length;

	/* get info */
	work = getInfo(identifier, fp, buff, 300);

	if(!work)
	{
		return NULL;
	}

	/* Allocate memory and copy info to it */
	length = (int)strlen(work);
	output = malloc(length + 1);
	if(!output)
	{
		printf("Out of memory\n");
		exit(1);
	}
	strcpy(output, work);

	return output;
}

