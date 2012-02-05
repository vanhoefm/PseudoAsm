/** 
 * Sorted linked list of numbers.
 *
 * NOTE: Doubles are ignored! Meaning when adding a number that is already in the list,
 *	the list will remain UNCHANGED.
 */
#include <stdlib.h>
#include <assert.h>
#include "numberlist.h"

/** Add a number to the list
 *
 * @retval ERR_OutOfMemory	Malloc failed
 */
Error addNumber(NumberList **l, int number)
{
	NumberList	*p = NULL,
				*q = NULL,
				*toAdd = NULL;

	assert(l != NULL);

	p = *l;

	// Find where to add the number. This is a sorted list!
	while(p != NULL && p->number < number)
	{
		q = p;
		p = p->next;
	}

	// If the number is already in the list, ignore it.
	if(p != NULL && p->number == number)
	{
		return ERR_None;
	}

	// Add the number to the list
	if(q == NULL)
	{	// Add as beginning of list
		toAdd = (NumberList*) malloc(sizeof(NumberList));
		if(!toAdd)
		{
			return ERR_OutOfMemory;
		}
		toAdd->number = number;
		toAdd->next = *l;
		*l = toAdd;
	}
	else
	{	// Add in the middle or at the end
		toAdd = (NumberList*) malloc(sizeof(NumberList));
		if(!toAdd)
		{
			return ERR_OutOfMemory;
		}
		toAdd->number = number;
		toAdd->next = p;
		q->next = toAdd;
	}

	return ERR_None;
}

/** Pops the smallest number in the list.
 *
 * @retval ERR_ListEmpty The list is emtpy.
 */
Error popNumber(NumberList **l, int * number)
{
	NumberList *toDel = NULL;

	assert(l != NULL);

	// Return error if the list is empty
	if(*l == NULL)
	{
		return ERR_ListEmpty;
	}

	// Return and free the number
	toDel = *l;
	*l = (*l)->next;
	*number = toDel->number;
	free(toDel);

	return ERR_None;
}

/* returns ERR_NotFound, ERR_None */
Error delNumber(NumberList **l, int number)
{
	NumberList	*p = NULL,
			*q = NULL;

	assert(l != NULL);

	p = *l;
	while(p != NULL && p->number < number)
	{
		q = p;
		p = p->next;
	}

	// Delete first item of list
	if(q == NULL && p != NULL && p->number == number)
	{
		*l = p->next;
		free(p);
	}
	else if(p != NULL && p->number == number)
	{
		q->next = p->next;
		free(p);
	}
	else
	{
		return ERR_NotFound;
	}

	return ERR_None;
}

/* Check if a number is in the list */
int hasNumber(NumberList *l, int number)
{
	while(l != NULL && l->number < number)
	{
		l = l->next;
	}

	return (l != NULL && l->number == number);
}

/** Frees the linked list of numbers */
void freeNumberList(NumberList **l)
{
	NumberList	*toDel = NULL,
				*p = NULL;

	assert(l != NULL);

	// Free every element in the list
	p = *l;
	while(p != NULL)
	{
		toDel = p;
		p = p->next;
		free(toDel);
	}

	// Reset the list pointer
	*l = NULL;
}
