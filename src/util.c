#include "util.h"
#include <ctype.h>

/** Convert a string to lowercase */
char * strtolower(char input[])
{
	int i;

	for(i = 0; input[i] != '\0'; i++)
	{
		input[i] = (char)tolower(input[i]);
	}

	return input;
}
