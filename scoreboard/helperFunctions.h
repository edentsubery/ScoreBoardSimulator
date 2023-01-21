#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "structs.h"

Configuration* initConfiguration();

void freeConfiguration();

Configuration* analyzeConfiguration(FILE* cfgFd);

int analyzeConfig(Configuration* cfg);

int traceUnitAnalyze(char* ptr, int paramType, char* delimeter, char* unitTraceName);

int findIntegerInString(char* str);

int findUnitTraceName(char* str);

Instruction* initInstruction();

void freeInstruction(Instruction* inst);

void analyzeInstruction(Instruction* inst, int command);

InstQueue* initInstQueue();

void freeInstQueue();

int insertInstToQueue(Instruction* inst);

int removeInstAtIdxFromQueue(int instIdx);

void setQueueFullEmptyAttr();

Unit* initUnit(unitType type, int num);

void freeUnit(Unit* src);

Units* initUnits(int numOfUnits, int delay, unitType type);

void freeUnits(Units* src);

functionalUnit* initFunctionalUnits(Configuration* config);

void freeFunctionalUnit();

int openFiles(char** filesPaths);

void printMemoutFile(FILE* fd, int maxLines);

void printRegoutFile(FILE* fd);

void printTraceunitFile(FILE* fd, int cc);

float singlePrecisionToFloat(unsigned long singlePrecision);

int floatToSinglePrecision(float fl);

int unitsCompare(int q_type, int q_index, Unit* src2, int isJ);

void writeToFiles(FILE* fd);

void printUnitsToTraceInstFile(FILE* fd);

void printTracinstFile(FILE* fd, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write);

unitWrapper* initUnitWrapper();

void insertUnitWrapper(Unit* unitSrc);

unitWrapper* deleteUnitWrapper();

int length();

void swap(unitWrapper* current, unitWrapper* temp, unitWrapper* next);

void sort();

void updateUnitWrapper(unitWrapper* src, unitWrapper* dst);


static int busyUnitCnt = 0;

void memoryAllocation();

int simulator(char** filesPaths);

int initializeDataStructures(char** filesPaths);

void performCommand(int cc);

void printFiles();

void finalize();

int initMemory(FILE* meminFd);

void initRegs();

int issue(int cc);

int instructionToUnit(Instruction* instruction, int cc);

void setUnitFields(int type, int i, Instruction* instruction, int cc);

void readOp(int cc);

void executionOp(int cc);

void executionInst(unsigned int instOp, int i, int j, int cc);

void writeResult(int cc);

void writeResultFunctionalUnit(Unit* unit, int cc);

void checkLdSt(Instruction* stInst, int cc);

void deallocateMemory();
void ADD(int i, int j);
void SUB(int i, int j);
void DIV(int i, int j);

void MULT(int i, int j);
void ST(int i, int j, int clockCycles);
void LD(int i, int j);