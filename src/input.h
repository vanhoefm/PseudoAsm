/* Mathy Vanhoef - Version 1.2
 *
 * Provides more advanced functions to get user/file input in console mode.
 */
#ifndef REISBUREAU_INC_INPUT_H_
#define REISBUREAU_INC_INPUT_H_

#include <stdio.h> // for FILE struct declaration

#define ERROVERFLOW -1
#define ERREOF -2
#define BOOL int

#include <stdlib.h>
#include "errors.h"

/** All the functions will "flush" the buffer untill the next EndOfLine **/

/* Gets a line from stdin. If input is too small, it will return ERROVERFLOW */
Error getline(char input[], int maxlen, FILE *fp);

/* Returns a line from the user input. Memory for the string is allocated in
 * the function! If the input of the user it too large, it will ask a smaller
 * input line from the user. */
char *userinput(int *outLen);

/* Returns TRUE if all the characters are numbers, else FALSE.
 * Note that in the way, negative numbers are not accepcted. */
BOOL isnumber(char input[]);

/* Read a single (printable and not whitespace) character */
int optioninput(void);

/* Reads untill a proper number is entered. Postive or negative number. Not float. */
int numberinput(void);

/* Reads untill a proper number is entered */
double doubleinput(void);

/* Returns the string that is behind the "identifier string" */
char *getInfo(const char identifier[], FILE *fp, char output[], int maxlen);

/* Returns the allocated string that is behind the "identifier string" */
char *getMallocedInfo(const char identifier[], FILE *fp);

/* Reads untill it encounters an EOF. Returns the last character read, this can be EOF! */
int readendline(FILE *fp);

#endif // REISBUREAU_INC_VAKANTIE_H_

