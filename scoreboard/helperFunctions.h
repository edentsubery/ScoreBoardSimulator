#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"

cfg* createConfig();

void freeConfig(cfg* cfg);

cfg* parseConfig(FILE* cfgFd, char* line);

/*Seperate config params by type and execute the right parser function.*/
int parse(cfg* cfg, char* line);

int parseTraceUnitParam(char* ptr, int paramType, char* delimeter, char* unitTraceName);

int extractDigitFromStr(char* str);

int unitTraceNameToInt(char* str);

Instruction* createInstruction();

void freeInstruction(Instruction* inst);

void parseInstruction(Instruction* inst, int command);

InstructionQueue* initializeInstQueue();

void freeInstructionQueue(InstructionQueue* queue);

int addInstructionToInstructionQueue(InstructionQueue* queue, Instruction* inst);

int removeInstructionFromInstructionQueue(InstructionQueue* queue, int instIdx);

/*Check if instruction queue is full.*/
void setQueueProperties(InstructionQueue* queue);

Unit* createUnit(unitType type, int num);

void freeUnit(Unit* src);

Units* createUnits(int numOfUnits, int delay, unitType type);

void freeUnits(Units* src);

FunctionalUnit* createFunctionalUnit(cfg* cfg);

void freeFunctionalUnit(FunctionalUnit* src);

int verifyFiles(FILE** filesFd, char** filesPaths);

void printMemoutFile(FILE* fd, int* memory, int maxLines);

void printRegoutFile(FILE* fd, double* regs);

void printTraceunitFile(FILE* fd, FunctionalUnit* fus, int* resultTypes, int* resultIndexes, int cc);

int cmdToHex(Instruction* instruction);

float singlePrecisionToFloat(unsigned long singlePrecision);

int floatToSinglePrecision(float fl);

int areUnitsEqual(FunctionalUnit* fu, int q_type, int q_index, Unit* src2, int isJ);

void cleanAndWriteToFiles(FILE* fd, FunctionalUnit* fus, InstructionQueue* queue);

void printUnitsToTraceInstFile(FILE* fd);

void printTracinstFile(FILE* fd, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write);

unit* createEmptyPrintUnit();

void insertFirst(Unit* unitSrc);

unit* deleteFirst();

/*Return the length of the linked list.*/
int getLinkedListLength();

void sort();

void setData(unit* unitSrc, unit* dst);


static int numOfWorkingUnits = 0;

/*Simulator function, execute all simulator steps.*/
int simulator(char** filesPaths);

void performCommand(FunctionalUnit* functionalUnit, double  regs[16], int cc, int  memory[4096], FILE* filesFd[6], InstructionQueue* instQueue, int  resultTypes[16], int  resultIndexes[16]);

void finalize(FILE* filesFd[6], int  memory[4096], double  regs[16], char* line, cfg* cfg, FunctionalUnit* functionalUnit, InstructionQueue* instQueue);

int readMemin(FILE* meminFd, char* line, int* memory);

void initializeRegs(double* regs);

int issue(FunctionalUnit* fu, InstructionQueue* queue, int* resultTypes, int* resultIndexes, int cc);

int attachInstructionToUnit(FunctionalUnit* fus, Instruction* instruction, int* resultTypes, int* resultIndexes, int cc);

void setUnitFields(FunctionalUnit* functionalUnits, int type, int i, Instruction* instruction, int* resultIndexes, int* resultTypes, int cc);

void readOperand(FunctionalUnit* fu, double* regs, int cc);

void execution(FunctionalUnit* fu, int* mem, double* regs, int cc);

void executionInstruction(FunctionalUnit* fu, int* mem, double* regs, unsigned int instOp, int i, int j, int cc);

void writeResult(FILE** fds, FunctionalUnit* fu, InstructionQueue* queue, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc);

void writeResultFunctionalUnit(FILE** fds, FunctionalUnit* fus, Unit* unit, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc);

void checkIfLdAndStCollide(FunctionalUnit* fu, Instruction* stInst, int cc);

void freeSolution(FILE** fds, char* line, cfg* cfg, Instruction* cmd, FunctionalUnit* fus, InstructionQueue* queue);