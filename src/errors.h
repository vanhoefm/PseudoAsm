#ifndef _PSEUDOASM_INC_ERROR_H_
#define _PSEUDOASM_INC_ERROR_H_

typedef enum Error
{
	ERR_None,

	ERR_OutOfMemory,

	ERR_InvalidInstr,

	ERR_DivideZero,

	ERR_UnknownInstr,

	ERR_EndOfProgram,

	ERR_ReadingFile,

	ERR_ListEmpty,

	ERR_NotFound,

	ERR_OpeningFile,

	ERR_InvalidState,

	ERR_Breakpoint
} Error;

#endif // _PSEUDOASM_INC_ERROR_H_

