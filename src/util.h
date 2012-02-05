#ifndef _PSEUDOASM_INC_UTIL_H_
#define _PSEUDOASM_INC_UTIL_H_

#include <stdlib.h>
// To clear the console screen
// Linux
#define CLEARSCREEN system("clear")
// Windows
//#define CLEARSCREEN system("cls")

/* Convert a string to lowercase */
char * strtolower(char input[]);

#endif // _PSEUDOASM_INC_UTIL_H_
