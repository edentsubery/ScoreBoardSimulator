#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"

Configuration* initConfiguration();

void freeConfiguration(Configuration* cfg);

Configuration* analyzeConfiguration(FILE* cfgFd, char* line);

/*Seperate config params by type and execute the right parser function.*/
int analyzeConfig(Configuration* cfg, char* line);

int traceUnitAnalyze(char* ptr, int paramType, char* delimeter, char* unitTraceName);

int findIntegerInString(char* str);

int findUnitTraceName(char* str);

Instruction* initInstruction();

void freeInstruction(Instruction* inst);

void analyzeInstruction(Instruction* inst, int command);

InstQueue* initInstQueue();

void freeInstQueue(InstQueue* queue);

int enqueueInstQueue(InstQueue* queue, Instruction* inst);

int dequeueInstQueue(InstQueue* queue, int instIdx);

/*Check if instruction queue is full.*/
void emptyFullInstQueue(InstQueue* queue);

Unit* initUnit(unitType type, int num);

void freeUnit(Unit* src);

Units* initUnits(int numOfUnits, int delay, unitType type);

void freeUnits(Units* src);

ActiveUnit* initActiveUnit(Configuration* cfg);

void freeActiveUnit(ActiveUnit* src);

int openFiles(FILE** filesFd, char** filesPaths);

void printMemoutFile(FILE* fd, int* memory, int maxLines);

void printRegoutFile(FILE* fd, double* regs);

void printTraceunitFile(FILE* fd, ActiveUnit* fus, int* resultTypes, int* resultIndexes, int cc);

int hexCommand(Instruction* instruction);

float singlePrecisionToFloat(unsigned long singlePrecision);

int floatToSinglePrecision(float fl);

int unitsCompare(ActiveUnit* fu, int q_type, int q_index, Unit* src2, int isJ);

void writeToFiles(FILE* fd, ActiveUnit* fus, InstQueue* queue);

void printUnitsToTraceInstFile(FILE* fd);

void printTracinstFile(FILE* fd, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write);

PrintUnit* initPrintUnit();

void insertPrintUnit(Unit* unitSrc);

PrintUnit* deletePrintUnit();

/*Return the length of the linked list.*/
int length();

void sort();

void updatePrintUnit(PrintUnit* unitSrc, PrintUnit* dst);


static int bussyUnitsNumber = 0;

/*Simulator function, execute all simulator steps.*/
int simulator(char** filesPaths);

void performCommand(ActiveUnit* functionalUnit, double  regs[16], int cc, int  memory[4096], FILE* filesFd[6], InstQueue* instQueue, int  resultTypes[16], int  resultIndexes[16]);

void finalize(FILE* filesFd[6], int  memory[4096], double  regs[16], char* line, Configuration* cfg, ActiveUnit* functionalUnit, InstQueue* instQueue);

int initMemory(FILE* meminFd, char* line, int* memory);

void initRegs(double* regs);

int issue(ActiveUnit* fu, InstQueue* queue, int* resultTypes, int* resultIndexes, int cc);

int instructionToUnit(ActiveUnit* fus, Instruction* instruction, int* resultTypes, int* resultIndexes, int cc);

void setUnitFields(ActiveUnit* functionalUnits, int type, int i, Instruction* instruction, int* resultIndexes, int* resultTypes, int cc);

void readOp(ActiveUnit* fu, double* regs, int cc);

void executionOp(ActiveUnit* fu, int* mem, double* regs, int cc);

void executionInst(ActiveUnit* fu, int* mem, double* regs, unsigned int instOp, int i, int j, int cc);

void writeResult(FILE** fds, ActiveUnit* fu, InstQueue* queue, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc);

void writeResultActiveUnit(FILE** fds, ActiveUnit* fus, Unit* unit, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc);

void checkLdSt(ActiveUnit* fu, Instruction* stInst, int cc);

void freeSimulator(FILE** fds, char* line, Configuration* cfg, Instruction* cmd, ActiveUnit* fus, InstQueue* queue);