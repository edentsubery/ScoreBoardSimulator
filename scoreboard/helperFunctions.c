#include "helperFunctions.h"
/*initialization of config data structure*/
Configuration* initConfiguration() {
	Configuration* config = (Configuration*)malloc(sizeof(Configuration));
	if (!config) {
		return 0;
	}
	config->name = -1;
	return config;
}

void freeConfiguration() {
	if (!cfg) {
		return;
	}
	free(cfg);
}
/* returns the cfg address*/
Configuration* analyzeConfiguration(FILE* cfgFile) {
	Configuration* config = initConfiguration(); /*initialize data structure*/
	if (!config) {
		return 0;
	}
	int linesCounter = 0;
	size_t length = 0;
	ssize_t linesRead = 0;

	while (fgets(line, MAX_LINE_LENGTH, cfgFile) != 0) {
		if (!analyzeConfig(config)) { /*read each line*/
			return 0;
		}
	}
	return config;
}

int analyzePar(char* pointer, int parType, char* boundary) {
	pointer = strtok(NULL, boundary);
	int num = atoi(pointer);
	return num;
}
/*analize each line in the cfg.txt file to set configurations of the "processor"*/
int analyzeConfig(Configuration* cfg) {
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

/*analize the trace_unit line to know which unit should appear in the traceunit.txt*/
int traceUnitAnalyze(char* pointer, int parType, char* boundary, char* unitTraceName) {
	pointer = strtok(NULL, boundary);
	if (!pointer) {
		printf("Couldnt analyze traceunit name!\n");
		return 0;
	}
	strcpy(unitTraceName, pointer);
	return 1;
}

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
/*read different fields in the instruction according to instruction format*/
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

void freeInstQueue() {
	if (instructionQueue) {
		for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
			freeInstruction(instructionQueue->queue[i]);
		}
	}
}
/*insert inst to instruction queue and return the index of where it was inserted*/
int insertInstToQueue(Instruction* inst) {
	if (inst->operation == -1) {
		return -1;
	}
	if (instructionQueue->fullQueue) {
		return -1;
	}
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (instructionQueue->queue[i]->empty) {
			instructionQueue->queue[i] = inst;
			setQueueFullEmptyAttr(); /*update full and empty attributes of the queue*/
			return i;
		}
	}
	return -1;
}
/*remove instruction by index from the instruction queue*/
int removeInstAtIdxFromQueue(int instIndex) {
	if (instructionQueue->emptyQueue) {
		return 0;
	}
	instructionQueue->queue[instIndex] = initInstruction();
	setQueueFullEmptyAttr();
	return 1;
}
/*update instruction queue attributes (full or empty)*/
void setQueueFullEmptyAttr() {
	int instNum = 0;
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (instructionQueue->queue[i]->empty != Yes) {
			instNum++;
		}
	}
	if (instNum == NUM_OF_INSTRUCTION_QUEUE) {
		instructionQueue->fullQueue = Yes;
		instructionQueue->emptyQueue = No;
	}
	else if (instNum == 0) {
		instructionQueue->emptyQueue = Yes;
		instructionQueue->fullQueue = No;
	}
	else {
		instructionQueue->fullQueue = No;
		instructionQueue->emptyQueue = No;
	}
}

/*initialize a unit only by type and size according to cfg.txt*/
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
	unit->op = unit->f_i = unit->f_j = unit->f_k = unit->q_j_type = unit->q_k_type = unit->q_j_index = unit->q_k_index = unit->r_j = unit->r_k = -1; /*indicates empty unit*/
	return unit;
}

void freeUnit(Unit* unit) {
	if (!unit) {
		return;
	}
	free(unit);
}

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
	units->functionalUnitsNum = 0;
	units->delay = delay;
	units->type = type;
	units->canEnter = Yes;
	return units;
}

void freeUnits(Units* units) {
	if (units == NULL) {
		return;
	}
	for (int i = 0; i < MAX_NUM_OF_ACTIVE_UNITS; i++) {
		if (units->functionalUnitsNum != 0) {
			freeUnit(units->units[i]);
			units->functionalUnitsNum--;
		}
	}
	free(units);
}
/*initialize all functional units by requirements in cfg.txt*/
functionalUnit* initFunctionalUnits(Configuration* config) {
	functionalUnit* functionalUnits = (functionalUnit*)malloc(sizeof(functionalUnit));
	if (!functionalUnits) {
		printf("Error! Memory allocation failure!\n");
		return 0;
	}
	for (int i = 0; i < UNITS_NUMBER; i++) {
		functionalUnits->functionalUnit[i] = initUnits(config->units[i], config->delays[i], i);
		if (!functionalUnits->functionalUnit[i]) {
			printf("Error! Memory allocation failure!\n");
			for (int j = i; j > -1; j--) {
				freeUnits(functionalUnits->functionalUnit[j]);
			}
			return 0;
		}
	}
	functionalUnits->unitNum = config->unitNum;
	functionalUnits->unitName = config->name;
	return functionalUnits;
}

void freeFunctionalUnit() {
	if (functionalUnits == NULL) {
		return;
	}
	for (int i = 0; i < UNITS_NUMBER; i++) {
		freeUnits(functionalUnits->functionalUnit[i]);
	}
	free(functionalUnits);
}
/*create file descriptors for input output files*/
int openFiles(char** filesPaths) {
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
/*print memout.txt according to format*/
void printMemoutFile(FILE* file, int maxNumLines) {
	for (int i = 0; i < maxNumLines; i++) {
		fprintf(file, "%.8x\n", memory[i]);
	}
}
/*print regout.txt according to format*/
void printRegoutFile(FILE* file) {
	int i;
	for (i = 0; i < REGISTERS_NUMBER; i++) {
		fprintf(file, "%f\n", regs[i]);
	}
}
/*print traceunit.txt according to format*/
void printTraceunitFile(FILE* file, int clockCycle) {
	int unitBusy = No;
	for (int i = 0; i < REGISTERS_NUMBER; i++) {
		if (resTypes[i] == functionalUnits->unitName && resIndexes[i] == functionalUnits->unitNum) {
			unitBusy = Yes;
		}
	}
	if (unitBusy) {
		fprintf(file, "%d ", clockCycle);
		fprintf(file, "%s%d ", unitsTypeNames[functionalUnits->unitName], functionalUnits->unitNum);

		fprintf(file, "F%d ", functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->f_i);
		fprintf(file, "F%d ", functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->f_j);
		fprintf(file, "F%d ", functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->f_k);

		int q_j = (functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_j_type == -1) ? 0 : 1;
		int q_k = (functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_k_type == -1) ? 0 : 1;

		if (q_j) {
			fprintf(file, "%s%d ", unitsTypeNames[functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_j_type], functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_j_index);
		}
		else {
			fprintf(file, "- ");
		}
		if (q_k) {
			fprintf(file, "%s%d ", unitsTypeNames[functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_k_type], functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->q_k_index);
		}
		else {
			fprintf(file, "- ");
		}

		fprintf(file, (functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->r_j) ? "Yes " : "No ");
		fprintf(file, (functionalUnits->functionalUnit[functionalUnits->unitName]->units[functionalUnits->unitNum]->r_k) ? "Yes\n" : "No\n");
	}
}

/*comparator function for units to use when sorting them*/
int unitsCompare(int q_type, int q_index, Unit* unit, int j) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		if (i == q_type) { //first compare by type
			for (int j = 0; j < functionalUnits->functionalUnit[i]->totalUnitsNum; j++) {
				if (j == q_index) { //if type is the same, compare by index
					if (functionalUnits->functionalUnit[i]->units[j] == unit) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
/* matematical conversion from single precision to float */
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
/* matematical conversion from float to single precision */
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
void writeToFiles(FILE* file) {
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		if (!instructionQueue->queue[i]->empty && instructionQueue->queue[i]->clockCyclesOperation[WRITE_RESULT] > 0) {
			int type = instructionQueue->queue[i]->operation, index = instructionQueue->queue[i]->index;
			insertUnitWrapper(functionalUnits->functionalUnit[type]->units[index]);
			functionalUnits->functionalUnit[type]->units[index]->instruction->empty = Yes;
			functionalUnits->functionalUnit[type]->units[index] = initUnit(type, index);
			functionalUnits->functionalUnit[type]->units[index]->empty = Yes;
			functionalUnits->functionalUnit[type]->functionalUnitsNum--;
		}
	}
}
/*print unit to traceinst.txt according to format*/
void printUnitsToTraceInstFile(FILE* file) {
	sort();
	int listLength = length();
	for (int i = 0; i < listLength; i++) {
		printTracinstFile(file, head->op, head->unitType, head->unitIndex, head->fetchCC, head->issueCC, head->readCC, head->exeCC, head->writeCC);
		deleteUnitWrapper();
	}
}
/*print traceinst.txt according to format*/
void printTracinstFile(FILE* file, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write) {
	fseek(file, 0, SEEK_END);
	fprintf(file, "%.8x %d %s%d %d %d %d %d\n", opLine, fetch, unitsTypeNames[type], index, issue, read, exe, write);
}

/*create a new unit wrappet which has all of units attributes and also some fields required to print by format*/
unitWrapper* initUnitWrapper() {
	unitWrapper* unit = (unitWrapper*)malloc(sizeof(unitWrapper));
	if (!unit) {
		return NULL;
	}
	unit->op = unit->unitType = unit->unitIndex = unit->fetchCC = unit->issueCC = unit->readCC = unit->exeCC = unit->writeCC = -1;
	return unit;
}
/*insert unit wrapper consistent to input inst to the head of the list*/
void insertUnitWrapper(Unit* unit) {
	unitWrapper* unitWrapper = malloc(sizeof(unitWrapper));
	if (unitWrapper == NULL) {
		return;
	}
	unitWrapper->op = unit->instruction->command;
	unitWrapper->unitType = unit->type;
	unitWrapper->unitIndex = unit->unitNum;
	unitWrapper->fetchCC = unit->instruction->fetchCycles;
	unitWrapper->issueCC = unit->instruction->clockCyclesOperation[ISSUE];
	unitWrapper->readCC = unit->instruction->clockCyclesOperation[READ_OPERAND];
	unitWrapper->exeCC = unit->instruction->clockCyclesOperation[EXECUTION];
	unitWrapper->writeCC = unit->instruction->clockCyclesOperation[WRITE_RESULT];

	unitWrapper->next = head;

	head = unitWrapper;
}
/*delete head of list and make head point to the next unit wrapper*/
unitWrapper* deleteUnitWrapper() {

	unitWrapper* tempLink = head;
	head = head->next;
	return tempLink;
}
/*make list empty by setting head to null*/
int empty() {
	return head == NULL;
}
/* get length of linked list*/
int length() {
	int length = 0;
	unitWrapper* iterator;

	for (iterator = head; iterator != NULL; iterator = iterator->next) {
		length++;
	}

	return length;
}
/* swap locations in list link, part of bubble sort algorithm*/
void swap(unitWrapper* current, unitWrapper* temp, unitWrapper* next)
{
	updateUnitWrapper(current, temp);
	updateUnitWrapper(next, current);
	updateUnitWrapper(temp, next);
}
/* sort by bubble sort algorithm*/
void sort() {

	int i, j, k;
	unitWrapper* current;
	unitWrapper* next;
	unitWrapper* temp = initUnitWrapper();

	int size = length();
	k = size;

	for (i = 0; i < size - 1; i++, k--) {
		current = head;
		next = head->next;

		for (j = 1; j < k; j++) {

			if (current->fetchCC > next->fetchCC) {
				swap(current, temp, next);
			}

			current = current->next;
			next = next->next;
		}
	}
}
/*set dst attributes according to src attributes */
void updateUnitWrapper(unitWrapper* src, unitWrapper* dst) {
	dst->op = src->op;
	dst->unitType = src->unitType;
	dst->unitIndex = src->unitIndex;
	dst->fetchCC = src->fetchCC;
	dst->issueCC = src->issueCC;
	dst->readCC = src->readCC;
	dst->exeCC = src->exeCC;
	dst->writeCC = src->writeCC;

}
/*returns a pointer to a double array of size (size), vith 0 value. we use this instead of calloc to avoid heap corruption*/
double* getDoubleZeros(int size) {
	double* ret = malloc(size * sizeof(double));
	for (int i = 0; i < size; i++) {
		ret[i] = 0.0;
	}
	return ret;
}
/*returns a pointer to an int array of size (size), vith value (val) . we use this instead of calloc to avoid heap corruption*/
int* getIntVal(int size, int val) {
	int* ret = malloc(size * sizeof(int));
	for (int i = 0; i < size; i++) {
		ret[i] = val;
	}
	return ret;
}

/*allocate memory for all the static data structures*/
void memoryAllocation()
{
	regs = getDoubleZeros(REGISTERS_NUMBER);
	memory = getIntVal(MEMORY_LENGTH, 0);
	resTypes = getIntVal(REGISTERS_NUMBER, -1);
	resIndexes = getIntVal(REGISTERS_NUMBER, -1);
	line = calloc(1, sizeof(char) * MAX_LINE_LENGTH);
}

int simulator(char** filesPaths) {
	unsigned int pc = 0;
	unsigned int operation = 0;
	int instructionNumber = 0;
	int instructionIndex = -1;
	int tempExecutedInstCnt = busyUnitCnt;
	int clockCycles = -1;
	int fetchPossible = Yes;
	int runPossible = Yes;
	int stopSet = No;
	memoryAllocation();
	if (initializeDataStructures(filesPaths) == 0) {
		return 1;
	}
	while (1) {
		clockCycles++;
		printTraceunitFile(filesArray[TRACEUNIT], clockCycles);
		Instruction* instruction = initInstruction();
		if (!instruction) {
			deallocateMemory();
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
			instructionIndex = insertInstToQueue(instruction);
			if (instructionIndex != -1) {
				instructionNumber++;
				instruction->fetchCycles = clockCycles;
			}
			if (clockCycles == 0) {
				continue;
			}
			tempExecutedInstCnt = issue(clockCycles);
			if (tempExecutedInstCnt == 0) {
				removeInstAtIdxFromQueue(instructionIndex);
				instructionNumber--;
				instruction->fetchCycles = -1;
			}
			else {
				busyUnitCnt += tempExecutedInstCnt;
			}
		}
		else {
			runPossible = No;
			for (int i = 0; i < REGISTERS_NUMBER; i++) {
				if (resTypes[i] != -1) {
					runPossible = Yes;
				}
			}
			if (!runPossible && busyUnitCnt == 0) {
				clockCycles--;
				break;
			}
			if (busyUnitCnt > 0) {
				busyUnitCnt += issue(clockCycles);
			}

		}
		performCommand(clockCycles);
	}

	finalize();
	return 0;
}

int initializeDataStructures(char** filesPaths)
{

	if (!line) {
		deallocateMemory();
		return 0;
	}


	instructionQueue = initInstQueue();
	if (!instructionQueue) {
		deallocateMemory();
		return 0;
	}
	if (!openFiles(filesPaths)) {
		return 0;
	}
	if ((initMemory(filesArray[MEMIN])) == 0) {
		deallocateMemory();
		return 0;
	}
	initRegs();
	if ((cfg = analyzeConfiguration(filesArray[CONFIG])) == 0) {
		deallocateMemory();
		return 0;
	}
	if ((functionalUnits = initFunctionalUnits(cfg)) == 0) {
		deallocateMemory();
		return 0;
	}
	return 1;
}

/*for each instuction we rub this function, clockCycles determine in which part we are in the execution*/
void performCommand(int clockCycles)
{
	readOp(clockCycles);
	executionOp(clockCycles);
	writeResult(clockCycles);

	writeToFiles(filesArray[TRACEINST]);
}

void printFiles()
{
	printUnitsToTraceInstFile(filesArray[TRACEINST]);
	printMemoutFile(filesArray[MEMOUT], MEMORY_LENGTH);
	printRegoutFile(filesArray[REGOUT]);
}

void finalize()
{
	printFiles();
	deallocateMemory();
}
/*read memin.txt into the static memory structure*/
int initMemory(FILE* meminFile) {
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
/*initialize registers according to requirments*/
void initRegs() {
	for (int i = 0; i < REGISTERS_NUMBER; i++) {
		regs[i] = (float)i;
	}
}
/*try to issue a command from the instruction queue*/
int issue(int clockCycles) {
	int instUnitIndex = -1;
	for (int i = 0; i < NUM_OF_INSTRUCTION_QUEUE; i++) {
		instUnitIndex += instructionToUnit(instructionQueue->queue[i], clockCycles);
		if (instUnitIndex == 0) {
			return 1; /* can only issue one instruction in each clock cycle*/
		}
	}
	return 0;
}

int instructionToUnit(Instruction* instruction, int clockCycles) {
	if (!instruction || instruction->empty == Yes) {
		return 0;
	}
	int type = instruction->operation;
	/*Halt instruction*/
	if (type < 0 || type > 5) {
		return 0;
	}
	if (functionalUnits->functionalUnit[type]->canEnter) {
		for (int i = 0; i < functionalUnits->functionalUnit[type]->totalUnitsNum; i++) {
			if (!functionalUnits->functionalUnit[type]->units[i] || functionalUnits->functionalUnit[type]->units[i]->empty) {
				if (!functionalUnits->functionalUnit[type]->units[i]->busy && instruction->fetchCycles != clockCycles && instruction->clockCyclesOperation[ISSUE] == -1) {
					if ((resTypes[instruction->dst] == -1 && type != UNIT_ST) || (type == UNIT_ST)) {
						setUnitFields(type, i, instruction, clockCycles);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}
/*set functional unit fields according to the scoreboard algorithm*/
void setUnitFields(int type, int i, Instruction* instruction, int clockCycles)
{
	functionalUnits->functionalUnit[type]->units[i]->busy = Yes;
	functionalUnits->functionalUnit[type]->units[i]->instruction = instruction;
	functionalUnits->functionalUnit[type]->units[i]->op = instruction->opcode;

	functionalUnits->functionalUnit[type]->units[i]->f_i = instruction->dst;
	functionalUnits->functionalUnit[type]->units[i]->f_j = instruction->src0;
	functionalUnits->functionalUnit[type]->units[i]->f_k = instruction->src1;

	functionalUnits->functionalUnit[type]->units[i]->q_j_index = resIndexes[instruction->src0];
	functionalUnits->functionalUnit[type]->units[i]->q_k_index = resIndexes[instruction->src1];

	functionalUnits->functionalUnit[type]->units[i]->q_j_type = resTypes[instruction->src0];
	functionalUnits->functionalUnit[type]->units[i]->q_k_type = resTypes[instruction->src1];

	functionalUnits->functionalUnit[type]->units[i]->r_j = (resTypes[instruction->src0] == -1) ? Yes : No;
	functionalUnits->functionalUnit[type]->units[i]->r_k = (resTypes[instruction->src1] == -1) ? Yes : No;

	functionalUnits->functionalUnit[type]->functionalUnitsNum++;
	if (functionalUnits->functionalUnit[type]->functionalUnitsNum == functionalUnits->functionalUnit[type]->totalUnitsNum) {
		functionalUnits->functionalUnit[type]->canEnter = No;
	}
	functionalUnits->functionalUnit[type]->units[i]->empty = No;

	functionalUnits->functionalUnit[type]->units[i]->instruction->clockCyclesOperation[ISSUE] = clockCycles;

	if (functionalUnits->functionalUnit[type]->units[i]->type != UNIT_ST) {
		resTypes[instruction->dst] = functionalUnits->functionalUnit[type]->units[i]->type;
		resIndexes[instruction->dst] = functionalUnits->functionalUnit[type]->units[i]->unitNum;
	}

	functionalUnits->functionalUnit[type]->units[i]->instruction->index = i;
}
/*check if we can read operands and set NO and YES according to what we saw at class*/
void readOp(int clockCycles) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		for (int j = 0; j < functionalUnits->functionalUnit[i]->totalUnitsNum; j++) {
			if (!functionalUnits->functionalUnit[i]->units[j]->empty && !functionalUnits->functionalUnit[i]->units[j]->instruction->empty) {
				if (functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[ISSUE] < clockCycles) {
					if (functionalUnits->functionalUnit[i]->units[j]->r_j == Yes && functionalUnits->functionalUnit[i]->units[j]->r_k == Yes) {
						if (functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] == -1) {
							functionalUnits->functionalUnit[i]->units[j]->f_j_value = regs[functionalUnits->functionalUnit[i]->units[j]->f_j];
							functionalUnits->functionalUnit[i]->units[j]->f_k_value = regs[functionalUnits->functionalUnit[i]->units[j]->f_k];

							functionalUnits->functionalUnit[i]->units[j]->r_j = No;
							functionalUnits->functionalUnit[i]->units[j]->r_k = No;
							functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] = clockCycles;
							if (functionalUnits->functionalUnit[i]->delay == 1) {
								functionalUnits->functionalUnit[i]->units[j]->instruction->executionCycles = clockCycles + functionalUnits->functionalUnit[i]->delay;
							}
							else {
								functionalUnits->functionalUnit[i]->units[j]->instruction->executionCycles = clockCycles + functionalUnits->functionalUnit[i]->delay - 1;
							}
						}
					}
				}
			}
		}
	}
}

void executionOp(int clockCycles) {
	for (int i = 0; i < UNITS_NUMBER; i++) {
		for (int j = 0; j < functionalUnits->functionalUnit[i]->totalUnitsNum; j++) {
			if (!functionalUnits->functionalUnit[i]->units[j]->empty) {
				if (functionalUnits->functionalUnit[i]->units[j]->instruction->executionCycles >= 0 &&
					0 < functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] &&
					functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[READ_OPERAND] < clockCycles) {
					/* if in real life, scoreboard was supposed to be done executing the instruction, actually make calculations and set the result in the registers*/
					/*this is instead of trying to inplement the instructions in many parts through different clock cycles, this is just a simulator and we show when it is done*/
					if (functionalUnits->functionalUnit[i]->units[j]->instruction->executionCycles == clockCycles) {
						if (functionalUnits->functionalUnit[i]->units[j]->instruction->result == -1) {
							executionInst(functionalUnits->functionalUnit[i]->units[j]->instruction->opcode, i, j, clockCycles);

						}
						functionalUnits->functionalUnit[i]->units[j]->instruction->clockCyclesOperation[EXECUTION] = clockCycles;
					}
				}
			}
		}
	}
}
void ADD(int i, int j) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = functionalUnits->functionalUnit[i]->units[j]->f_j_value + functionalUnits->functionalUnit[i]->units[j]->f_k_value;
}
void SUB(int i, int j) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = functionalUnits->functionalUnit[i]->units[j]->f_j_value - functionalUnits->functionalUnit[i]->units[j]->f_k_value;
}

void DIV(int i, int j) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = functionalUnits->functionalUnit[i]->units[j]->f_j_value / functionalUnits->functionalUnit[i]->units[j]->f_k_value;
}

void MULT(int i, int j) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = functionalUnits->functionalUnit[i]->units[j]->f_j_value * functionalUnits->functionalUnit[i]->units[j]->f_k_value;
}

void ST(int i, int j, int clockCycles) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = functionalUnits->functionalUnit[i]->units[j]->f_k_value;
	checkLdSt(functionalUnits->functionalUnit[i]->units[j]->instruction, clockCycles);
}
void LD(int i, int j) {
	functionalUnits->functionalUnit[i]->units[j]->instruction->result = singlePrecisionToFloat(memory[functionalUnits->functionalUnit[i]->units[j]->instruction->imm]);
}


/*actual execution of the instruction*/
void executionInst(unsigned int opcode, int i, int j, int clockCycles) {
	switch (opcode) {
	case OP_ADD:
		ADD(i, j);
		break;
	case OP_SUB:
		SUB(i, j);
		break;
	case OP_MULT:
		MULT(i, j);
		break;
	case OP_DIV:
		DIV(i, j);
		break;
	case OP_LD:
		LD(i, j);
		break;
	case OP_ST:
		ST(i, j, clockCycles);
		break;
	}
}
/*only set data if no loading instructions are operating in this address*/
void checkLdSt(Instruction* stInst, int clockCycles) {
	for (int i = 0; i < functionalUnits->functionalUnit[LD_UNIT]->totalUnitsNum; i++) {
		if (!functionalUnits->functionalUnit[LD_UNIT]->units[i]->empty) {
			if (functionalUnits->functionalUnit[LD_UNIT]->units[i]->instruction->imm == stInst->imm) {
				if (functionalUnits->functionalUnit[LD_UNIT]->units[i]->instruction->clockCyclesOperation[ISSUE] < stInst->clockCyclesOperation[ISSUE]) {
					if (functionalUnits->functionalUnit[LD_UNIT]->units[i]->instruction->clockCyclesOperation[EXECUTION] <= clockCycles &&
						stInst->executionCycles == clockCycles) {
						stInst->executionCycles++;
					}
				}
			}
		}
	}
}
/*write back stage*/
void writeResult(int clockCycles) {
	int writeResultTypes = 0, unitsNum;
	for (int x = 0; x < UNITS_NUMBER; x++) {
		unitsNum = 0;
		for (int y = 0; y < functionalUnits->functionalUnit[x]->totalUnitsNum; y++) {
			if (functionalUnits->functionalUnit[x]->units[y]->empty) {
				continue;
			}
			writeResultFunctionalUnit(functionalUnits->functionalUnit[x]->units[y], clockCycles);
		}
	}
}

void writeResultFunctionalUnit(Unit* unit, int clockCycles) {
	int writeResultTypes = 0;
	if (!unit->writeResult) {
		for (int x = 0; x < UNITS_NUMBER; x++) {
			for (int y = 0; y < functionalUnits->functionalUnit[x]->totalUnitsNum; y++) {
				if (functionalUnits->functionalUnit[x]->units[y]->empty) {
					continue;
				}
				if ((functionalUnits->functionalUnit[x]->units[y]->f_j != unit->f_i || functionalUnits->functionalUnit[x]->units[y]->r_j == No) &&
					(functionalUnits->functionalUnit[x]->units[y]->f_k != unit->f_i || functionalUnits->functionalUnit[x]->units[y]->r_k == No)) {
					writeResultTypes++;
				}
			}
		}
		if (writeResultTypes != busyUnitCnt || busyUnitCnt == 0) {
			return;
		}
		else {
			unit->writeResult = Yes;
		}
	}
	else {
		writeResultTypes = 0;
		for (int x = 0; x < UNITS_NUMBER; x++) {
			for (int y = 0; y < functionalUnits->functionalUnit[x]->totalUnitsNum; y++) {
				if (functionalUnits->functionalUnit[x]->units[y]->empty) {
					continue;
				}
				if (0 < unit->instruction->clockCyclesOperation[EXECUTION] && unit->instruction->clockCyclesOperation[EXECUTION] < clockCycles) {
					if (unitsCompare(functionalUnits->functionalUnit[x]->units[y]->q_j_type, functionalUnits->functionalUnit[x]->units[y]->q_j_index, unit, 1)) {
						functionalUnits->functionalUnit[x]->units[y]->q_j_index = -1;
						functionalUnits->functionalUnit[x]->units[y]->q_j_type = -1;
						functionalUnits->functionalUnit[x]->units[y]->r_j = Yes;
					}
					else if (unitsCompare(functionalUnits->functionalUnit[x]->units[y]->q_k_type, functionalUnits->functionalUnit[x]->units[y]->q_k_index, unit, 0)) {
						functionalUnits->functionalUnit[x]->units[y]->q_k_index = -1;
						functionalUnits->functionalUnit[x]->units[y]->q_k_type = -1;
						functionalUnits->functionalUnit[x]->units[y]->r_k = Yes;
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
				regs[unit->f_i] = unit->instruction->result;
				break;
			default:
				regs[unit->f_i] = unit->instruction->result;
				break;
			}
			busyUnitCnt--;
			unit->instruction->clockCyclesOperation[WRITE_RESULT] = clockCycles;
			resTypes[unit->f_i] = -1;
			resIndexes[unit->f_i] = -1;
			unit->busy = No;
		}
	}
}
/*free all the memory at the end of the simulation*/
void deallocateMemory() {
	for (int i = 0; i < FILES_NUMBER; i++) {
		fclose(filesArray[i]);
	}
	if (line) { free(line); }
	freeConfiguration();
	freeFunctionalUnit();
	freeInstQueue();
	free(regs);
	free(memory);
	free(resIndexes);
	free(resTypes);
}