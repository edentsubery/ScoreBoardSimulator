
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define MEMORY_LENGTH 4096
#define REGISTERS_NUMBER 16
#define NUM_OF_OPERATIONS_STAGES 4
#define FILES_NUMBER 6
#define UNITS_NUMBER 6
#define NUM_OF_UNITS_FOR_FUNCTIONAL_UNITS 5
#define NUM_OF_OPCODES 7
#define MAX_NUM_OF_ACTIVE_UNITS 5
#define MAX_LENGTH 10
#define MAX_LINE_LENGTH 300
#define NUM_OF_INSTRUCTION_QUEUE 16

#define No 0
#define Yes 1

typedef enum {
	CONFIG = 0,
	MEMIN,
	MEMOUT,
	REGOUT,
	TRACEINST,
	TRACEUNIT,
}file;

typedef enum {
	OP_LD = 0,
	OP_ST,
	OP_ADD,
	OP_SUB,
	OP_MULT,
	OP_DIV,
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

static const char* filesNames[FILES_NUMBER] = { "cfg", "memin", "memout", "regout", "traceinst", "traceunit" };


static const char* unitsTypeNames[UNITS_NUMBER] = { "LD", "ST", "ADD", "SUB", "MUL", "DIV" };


static const char* opcodeNames[NUM_OF_OPCODES] = { "LD", "ST", "ADD", "SUB", "MUL", "DIV", "HALT" };



static const char* configUnitsTypes[2 * UNITS_NUMBER + 1] = { "ld_nr_units", "st_nr_units", "add_nr_units", "sub_nr_units", "mul_nr_units", "div_nr_units",
"ld_delay", "st_delay", "add_delay" , "sub_delay", "mul_delay", "div_delay", "trace_unit" };


typedef struct instructionCommand {
	unsigned int opcode;
	unsigned int dst;
	unsigned int src0;
	unsigned int src1;
	unsigned int imm;
	unsigned int command;

	int operation;
	int index;
	int status;
	int empty;
	int fetchCycles;
	int queueIndex;
	int fileWriting;
	int issue;

	int executionCycles;
	int clockCyclesOperation[NUM_OF_OPERATIONS_STAGES];
	double result;
}Instruction;

typedef struct instructionQueue {
	Instruction* queue[NUM_OF_INSTRUCTION_QUEUE];
	int fullQueue;
	int emptyQueue;
}InstQueue;



typedef struct unit {
	Instruction* instruction;
	int type;
	int unitNum;
	int empty;
	int writeResult;

	int busy;
	int op;
	int f_i;
	int f_j;
	double f_j_value;
	int f_k;
	double f_k_value;

	int q_j_type;
	int q_k_type;
	int q_j_index;
	int q_k_index;

	int r_j;
	int r_k;
}Unit;

typedef struct units {
	Unit* units[UNITS_NUMBER];
	int type;
	int totalUnitsNum;
	int activeUnitsNum;
	int delay;
	int canEnter;
}Units;

typedef struct functionalUnit {
	Units* activeUnit[UNITS_NUMBER];
	int unitName;
	int unitNum;
}ActiveUnit;


typedef intptr_t ssize_t;

typedef struct configParams {
	int units[6];
	int delays[6];
	int name;
	int unitNum;
}Configuration;

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
}PrintUnit;

static PrintUnit* head = NULL;
static PrintUnit* current = NULL;