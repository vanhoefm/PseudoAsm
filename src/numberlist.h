#ifndef _PSEUDOASM_INC_NUMBERLIST_H_
#define _PSEUDOASM_INC_NUMBERLIST_H_

#include "errors.h"

typedef struct NumberList
{
	int number;
	struct NumberList *next;
} NumberList;

/* Add a number to the list. Ignores doubles */
Error addNumber(NumberList **l, int number);

/* Pop the first number of the list */
Error popNumber(NumberList **l, int * number);

/* Delete a number from the list */
Error delNumber(NumberList **l, int number);

/* Is this number in the list? */
int hasNumber(NumberList *l, int number);

/* Free the number list */
void freeNumberList(NumberList **l);

#endif // _PSEUDOASM_INC_NUMBERLIST_H_
