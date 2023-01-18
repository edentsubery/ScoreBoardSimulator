#include "helperFunctions.h"
//
Configuration* initConfiguration() {
	Configuration* config = (Configuration*)malloc(sizeof(Configuration));
	if (!config) {
		return 0;
	}
	config->name = -1;
	return config;
}
//
void freeConfiguration() {
	if (!cfg) {
		return;
	}
	free(cfg);
}
//
Configuration* analyzeConfiguration(FILE* cfgFile, char* line) {
	Configuration* config = initConfiguration();
	if (!config) {
		return 0;
	}
	int linesCounter = 0;
	size_t length = 0;
	ssize_t linesRead = 0;
	
	while (fgets(line, MAX_LINE_LENGTH, cfgFile) != 0) {
		if (!analyzeConfig(config, line)) {
			return 0;
		}
	}
	return config;
}
//
int analyzePar(char* pointer, int parType, char* boundary) {
	pointer = strtok(NULL, boundary);
	int num = atoi(pointer);
	return num;
}
//
int analyzeConfig(Configuration* cfg, char* line) {
	if (line == '\n') {
		return 1;
	}
	char* boundary = " =\n\t";
	char* pointer = strtok(line, boundary);
	if (!pointer) {
		return 1;
	}
	int par;
	char unitTraceName[MAX_LENGTH];
	for (int i = 0; i < UNITS_NUMBER; i++) {
		if (strcmp(pointer, configUnitsTypes[i]) == 0) {
			if ((par = analyzePar(pointer, i, boundary)) == -1) {
				return 0;
			}
			cfg->units[i] = par;
			return 1;
		}
	}
	for (int i = 0; i < UNITS_NUMBER; i++) {
		if (strcmp(pointer, configUnitsTypes[i + UNITS_NUMBER]) == 0) {
			if ((par = analyzePar(pointer, i, boundary)) == -1) {
				return 0;
			}
			cfg->delays[i] = par;
			return 1;
		}
	}
	if (strcmp(pointer, configUnitsTypes[TRACE_UNIT]) == 0) {
		if ((par = traceUnitAnalyze(pointer, TRACE_UNIT, boundary, unitTraceName)) == -1) {
			return 0;
		}
		if (unitTraceName) {
			char* bound = "01234";
			cfg->unitNum = findIntegerInString(unitTraceName);
			char* pointer = strtok(unitTraceName, bound);
			int name = findUnitTraceName(unitTraceName);
			if (name == -1) {
				printf("conifguration trace_unit_name invalide!\n");
				return 0;
			}
			cfg->name = name;
		}
	}
	return 1;
}

//
int traceUnitAnalyze(char* pointer, int parType, char* boundary, char* unitTraceName) {
	pointer = strtok(NULL, boundary);
	if (!pointer) {
		printf("Couldnt analyze traceunit name!\n");
		return 0;
	}
	strcpy(unitTraceName, pointer);
	return 1;
}
//
int findIntegerInString(char* str) {
	char* pointer = str;
	while (*pointer) {
		if (isdigit(*pointer)) {
			int result = atoi(pointer);
			return result;
		}
		else {
			pointer++;
		}
	}
	return -1;
}
//3
int findUnitTraceName(char* str) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		if (strcmp(str, unitsTypeNames[i]) == 0) {
			return i;
		}
	}
	return -1;
}

//inst
//
Instruction* initInstruction() {
	Instruction* inst = malloc(sizeof(Instruction));
	if (!inst) {
		return NULL;
	}
	inst->empty = Yes;
	inst->fileWriting = No;
	inst->issue = Yes;
	for (int i = 0; i < NUM_OF_OPERATIONS_STAGES; i++) {
		inst->clockCyclesOperation[i] = -1;
	}
	inst->result = inst->executionCycles = inst->operation = inst->index = inst->queueIndex = inst->status = -1;
	inst->fetchCycles = 0;
	return inst;
}
//
void freeInstruction(Instruction* inst) {
	if (inst) {
		free(inst);
	}
}
//
void analyzeInstruction(Instruction* inst, int command) {
	inst->command = command;
	inst->opcode = 0xF & ((command << 4) >> 28);
	inst->dst = 0xF & ((command << 8) >> 28);
	inst->src0 = 0xF & ((command << 12) >> 28);
	inst->src1 = 0xF & ((command << 16) >> 28);
	inst->imm = 0xF & ((command << 20) >> 20);
	inst->empty = No;
	if (inst->opcode != HALT) {
		if (inst->opcode != OP_LD) {
			inst->operation = inst->opcode;
		}
		else {
			inst->operation = inst->opcode;
		}
	}
	else {
		inst->operation = -1;
	}
}

//
InstQueue* initInstQueue() {
	InstQueue* instQueue = malloc(sizeof(InstQueue));
	if (!instQueue) {
		printf("Error! Memory allocation failure.\n");
		return NULL;
	}
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		instQueue->queue[i] = initInstruction();
		if (!instQueue->queue[i]) {
			printf("Error! Memory allocation failure.\n");
			for (int j = i; j >= 0; j--) {
				freeInstruction(instQueue->queue[j]);
			}
			free(instQueue);
			return NULL;
		}
	}
	instQueue->fullQueue = No;
	instQueue->emptyQueue = Yes;
	return instQueue;
}
//
void freeInstQueue() {
	if (instructionQueue) {
		for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
			freeInstruction(instructionQueue->queue[i]);
		}
	}
}

int enqueueInstQueue(InstQueue* instQueue, Instruction* inst) {
	if (inst->operation == -1) {
		return -1;
	}
	if (instQueue->fullQueue) {
		return -1;
	}
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (instQueue->queue[i]->empty) {
			instQueue->queue[i] = inst;
			emptyFullInstQueue(instQueue);
			return i;
		}
	}
	return -1;
}
//
int dequeueInstQueue(InstQueue* instQueue, int instIndex) {
	if (instQueue->emptyQueue) {
		return 0;
	}
	instQueue->queue[instIndex] = initInstruction();
	emptyFullInstQueue(instQueue);
	return 1;
}
//
void emptyFullInstQueue(InstQueue* instQueue) {
	int instNum = 0;
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (instQueue->queue[i]->empty != Yes) {
			instNum++;
		}
	}
	if (instNum == NUM_OF_INSTRUCTION_QUEUE) {
		instQueue->fullQueue = Yes;
		instQueue->emptyQueue = No;
	}
	else if (instNum == 0) {
		instQueue->emptyQueue = Yes;
		instQueue->fullQueue = No;
	}
	else {
		instQueue->fullQueue = No;
		instQueue->emptyQueue = No;
	}
}

//units

//
Unit* initUnit(unitType type, int num) {
	Unit* unit = (Unit*)malloc(sizeof(Unit));
	if (!unit) {
		printf("Error! Memory allocation failure!\n");
		return 0;
	}
	unit->type = type;
	unit->unitNum = num;
	unit->empty = Yes;
	unit->writeResult = No;
	unit->busy = No;
	unit->op = unit->f_i = unit->f_j = unit->f_k = unit->q_j_type = unit->q_k_type = unit->q_j_index = unit->q_k_index = unit->r_j = unit->r_k = -1;
	return unit;
}
//
void freeUnit(Unit* unit) {
	if (!unit) {
		return;
	}
	free(unit);
}
//
Units* initUnits(int numOfUnits, int delay, unitType type) {
	Units* units = malloc(sizeof(Units));
	if (units == NULL) {
		printf("Error! Memory allocation failure!\n");
		return NULL;
	}

	Unit** unitsArray = malloc(numOfUnits * sizeof(Unit*));
	if (unitsArray == NULL) {
		printf("Error! Memory allocation failure!\n");
		return NULL;
	}

	for (int i = 0; i < numOfUnits; i++) {
		Unit* unit = initUnit(type, i);
		units->units[i] = unit;
		if (units->units[i] == NULL) {
			printf("Error! Memory allocation failure!\n");
			for (int j = i; j > -1; j--) {
				freeUnit(units->units[j]);
			}
			return NULL;
		}
	}
	units->totalUnitsNum = numOfUnits;
	units->activeUnitsNum = 0;
	units->delay = delay;
	units->type = type;
	units->canEnter = Yes;
	return units;
}
//
void freeUnits(Units* units) {
	if (units == NULL) {
		return;
	}
	for (int i = 0; i < MAX_NUM_OF_ACTIVE_UNITS; i++) {
		if (units->activeUnitsNum != 0) {
			freeUnit(units->units[i]);
			units->activeUnitsNum--;
		}
	}
	free(units);
}
//
ActiveUnit* initActiveUnit(Configuration* config) {
	ActiveUnit* activeUnit = (ActiveUnit*)malloc(sizeof(ActiveUnit));
	if (!activeUnit) {
		printf("Error! Memory allocation failure!\n");
		return 0;
	}
	for (int i = 0; i < UNITS_NUMBER; i++) {
		activeUnit->activeUnit[i] = initUnits(config->units[i], config->delays[i], i);
		if (!activeUnit->activeUnit[i]) {
			printf("Error! Memory allocation failure!\n");
			for (int j = i; j > -1; j--) {
				freeUnits(activeUnit->activeUnit[j]);
			}
			return 0;
		}
	}
	activeUnit->unitNum = config->unitNum;
	activeUnit->unitName = config->name;
	return activeUnit;
}
//
void freeActiveUnit() {
	if (activeUnit == NULL) {
		return;
	}
	for (int i = 0; i < UNITS_NUMBER; i++) {
		freeUnits(activeUnit->activeUnit[i]);
	}
	free(activeUnit);
}

//utilities

//
int openFiles(FILE** filesArray, char** filesPaths) {
	for (int i = 0; i < FILES_NUMBER; i++) {
		char* mode = "r";
		if (i > 1) {
			mode = "w";
		}
		FILE* file = fopen(filesPaths[i], mode);
		if (!file) {
			printf("Error! could'nt open %s!\n", filesPaths[i]);
			for (int j = 0; j < i; j++) {
				fclose(filesArray[j]);
			}
			return 0;
		}
		filesArray[i] = file;
	}
	return 1;
}
//
void printMemoutFile(FILE* file, int* memory, int maxNumLines) {
	for (int i = 0; i < maxNumLines; i++) {
		fprintf(file, "%.8x\n", memory[i]);
	}
}
//
void printRegoutFile(FILE* file, double* regs) {
	int i;
	for (i = 0; i < REGISTERS_NUMBER; i++) {
		fprintf(file, "%f\n", regs[i]);
	}
}
//
void printTraceunitFile(FILE* file, ActiveUnit* activeUnit, int* resultTypes, int* resultIndexes, int clockCycle) {
	int unitBusy = No;
	for (int i = 0; i < REGISTERS_NUMBER; i++) {
		if (resultTypes[i] == activeUnit->unitName && resultIndexes[i] == activeUnit->unitNum) {
			unitBusy = Yes;
		}
	}
	if (unitBusy) {
		fprintf(file, "%d ", clockCycle);
		fprintf(file, "%s%d ", unitsTypeNames[activeUnit->unitName], activeUnit->unitNum);

		fprintf(file, "F%d ", activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->f_i);
		fprintf(file, "F%d ", activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->f_j);
		fprintf(file, "F%d ", activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->f_k);

		int q_j = (activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_j_type == -1) ? 0 : 1;
		int q_k = (activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_k_type == -1) ? 0 : 1;

		if (q_j) {
			fprintf(file, "%s%d ", unitsTypeNames[activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_j_type], activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_j_index);
		}
		else {
			fprintf(file, "- ");
		}
		if (q_k) {
			fprintf(file, "%s%d ", unitsTypeNames[activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_k_type], activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->q_k_index);
		}
		else {
			fprintf(file, "- ");
		}

		fprintf(file, (activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->r_j) ? "Yes " : "No ");
		fprintf(file, (activeUnit->activeUnit[activeUnit->unitName]->units[activeUnit->unitNum]->r_k) ? "Yes\n" : "No\n");
	}
}
//
int hexCommand(Instruction* inst) {
	unsigned int hex = 0;
	hex += inst->opcode << 24;
	hex += inst->dst << 20;
	hex += inst->src0 << 16;
	hex += inst->src1 << 12;
	hex += 0xFFFFF & inst->imm;
	return hex;
}
//
int unitsCompare(ActiveUnit* activeUnit, int q_type, int q_index, Unit* unit, int j) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		if (i == q_type) {
			for (int j = 0; j < activeUnit->activeUnit[i]->totalUnitsNum; j++) {
				if (j == q_index) {
					if (activeUnit->activeUnit[i]->units[j] == unit) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
//
float singlePrecisionToFloat(unsigned long singlePrecision) {
	unsigned long sign, exp, fractionBits, fractionB;
	int i = 0;
	float result, fraction = 1.0;
	if (singlePrecision == 0) {
		return 0.0;
	}
	sign = singlePrecision >> 31;
	exp = singlePrecision << 1 >> 24;
	fractionBits = singlePrecision << 9 >> 9;
	for (i = 0; i < 23; i++) {
		fractionB = fractionBits << (i + 9);
		fractionB = fractionB >> 31;
		fraction += fractionB * powf(2, -1 * (i + 1));
	}
	result = powf(-1, sign) * powf(2, exp - 127) * fraction;

	return result;
}
//
int floatToSinglePrecision(float f) {
	int exp, fraction, expLenBits, i = 0, fractionToBit = 0;
	int floatInInt = (int)floor(f);
	int result;

	float rat = f - floatInInt;
	int sign = f > 0.0 ? 0 : 1;

	expLenBits = floor(log2(floatInInt));
	exp = expLenBits + 127;
	fraction = floatInInt << (23 - expLenBits);
	for (i = 0; i < (23 - expLenBits); i++) {
		rat = rat * 2;
		fractionToBit <<= 1;
		if (1 <= rat) {
			fractionToBit++;
			rat--;
		}
	}
	fractionToBit >>= 1;
	fraction = fraction | fractionToBit;
	fraction = fraction & 0x7FFFFF;
	result = (((sign << 8) + exp) << 23) + fraction;

	return result;
}
//
void writeToFiles(FILE* file, ActiveUnit* activeUnit, InstQueue* instQueue) {
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (!instQueue->queue[i]->empty && instQueue->queue[i]->clockCyclesOperation[WRITE_RESULT] > 0) {
			int type = instQueue->queue[i]->operation, index = instQueue->queue[i]->index;
			insertPrintUnit(activeUnit->activeUnit[type]->units[index]);
			activeUnit->activeUnit[type]->units[index]->instruction->empty = Yes;
			activeUnit->activeUnit[type]->units[index] = initUnit(type, index);
			activeUnit->activeUnit[type]->units[index]->empty = Yes;
			activeUnit->activeUnit[type]->activeUnitsNum--;
		}
	}
}
//
void printUnitsToTraceInstFile(FILE* file) {
	sort();
	int listLength = length();
	for (int i = 0; i < listLength; i++) {
		printTracinstFile(file, head->op, head->unitType, head->unitIndex, head->fetchCC, head->issueCC, head->readCC, head->exeCC, head->writeCC);
		deletePrintUnit();
	}
}
//
void printTracinstFile(FILE* file, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write) {
	fseek(file, 0, SEEK_END);
	fprintf(file, "%.8x %d %s%d %d %d %d %d\n", opLine, fetch, unitsTypeNames[type], index, issue, read, exe, write);
}
//
PrintUnit* initPrintUnit() {
	PrintUnit* printUnit = (PrintUnit*)malloc(sizeof(PrintUnit));
	if (!printUnit) {
		return NULL;
	}
	printUnit->op = printUnit->unitType = printUnit->unitIndex = printUnit->fetchCC = printUnit->issueCC = printUnit->readCC = printUnit->exeCC = printUnit->writeCC = -1;
	return printUnit;
}//
void insertPrintUnit(Unit* unit) {
	// create a link
	PrintUnit* printUnit = malloc(sizeof(PrintUnit));
	if (printUnit == NULL) {
		return;
	}
	printUnit->op = unit->instruction->command;
	printUnit->unitType = unit->type;
	printUnit->unitIndex = unit->unitNum;
	printUnit->fetchCC = unit->instruction->fetchCycles;
	printUnit->issueCC = unit->instruction->clockCyclesOperation[ISSUE];
	printUnit->readCC = unit->instruction->clockCyclesOperation[READ_OPERAND];
	printUnit->exeCC = unit->instruction->clockCyclesOperation[EXECUTION];
	printUnit->writeCC = unit->instruction->clockCyclesOperation[WRITE_RESULT];

	printUnit->next = head;

	head = printUnit;
}
//
PrintUnit* deletePrintUnit() {

	PrintUnit* tempLink = head;
	head = head->next;
	return tempLink;
}
//
int empty() {
	return head == NULL;
}
//
int length() {
	int length = 0;
	PrintUnit* current;

	for (current = head; current != NULL; current = current->next) {
		length++;
	}

	return length;
}
//
void sort() {

	int i, j, k;
	PrintUnit* current;
	PrintUnit* next;
	PrintUnit* temp = initPrintUnit();

	int size = length();
	k = size;

	for (i = 0; i < size - 1; i++, k--) {
		current = head;
		next = head->next;

		for (j = 1; j < k; j++) {

			if (current->fetchCC > next->fetchCC) {
				updatePrintUnit(current, temp);
				updatePrintUnit(next, current);
				updatePrintUnit(temp, next);
			}

			current = current->next;
			next = next->next;
		}
	}
}
//
void updatePrintUnit(PrintUnit* printUnitSource, PrintUnit* printUnitDestenation) {
	printUnitDestenation->op = printUnitSource->op;
	printUnitDestenation->unitType = printUnitSource->unitType;
	printUnitDestenation->unitIndex = printUnitSource->unitIndex;
	printUnitDestenation->fetchCC = printUnitSource->fetchCC;
	printUnitDestenation->issueCC = printUnitSource->issueCC;
	printUnitDestenation->readCC = printUnitSource->readCC;
	printUnitDestenation->exeCC = printUnitSource->exeCC;
	printUnitDestenation->writeCC = printUnitSource->writeCC;

}

double* getDoubleZeros(int size) {
	double* ret = malloc(size * sizeof(double));
	for (int i = 0; i < size; i++) {
		ret[i] = 0.0;
	}
	return ret;
}
int* getIntZeros(int size) {
	int* ret = malloc(size * sizeof(int));
	for (int i = 0; i < size; i++) {
		ret[i] = 0;
	}
	return ret;
}
//simulator

int simulator(char** filesPaths) {

	regs = getDoubleZeros(REGISTERS_NUMBER);
	memory = getIntZeros(MEMORY_LENGTH);
	unsigned int pc = 0;
	unsigned int operation = 0;
	unsigned int lineNumber = 0;
	int linesNumber;
	int instructionNumber = 0;
	int instructionIndex = -1;
	int tempBussyInstructionsNumber = bussyUnitsNumber;
	int clockCycles = -1;
	int fetchPossible = Yes;
	int runPossible = Yes;
	int stopSet = No;
	int resTypes[REGISTERS_NUMBER];
	int resIndexes[REGISTERS_NUMBER];
	char* line = calloc(1, sizeof(char) * MAX_LINE_LENGTH);

	if (!line) {
		freeSimulator(0);
		return 0;
	}
	for (int i = 0; i < REGISTERS_NUMBER; i++) {
		resTypes[i] = -1;
		resIndexes[i] = -1;
	}
	

	instructionQueue = initInstQueue();
	if (!instructionQueue) {
		freeSimulator( line);
		return 0;
	}
	if (!openFiles(filesArray, filesPaths)) {
		return 0;
	}
	if ((linesNumber = initMemory(filesArray[MEMIN], line, memory)) == 0) {
		freeSimulator(line);
		return 0;
	}
	initRegs(regs);
	if ((cfg = analyzeConfiguration(filesArray[CONFIG], line)) == 0) {
		freeSimulator(line);
		return 0;
	}
	if ((activeUnit = initActiveUnit(cfg)) == 0) {
		freeSimulator(line);
		return 0;
	}
	printf("blup\n");
	while (1) {
		clockCycles++;
		printTraceunitFile(filesArray[TRACEUNIT], activeUnit, resTypes, resIndexes, clockCycles);
		Instruction* instruction = initInstruction();
		if (!instruction) {
			freeSimulator(line);
			return 0;
		}
		if (!stopSet) {
			analyzeInstruction(instruction, memory[instructionNumber]);
			if (instruction->opcode == HALT) {
				stopSet = Yes;
				runPossible = No;
				for (int i = 0; i < REGISTERS_NUMBER; i++) {
					if (resTypes[i] != -1) {
						runPossible = Yes;
					}
				}
			}
			instructionIndex = enqueueInstQueue(instructionQueue, instruction);
			if (instructionIndex != -1) {
				instructionNumber++;
				instruction->fetchCycles = clockCycles;
			}
			if (clockCycles == 0) {
				continue;
			}
			tempBussyInstructionsNumber = issue(activeUnit, instructionQueue, resTypes, resIndexes, clockCycles);
			if (tempBussyInstructionsNumber == 0) {
				dequeueInstQueue(instructionQueue, instructionIndex);
				instructionNumber--;
				instruction->fetchCycles = -1;
			}
			else {
				bussyUnitsNumber += tempBussyInstructionsNumber;
			}
		}
		else {
			runPossible = No;
			for (int i = 0; i < REGISTERS_NUMBER; i++) {
				if (resTypes[i] != -1) {
					runPossible = Yes;
				}
			}
			if (!runPossible && bussyUnitsNumber == 0) {
				clockCycles--;
				break;
			}
			if (bussyUnitsNumber > 0) {
				bussyUnitsNumber += issue(activeUnit, instructionQueue, resTypes, resIndexes, clockCycles);
			}

		}
		performCommand(activeUnit, regs, clockCycles, memory, filesArray, instructionQueue, resTypes, resIndexes);
	}

	finalize(filesArray, memory, regs, line, cfg, activeUnit, instructionQueue);
	return 0;
}

void performCommand(ActiveUnit* activelUnit, double  regs[16], int clockCycles, int  memory[4096], FILE* filesArray[6], InstQueue* instructionQueue, int  resTypes[16], int  resIndexes[16])
{
	readOp(activelUnit, regs, clockCycles);
	executionOp(activelUnit, memory, regs, clockCycles);
	writeResult(filesArray, activelUnit, instructionQueue, memory, resTypes, resIndexes, regs, clockCycles);

	writeToFiles(filesArray[TRACEINST], activelUnit, instructionQueue);
}

void finalize(FILE* filesArray[6], int * memory, double * regs, char* line, Configuration* cfg, ActiveUnit* activelUnit, InstQueue* instructionQueue)
{
	printUnitsToTraceInstFile(filesArray[TRACEINST]);

	printMemoutFile(filesArray[MEMOUT], memory, MEMORY_LENGTH);

	printRegoutFile(filesArray[REGOUT], regs);

	freeSimulator(line);
	free(regs);
	free(memory);
}

int initMemory(FILE* meminFile, char* line, int* memory) {
	int linesNumber = 0;
	int endLine = 0;
	char currentLine[MAX_LINE_LENGTH];
	while (fgets(line, MAX_LINE_LENGTH, meminFile) != 0) {
		if (sscanf(line, "%s", currentLine)) {
			if (strcmp(currentLine, "0x00000000") != 0) {
				endLine = linesNumber;
			}
			if (sscanf(currentLine, "%x", &memory[linesNumber]))
				linesNumber++;
			else return 0;

		}
		else return 0;
	}
	return endLine;
}

void initRegs(double* registers) {
	for (int i = 0; i < REGISTERS_NUMBER; i++) {
		registers[i] = i / 1.0;
	}
}

int issue(ActiveUnit* activeUnit, InstQueue* queue, int* resTypes, int* resIndexes, int clockCycles) {
	int instUnitIndex = -1;
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		instUnitIndex += instructionToUnit(activeUnit, queue->queue[i], resTypes, resIndexes, clockCycles);
		if (instUnitIndex == 0) {
			return 1;
		}
	}
	return 0;
}

int instructionToUnit(ActiveUnit* activeUnit, Instruction* instruction, int* resTypes, int* resIndexes, int clockCycles) {
	if (!instruction || instruction->empty == Yes) {
		return 0;
	}
	int type = instruction->operation;
	/*Halt instruction*/
	if (type < 0 || type > 5) {
		return 0;
	}
	if (activeUnit->activeUnit[type]->canEnter) {
		for (int i = 0; i < activeUnit->activeUnit[type]->totalUnitsNum; i++) {
			if (!activeUnit->activeUnit[type]->units[i] || activeUnit->activeUnit[type]->units[i]->empty) {
				if (!activeUnit->activeUnit[type]->units[i]->busy && instruction->fetchCycles != clockCycles && instruction->clockCyclesOperation[ISSUE] == -1) {
					if ((resTypes[instruction->dst] == -1 && type != UNIT_ST) || (type == UNIT_ST)) {
						setUnitFields(activeUnit, type, i, instruction, resIndexes, resTypes, clockCycles);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

void setUnitFields(ActiveUnit* activeUnit, int type, int i, Instruction* instruction, int* resIndexes, int* resTypes, int clockCycles)
{
	activeUnit->activeUnit[type]->units[i]->busy = Yes;
	activeUnit->activeUnit[type]->units[i]->instruction = instruction;
	activeUnit->activeUnit[type]->units[i]->op = instruction->opcode;

	activeUnit->activeUnit[type]->units[i]->f_i = instruction->dst;
	activeUnit->activeUnit[type]->units[i]->f_j = instruction->src0;
	activeUnit->activeUnit[type]->units[i]->f_k = instruction->src1;

	activeUnit->activeUnit[type]->units[i]->q_j_index = resIndexes[instruction->src0];
	activeUnit->activeUnit[type]->units[i]->q_k_index = resIndexes[instruction->src1];

	activeUnit->activeUnit[type]->units[i]->q_j_type = resTypes[instruction->src0];
	activeUnit->activeUnit[type]->units[i]->q_k_type = resTypes[instruction->src1];

	activeUnit->activeUnit[type]->units[i]->r_j = (resTypes[instruction->src0] == -1) ? Yes : No;
	activeUnit->activeUnit[type]->units[i]->r_k = (resTypes[instruction->src1] == -1) ? Yes : No;

	activeUnit->activeUnit[type]->activeUnitsNum++;
	if (activeUnit->activeUnit[type]->activeUnitsNum == activeUnit->activeUnit[type]->totalUnitsNum) {
		activeUnit->activeUnit[type]->canEnter = No;
	}
	activeUnit->activeUnit[type]->units[i]->empty = No;

	activeUnit->activeUnit[type]->units[i]->instruction->clockCyclesOperation[ISSUE] = clockCycles;

	if (activeUnit->activeUnit[type]->units[i]->type != UNIT_ST) {
		resTypes[instruction->dst] = activeUnit->activeUnit[type]->units[i]->type;
		resIndexes[instruction->dst] = activeUnit->activeUnit[type]->units[i]->unitNum;
	}

	activeUnit->activeUnit[type]->units[i]->instruction->index = i;
}

void readOp(ActiveUnit* activeUnit, double* registers, int clockCycles) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		for (int j = 0; j < activeUnit->activeUnit[i]->totalUnitsNum; j++) {
			if (!activeUnit->activeUnit[i]->units[j]->empty && !activeUnit->activeUnit[i]->units[j]->instruction->empty) {
				if (activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[ISSUE] < clockCycles) {
					if (activeUnit->activeUnit[i]->units[j]->r_j == Yes && activeUnit->activeUnit[i]->units[j]->r_k == Yes) {
						if (activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] == -1) {
							activeUnit->activeUnit[i]->units[j]->f_j_value = registers[activeUnit->activeUnit[i]->units[j]->f_j];
							activeUnit->activeUnit[i]->units[j]->f_k_value = registers[activeUnit->activeUnit[i]->units[j]->f_k];

							activeUnit->activeUnit[i]->units[j]->r_j = No;
							activeUnit->activeUnit[i]->units[j]->r_k = No;
							activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] = clockCycles;
							if (activeUnit->activeUnit[i]->delay == 1) {
								activeUnit->activeUnit[i]->units[j]->instruction->executionCycles = clockCycles + activeUnit->activeUnit[i]->delay;
							}
							else {
								activeUnit->activeUnit[i]->units[j]->instruction->executionCycles = clockCycles + activeUnit->activeUnit[i]->delay - 1;
							}
						}
					}
				}
			}
		}
	}
}

void executionOp(ActiveUnit* activeUnit, int* memory, double* registers, int clockCycles) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		for (int j = 0; j < activeUnit->activeUnit[i]->totalUnitsNum; j++) {
			if (!activeUnit->activeUnit[i]->units[j]->empty) {
				if (activeUnit->activeUnit[i]->units[j]->instruction->executionCycles >= 0 &&
					0 < activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] &&
					activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] < clockCycles) {

					if (activeUnit->activeUnit[i]->units[j]->instruction->executionCycles == clockCycles) {
						if (activeUnit->activeUnit[i]->units[j]->instruction->result == -1) {
							executionInst(activeUnit, memory, registers, activeUnit->activeUnit[i]->units[j]->instruction->opcode, i, j, clockCycles);

						}
						activeUnit->activeUnit[i]->units[j]->instruction->clockCyclesOperation[EXECUTION] = clockCycles;
					}
				}
			}
		}
	}
}

void executionInst(ActiveUnit* activeUnit, int* memory, double* registers, unsigned int opcode, int i, int j, int clockCycles) {
	switch (opcode) {
	case OP_ADD:
		activeUnit->activeUnit[i]->units[j]->instruction->result = activeUnit->activeUnit[i]->units[j]->f_j_value + activeUnit->activeUnit[i]->units[j]->f_k_value;
		break;
	case OP_SUB:
		activeUnit->activeUnit[i]->units[j]->instruction->result = activeUnit->activeUnit[i]->units[j]->f_j_value - activeUnit->activeUnit[i]->units[j]->f_k_value;
		break;
	case OP_MULT:
		activeUnit->activeUnit[i]->units[j]->instruction->result = activeUnit->activeUnit[i]->units[j]->f_j_value * activeUnit->activeUnit[i]->units[j]->f_k_value;
		break;
	case OP_DIV:
		activeUnit->activeUnit[i]->units[j]->instruction->result = activeUnit->activeUnit[i]->units[j]->f_j_value / activeUnit->activeUnit[i]->units[j]->f_k_value;
		break;
	case OP_LD:
		activeUnit->activeUnit[i]->units[j]->instruction->result = singlePrecisionToFloat(memory[activeUnit->activeUnit[i]->units[j]->instruction->imm]);
		break;
	case OP_ST:
		activeUnit->activeUnit[i]->units[j]->instruction->result = activeUnit->activeUnit[i]->units[j]->f_k_value;
		checkLdSt(activeUnit, activeUnit->activeUnit[i]->units[j]->instruction, clockCycles);
		break;
	}
}

void checkLdSt(ActiveUnit* activeUnit, Instruction* stInst, int clockCycles) {
	for (int i = 0; i < activeUnit->activeUnit[LD_UNIT]->totalUnitsNum; i++) {
		if (!activeUnit->activeUnit[LD_UNIT]->units[i]->empty) {
			if (activeUnit->activeUnit[LD_UNIT]->units[i]->instruction->imm == stInst->imm) {
				if (activeUnit->activeUnit[LD_UNIT]->units[i]->instruction->clockCyclesOperation[ISSUE] < stInst->clockCyclesOperation[ISSUE]) {
					if (activeUnit->activeUnit[LD_UNIT]->units[i]->instruction->clockCyclesOperation[EXECUTION] <= clockCycles &&
						stInst->executionCycles == clockCycles) {
						stInst->executionCycles++;
					}
				}
			}
		}
	}
}

void writeResult(FILE** file, ActiveUnit* activeUnit, InstQueue* instQueue, int* memory, int* resTypes, int* resIndexes, double* registers, int clockCycles) {
	int writeResultTypes = 0, unitsNum;
	for (int x = 0; x < UNITS_NUMBER; x++) {
		unitsNum = 0;
		for (int y = 0; y < activeUnit->activeUnit[x]->totalUnitsNum; y++) {
			if (activeUnit->activeUnit[x]->units[y]->empty) {
				continue;
			}
			writeResultActiveUnit(file, activeUnit, activeUnit->activeUnit[x]->units[y], memory, resTypes, resIndexes, registers, clockCycles);
		}
	}
}

void writeResultActiveUnit(FILE** file, ActiveUnit* activeUnit, Unit* unit, int* memory, int* resTypes, int* resIndexes, double* registers, int clockCycles) {
	int writeResultTypes = 0;
	if (!unit->writeResult) {
		for (int x = 0; x < UNITS_NUMBER; x++) {
			for (int y = 0; y < activeUnit->activeUnit[x]->totalUnitsNum; y++) {
				if (activeUnit->activeUnit[x]->units[y]->empty) {
					continue;
				}
				if ((activeUnit->activeUnit[x]->units[y]->f_j != unit->f_i || activeUnit->activeUnit[x]->units[y]->r_j == No) &&
					(activeUnit->activeUnit[x]->units[y]->f_k != unit->f_i || activeUnit->activeUnit[x]->units[y]->r_k == No)) {
					writeResultTypes++;
				}
			}
		}
		if (writeResultTypes != bussyUnitsNumber || bussyUnitsNumber == 0) {
			return;
		}
		else {
			unit->writeResult = Yes;
		}
	}
	else {
		writeResultTypes = 0;
		for (int x = 0; x < UNITS_NUMBER; x++) {
			for (int y = 0; y < activeUnit->activeUnit[x]->totalUnitsNum; y++) {
				if (activeUnit->activeUnit[x]->units[y]->empty) {
					continue;
				}
				if (0 < unit->instruction->clockCyclesOperation[EXECUTION] && unit->instruction->clockCyclesOperation[EXECUTION] < clockCycles) {
					if (unitsCompare(activeUnit, activeUnit->activeUnit[x]->units[y]->q_j_type, activeUnit->activeUnit[x]->units[y]->q_j_index, unit, 1)) {
						activeUnit->activeUnit[x]->units[y]->q_j_index = -1;
						activeUnit->activeUnit[x]->units[y]->q_j_type = -1;
						activeUnit->activeUnit[x]->units[y]->r_j = Yes;
					}
					else if (unitsCompare(activeUnit, activeUnit->activeUnit[x]->units[y]->q_k_type, activeUnit->activeUnit[x]->units[y]->q_k_index, unit, 0)) {
						activeUnit->activeUnit[x]->units[y]->q_k_index = -1;
						activeUnit->activeUnit[x]->units[y]->q_k_type = -1;
						activeUnit->activeUnit[x]->units[y]->r_k = Yes;
					}
				}
			}
		}
		if (0 < unit->instruction->clockCyclesOperation[EXECUTION] && unit->instruction->clockCyclesOperation[EXECUTION] < clockCycles) {
			switch (unit->type) {
			case UNIT_ST:
				memory[unit->instruction->imm] = floatToSinglePrecision(unit->instruction->result);
				break;
			case UNIT_LD:
				registers[unit->f_i] = unit->instruction->result;
				break;
			default:
				registers[unit->f_i] = unit->instruction->result;
				break;
			}
			bussyUnitsNumber--;
			unit->instruction->clockCyclesOperation[WRITE_RESULT] = clockCycles;
			resTypes[unit->f_i] = -1;
			resIndexes[unit->f_i] = -1;
			unit->busy = No;
		}
	}
}

void freeSimulator(char* line) {
	for (int i = 0; i < FILES_NUMBER; i++) {
		fclose(filesArray[i]);
	}
	if (line) { free(line); }
	freeConfiguration();
	freeActiveUnit();
	freeInstQueue();
}