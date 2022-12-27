
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define MEMORY_LENGTH 4096
#define NUM_OF_REGISTERS 16
#define NUM_OF_CYCLES_TYPES 4
#define NUM_OF_FILES 6
#define NUM_OF_UNITS 6
#define NUM_OF_UNITS_FOR_FUNCTIONAL_UNITS 5
#define NUM_OF_OPCODES 7
#define MAX_NUM_OF_FUNCTIONAL_UNITS 5
#define MAX_LENGTH 10
#define MAX_LINE_LENGTH 300
#define NUM_OF_INSTRUCTION_IN_QUEUE 16

#define No 0
#define Yes 1

typedef enum {
	CFG = 0,
	MEMIN,
	MEMOUT,
	REGOUT,
	TRACEINST,
	TRACEUNIT,
}file;

typedef enum {
	LD = 0,
	ST,
	ADD,
	SUB,
	MULT,
	DIV,
	HALT,
}opcode;

typedef enum {
	Integer,
	Add,
	Sub,
	Mult,
	Div,
}regStatus;

typedef enum {
	UNIT_LD = 0,
	UNIT_ST,
	UNIT_ADD,
	UNIT_SUB,
	UNIT_MULT,
	UNIT_DIV,
}unitType;

typedef enum {
	ISSUE,
	READ_OPERAND,
	EXECUTION,
	WRITE_RESULT,
}instStatus;

static const char* filesNames[NUM_OF_FILES] = { "cfg", "memin", "memout", "regout", "traceinst", "traceunit" };


static const char* unitsTypeNames[NUM_OF_UNITS] = { "LD", "ST", "ADD", "SUB", "MUL", "DIV" };


static const char* opcodeNames[NUM_OF_OPCODES] = { "LD", "ST", "ADD", "SUB", "MUL", "DIV", "HALT" };



static const char* configUnitsTypes[2 * NUM_OF_UNITS + 1] = { "ld_nr_units", "st_nr_units", "add_nr_units", "sub_nr_units", "mul_nr_units", "div_nr_units",
"ld_delay", "st_delay", "add_delay" , "sub_delay", "mul_delay", "div_delay", "trace_unit" };


typedef struct instructionCommand {
	unsigned int opcode;
	unsigned int dst;
	unsigned int src0;
	unsigned int src1;
	unsigned int imm;
	unsigned int command;

	int instType;
	int instIndex;
	int status;
	int isEmpty;
	int fetchedTime;
	int queueIndex;
	int writeToFile;
	int canIssue;

	int executionTime;
	int stateCC[NUM_OF_CYCLES_TYPES];
	double instRes;
}Instruction;

typedef struct instructionQueue {
	Instruction* queue[NUM_OF_INSTRUCTION_IN_QUEUE];
	int isQueueFull;
	int isQueueEmpty;
}InstructionQueue;



typedef struct unit {
	Instruction* instruction;
	int type;
	int unitNum;
	int isEmpty;
	int canWriteResult;

	int busy;
	int op;
	int Fi;
	int Fj;
	double FjVal;
	int Fk;
	double FkVal;

	int QjType;
	int QkType;
	int QjIdx;
	int QkIdx;

	int Rj;
	int Rk;
}Unit;

typedef struct units {
	Unit* units[NUM_OF_UNITS];
	int type;
	int numOfTotalUnits;
	int numOfActiveUnits;
	int delay;
	int canInsert;
}Units;

typedef struct functionalUnit {
	Units* functionalUnit[NUM_OF_UNITS];
	int unitName;
	int unitNum;
}FunctionalUnit;


typedef intptr_t ssize_t;

typedef struct configParams {
	int units[6];
	int delays[6];
	int name;
	int unitNum;
}cfg;

typedef enum unitsEnum {
	ADD_UNIT = 0,
	SUB_UNIT,
	MULT_UNIT,
	DIV_UNIT,
	LD_UNIT,
	ST_UNIT,
	ADD_DELAY,
	SUB_DELAY,
	MULT_DELAY,
	DIV_DELAY,
	LD_DELAY,
	ST_DELAY,
	TRACE_UNIT,
}configUnits;


typedef struct unitNode {
	unsigned int op;
	int unitType;
	int unitIndex;
	int fetchCC;
	int issueCC;
	int readCC;
	int exeCC;
	int writeCC;
	struct unitNode* next;
}unit;

static unit* head = NULL;
static unit* current = NULL;