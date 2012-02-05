#ifndef _PSEUDOASM_INC_HARDWARE_H_
#define _PSEUDOASM_INC_HARDWARE_H_

// Functions used for input and output
// Number input
typedef int (*FuncNumInp)(void);
// Number output
typedef void (*FuncNumOut)(int number);
// Debug string output
typedef void (*OutputFunc)(char *line);

// -------------
//   PROCESSOR
// -------------

/** Assembly instruction */
typedef struct
{
	unsigned int operand	: 24;
	unsigned int adressering: 2;
	unsigned int operator	: 6;
} Instruction;

/** Opcodes of the assembly instructions */
typedef enum AsmInstr
{
	// No Operation
	A_NOP = 0x00,
	// Save/Load register
	A_LDA = 0x04,
	A_LDB = 0x05,
	A_STA = 0x08,
	A_STB = 0x09,
	// I/O
	A_INP = 0x18,
	A_OUT = 0x1C,
	// Aremethic
	A_ADD = 0x20,
	A_SUB = 0x24,
	A_MUL = 0x28,
	A_DIV = 0x2C,
	// Jumps
	A_JMP = 0x30,
	A_JSP = 0x32,
	A_JSN = 0x34,
	A_JIZ = 0x36,
	A_JOF = 0x38,
	A_JSB = 0x3C,
	A_RTS = 0x3D,
	// Stop
	A_HLT = 0x3F//,
	// Extra: set stack
	//A_SST = 0x80
} AsmInstr;

/** Addressing method */
enum Adressering
{
	ONMIDDELIJK	= 0x0,
	DIRECT		= 0x1,
	INDIRECT	= 0x2,
	GEINDEXEERD 	= 0x3
};

// -------------
//    MEMORY
// -------------

/** A memory cell */
typedef union geheugenCell
{
	int getal;
	Instruction instructie;
} MemCell;

#endif // _PSEUDOASM_INC_MEMORY_H_

