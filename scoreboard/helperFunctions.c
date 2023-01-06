#include "helperFunctions.h"

cfg* createConfig() {
	cfg* src = (cfg*)malloc(sizeof(cfg));
	if (!src) {
		return 0;
	}
	src->name = -1;
	return src;
}

void freeConfig(cfg* cfg) {
	if (!cfg) {
		return;
	}
	free(cfg);
}

cfg* parseConfig(FILE* cfgFd, char* line) {
	cfg* cfg = createConfig();
	if (!cfg) {
		return 0;
	}
	size_t len = 0;
	ssize_t readLines = 0;
	int linesCounter = 0;

	while (fgets(line, MAX_LINE_LENGTH, cfgFd) != 0) {
		if (!parse(cfg, line)) {
			return 0;
		}
	}
	return cfg;
}

int parse(cfg* cfg, char* line) {
	if (line == '\n') {
		return 1;
	}
	char* delimeter = " =\n\t";
	char* ptr = strtok(line, delimeter);
	if (!ptr) {
		return 1;
	}
	int param;
	char unitTraceName[MAX_LENGTH];
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		if (strcmp(ptr, configUnitsTypes[i]) == 0) {
			if ((param = parseParam(ptr, i, delimeter)) == -1) {
				return 0;
			}
			cfg->units[i] = param;
			return 1;
		}
	}
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		if (strcmp(ptr, configUnitsTypes[i + NUM_OF_UNITS]) == 0) {
			if ((param = parseParam(ptr, i, delimeter)) == -1) {
				return 0;
			}
			cfg->delays[i] = param;
			return 1;
		}
	}
	if (strcmp(ptr, configUnitsTypes[TRACE_UNIT]) == 0) {
		if ((param = parseTraceUnitParam(ptr, TRACE_UNIT, delimeter, unitTraceName)) == -1) {
			return 0;
		}
		if (unitTraceName) {
			char* del = "01234";
			cfg->unitNum = extractDigitFromStr(unitTraceName);
			char* ptr = strtok(unitTraceName, del);
			int name = unitTraceNameToInt(unitTraceName);
			if (name == -1) {
				printf("Error! cfg trace_unit_name invalide.\n");
				return 0;
			}
			cfg->name = name;
		}
	}
	return 1;
}

int parseParam(char* ptr, int paramType, char* delimeter) {
	ptr = strtok(NULL, delimeter);
	int num = atoi(ptr);
	return num;
}

int parseTraceUnitParam(char* ptr, int paramType, char* delimeter, char* unitTraceName) {
	ptr = strtok(NULL, delimeter);
	if (!ptr) {
		printf("Couldnt parse traceunit name.\n");
		return 0;
	}
	strcpy(unitTraceName, ptr);
	return 1;
}

int extractDigitFromStr(char* str) {
	char* ptr = str;
	while (*ptr) {
		if (isdigit(*ptr)) {
			int res = atoi(ptr);
			return res;
		}
		else {
			ptr++;
		}
	}
	return -1;
}

int unitTraceNameToInt(char* str) {
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		if (strcmp(str, unitsTypeNames[i]) == 0) {
			return i;
		}
	}
	return -1;
}

//inst

Instruction* createInstruction() {
	Instruction* inst = malloc(sizeof(Instruction));
	if (!inst) {
		return NULL;
	}
	inst->isEmpty = Yes;
	inst->writeToFile = No;
	inst->canIssue = Yes;
	for (int i = 0; i < NUM_OF_CYCLES_TYPES; i++) {
		inst->stateCC[i] = -1;
	}
	inst->instRes = inst->executionTime = inst->instType = inst->instIndex = inst->queueIndex = inst->status = -1;
	inst->fetchedTime = 0;
	return inst;
}

void freeInstruction(Instruction* inst) {
	if (inst) {
		free(inst);
	}
}
void parseInstruction(Instruction* inst, int command) {
	inst->command = command;
	inst->opcode = 0xF & ((command << 4) >> 28);
	inst->dst = 0xF & ((command << 8) >> 28);
	inst->src0 = 0xF & ((command << 12) >> 28);
	inst->src1 = 0xF & ((command << 16) >> 28);
	inst->imm = 0xF & ((command << 20) >> 20);
	inst->isEmpty = No;
	if (inst->opcode != HALT) {
		if (inst->opcode != LD) {
			inst->instType = inst->opcode;
		}
		else {
			inst->instType = inst->opcode;
		}
	}
	else {
		inst->instType = -1;
	}
}


InstructionQueue* initializeInstQueue() {
	InstructionQueue* instQueue = malloc(sizeof(InstructionQueue));
	if (!instQueue) {
		printf("Error! Memory allocation failure.\n");
		return NULL;
	}
	for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
		instQueue->queue[i] = createInstruction();
		if (!instQueue->queue[i]) {
			printf("Error! Memory allocation failure.\n");
			for (int j = i; j >= 0; j--) {
				freeInstruction(instQueue->queue[j]);
			}
			free(instQueue);
			return NULL;
		}
	}
	instQueue->isQueueFull = No;
	instQueue->isQueueEmpty = Yes;
	return instQueue;
}

void freeInstructionQueue(InstructionQueue* instQueue) {
	if (instQueue) {
		for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
			freeInstruction(instQueue->queue[i]);
		}
		free(instQueue);
	}
}

int addInstructionToInstructionQueue(InstructionQueue* queue, Instruction* inst) {
	if (inst->instType == -1) {
		return -1;
	}
	if (queue->isQueueFull) {
		return -1;
	}
	for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
		if (queue->queue[i]->isEmpty) {
			queue->queue[i] = inst;
			setQueueProperties(queue);
			return i;
		}
	}
	return -1;
}
int removeInstructionFromInstructionQueue(InstructionQueue* queue, int instIdx) {
	if (queue->isQueueEmpty) {
		return 0;
	}
	queue->queue[instIdx] = createInstruction();
	setQueueProperties(queue);
	return 1;
}

void setQueueProperties(InstructionQueue* queue) {
	int numOfInstructions = 0;
	for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
		if (queue->queue[i]->isEmpty != Yes) {
			numOfInstructions++;
		}
	}
	if (numOfInstructions == NUM_OF_INSTRUCTION_IN_QUEUE) {
		queue->isQueueFull = Yes;
		queue->isQueueEmpty = No;
	}
	else if (numOfInstructions == 0) {
		queue->isQueueEmpty = Yes;
		queue->isQueueFull = No;
	}
	else {
		queue->isQueueFull = No;
		queue->isQueueEmpty = No;
	}
}

//units


Unit* createUnit(unitType type, int num) {
	Unit* src = (Unit*)malloc(sizeof(Unit));
	if (!src) {
		printf("Error! Memory allocation failure.\n");
		return 0;
	}
	src->type = type;
	src->unitNum = num;
	src->isEmpty = Yes;
	src->canWriteResult = No;

	src->busy = No;
	src->op = src->Fi = src->Fj = src->Fk = src->QjType = src->QkType = src->QjIdx = src->QkIdx = src->Rj = src->Rk = -1;
	return src;
}

void freeUnit(Unit* src) {
	if (!src) {
		return;
	}
	free(src);
}

Units* createUnits(int numOfUnits, int delay, unitType type) {
	Units* src = malloc(sizeof(Units));
	if (src == NULL) {
		printf("Error! Memory allocation failure.\n");
		return NULL;
	}

	Unit** units = malloc(numOfUnits * sizeof(Unit*));
	if (units == NULL) {
		printf("Error! Memory allocation failure.\n");
		return NULL;
	}

	for (int i = 0; i < numOfUnits; i++) {
		Unit* unit = createUnit(type, i);
		src->units[i] = unit;
		if (src->units[i] == NULL) {
			printf("Error! Memory allocation failure.\n");
			for (int j = i; j > -1; j--) {
				freeUnit(src->units[j]);
			}
			return NULL;
		}
	}
	src->numOfTotalUnits = numOfUnits;
	src->numOfActiveUnits = 0;
	src->delay = delay;
	src->type = type;
	src->canInsert = Yes;
	return src;
}
void freeUnits(Units* src) {
	if (src == NULL) {
		return;
	}
	for (int i = 0; i < MAX_NUM_OF_FUNCTIONAL_UNITS; i++) {
		if (src->numOfActiveUnits != 0) {
			freeUnit(src->units[i]);
			src->numOfActiveUnits--;
		}
	}
	free(src);
}

FunctionalUnit* createFunctionalUnit(cfg* cfg) {
	FunctionalUnit* src = (FunctionalUnit*)malloc(sizeof(FunctionalUnit));
	if (!src) {
		printf("Error! Memory allocation failure.\n");
		return 0;
	}
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		src->functionalUnit[i] = createUnits(cfg->units[i], cfg->delays[i], i);
		if (!src->functionalUnit[i]) {
			printf("Error! Memory allocation failure.\n");
			for (int j = i; j > -1; j--) {
				freeUnits(src->functionalUnit[j]);
			}
			return 0;
		}
	}
	src->unitNum = cfg->unitNum;
	src->unitName = cfg->name;
	return src;
}

void freeFunctionalUnit(FunctionalUnit* fus) {
	if (fus == NULL) {
		return;
	}
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		freeUnits(fus->functionalUnit[i]);
	}
	free(fus);
}

//utilities


int verifyFiles(FILE** filesFd, char** filesPaths) {
	for (int i = 0; i < NUM_OF_FILES; i++) {
		char* mode = "r";
		if (i > 1) {
			mode = "w";
		}
		FILE* fd = fopen(filesPaths[i], mode);
		if (!fd) {
			printf("Error! could'nt open %s.\n", filesPaths[i]);
			for (int j = 0; j < i; j++) {
				fclose(filesFd[j]);
			}
			return 0;
		}
		filesFd[i] = fd;
	}
	return 1;
}

void printMemoutFile(FILE* fd, int* memory, int maxLines) {
	for (int i = 0; i < maxLines; i++) {
		fprintf(fd, "%.8x\n", memory[i]);
	}
}

void printRegoutFile(FILE* fd, double* regs) {
	int i;
	for (i = 0; i < NUM_OF_REGISTERS; i++) {
		fprintf(fd, "%f\n", regs[i]);
	}
}

void printTraceunitFile(FILE* fd, FunctionalUnit* fus, int* resultTypes, int* resultIndexes, int cc) {
	int unitBusy = No;
	for (int i = 0; i < NUM_OF_REGISTERS; i++) {
		if (resultTypes[i] == fus->unitName && resultIndexes[i] == fus->unitNum) {
			unitBusy = Yes;
		}
	}
	if (unitBusy) {
		fprintf(fd, "%d ", cc);
		fprintf(fd, "%s%d ", unitsTypeNames[fus->unitName], fus->unitNum);

		fprintf(fd, "F%d ", fus->functionalUnit[fus->unitName]->units[fus->unitNum]->Fi);
		fprintf(fd, "F%d ", fus->functionalUnit[fus->unitName]->units[fus->unitNum]->Fj);
		fprintf(fd, "F%d ", fus->functionalUnit[fus->unitName]->units[fus->unitNum]->Fk);

		int q_j = (fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QjType == -1) ? 0 : 1;
		int q_k = (fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QkType == -1) ? 0 : 1;

		if (q_j) {
			fprintf(fd, "%s%d ", unitsTypeNames[fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QjType], fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QjIdx);
		}
		else {
			fprintf(fd, "- ");
		}
		if (q_k) {
			fprintf(fd, "%s%d ", unitsTypeNames[fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QkType], fus->functionalUnit[fus->unitName]->units[fus->unitNum]->QkIdx);
		}
		else {
			fprintf(fd, "- ");
		}

		fprintf(fd, (fus->functionalUnit[fus->unitName]->units[fus->unitNum]->Rj) ? "Yes " : "No ");
		fprintf(fd, (fus->functionalUnit[fus->unitName]->units[fus->unitNum]->Rk) ? "Yes\n" : "No\n");
	}
}

int cmdToHex(Instruction* instruction) {
	unsigned int hex = 0;
	hex += instruction->opcode << 24;
	hex += instruction->dst << 20;
	hex += instruction->src0 << 16;
	hex += instruction->src1 << 12;
	hex += 0xFFFFF & instruction->imm;
	return hex;
}



int areUnitsEqual(FunctionalUnit* fu, int q_type, int q_index, Unit* src2, int isJ) {
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		if (i == q_type) {
			for (int j = 0; j < fu->functionalUnit[i]->numOfTotalUnits; j++) {
				if (j == q_index) {
					if (fu->functionalUnit[i]->units[j] == src2) {
						return 1;
					}
				}
			}
		}
	}
	return 0;
}




float singlePrecisionToFloat(unsigned long singlePrecision) {
	unsigned long sign, exp, fractionBits, fractionB;
	int i = 0;
	float res, fraction = 1.0;
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
	res = powf(-1, sign) * powf(2, exp - 127) * fraction;

	return res;
}

int floatToSinglePrecision(float f) {
	int exp, fraction, expLenBits, i = 0, fractionToBit = 0;
	int floatInInt = (int)floor(f);
	int res;

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
	res = (((sign << 8) + exp) << 23) + fraction;

	return res;
}
void cleanAndWriteToFiles(FILE* fd, FunctionalUnit* fus, InstructionQueue* queue) {
	for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
		if (!queue->queue[i]->isEmpty && queue->queue[i]->stateCC[WRITE_RESULT] > 0) {
			int type = queue->queue[i]->instType, index = queue->queue[i]->instIndex;
			insertFirst(fus->functionalUnit[type]->units[index]);
			fus->functionalUnit[type]->units[index]->instruction->isEmpty = Yes;
			fus->functionalUnit[type]->units[index] = createUnit(type, index);
			fus->functionalUnit[type]->units[index]->isEmpty = Yes;
			fus->functionalUnit[type]->numOfActiveUnits--;
		}
	}
}

void printUnitsToTraceInstFile(FILE* fd) {
	sort();
	int listLength = getLinkedListLength();
	for (int i = 0; i < listLength; i++) {
		printTracinstFile(fd, head->op, head->unitType, head->unitIndex, head->fetchCC, head->issueCC, head->readCC, head->exeCC, head->writeCC);
		deleteFirst();
	}
}

void printTracinstFile(FILE* fd, unsigned int opLine, int type, int index, int fetch, int issue, int read, int exe, int write) {
	fseek(fd, 0, SEEK_END);
	fprintf(fd, "%.8x %d %s%d %d %d %d %d\n", opLine, fetch, unitsTypeNames[type], index, issue, read, exe, write);
}

unit* createEmptyPrintUnit() {
	unit* src = (unit*)malloc(sizeof(unit));
	if (!src) {
		return;
	}
	src->op = src->unitType = src->unitIndex = src->fetchCC = src->issueCC = src->readCC = src->exeCC = src->writeCC = -1;
	return src;
}
void insertFirst(Unit* unitSrc) {
	// create a link
	unit* src = malloc(sizeof(unit));
	if (src == NULL) {
		return;
	}
	src->op = unitSrc->instruction->command;
	src->unitType = unitSrc->type;
	src->unitIndex = unitSrc->unitNum;
	src->fetchCC = unitSrc->instruction->fetchedTime;
	src->issueCC = unitSrc->instruction->stateCC[ISSUE];
	src->readCC = unitSrc->instruction->stateCC[READ_OPERAND];
	src->exeCC = unitSrc->instruction->stateCC[EXECUTION];
	src->writeCC = unitSrc->instruction->stateCC[WRITE_RESULT];

	src->next = head;

	head = src;
}

unit* deleteFirst() {

	unit* tempLink = head;
	head = head->next;
	return tempLink;
}

int isEmpty() {
	return head == NULL;
}

int getLinkedListLength() {
	int length = 0;
	unit* current;

	for (current = head; current != NULL; current = current->next) {
		length++;
	}

	return length;
}

void sort() {

	int i, j, k, tempKey;
	unit* current;
	unit* next;
	unit* temp = createEmptyPrintUnit();

	int size = getLinkedListLength();
	k = size;

	for (i = 0; i < size - 1; i++, k--) {
		current = head;
		next = head->next;

		for (j = 1; j < k; j++) {

			if (current->fetchCC > next->fetchCC) {
				setData(current, temp);
				setData(next, current);
				setData(temp, next);
			}

			current = current->next;
			next = next->next;
		}
	}
}

void setData(unit* unitSrc, unit* dst) {
	dst->op = unitSrc->op;
	dst->unitType = unitSrc->unitType;
	dst->unitIndex = unitSrc->unitIndex;
	dst->fetchCC = unitSrc->fetchCC;
	dst->issueCC = unitSrc->issueCC;
	dst->readCC = unitSrc->readCC;
	dst->exeCC = unitSrc->exeCC;
	dst->writeCC = unitSrc->writeCC;

}

//simulator

int simulator(char** filesPaths) {
	FILE* filesFd[NUM_OF_FILES];
	cfg* cfg;
	InstructionQueue* instQueue;
	FunctionalUnit* functionalUnit;
	int memory[MEMORY_LENGTH] = { 0 };
	double regs[NUM_OF_REGISTERS] = { 0 };

	int numOfLines;
	unsigned int pc = 0;
	int instNum = 0;
	unsigned int op = 0;
	unsigned int lineNum = 0;
	int instQueueIndex = -1;
	int cc = -1;
	int canRun = Yes;
	int haltSet = No;
	int tempNumOfWorkingInst = numOfWorkingUnits;
	int canFetch = Yes;

	int resultTypes[NUM_OF_REGISTERS];
	int resultIndexes[NUM_OF_REGISTERS];
	for (int i = 0; i < NUM_OF_REGISTERS; i++) {
		resultTypes[i] = -1;
		resultIndexes[i] = -1;
	}
	char* line = calloc(1, sizeof(char) * MAX_LINE_LENGTH);

	if (!line) {
		freeSolution(0, 0, 0, 0, 0, 0);
		return 0;
	}
	instQueue = initializeInstQueue();
	if (!instQueue) {
		freeSolution(0, line, 0, 0, 0, 0);
		return 0;
	}
	if (!verifyFiles(filesFd, filesPaths)) {
		return 0;
	}
	if ((numOfLines = readMemin(filesFd[MEMIN], line, memory)) == 0) {
		freeSolution(filesFd, line, 0, 0, 0, instQueue);
		return 0;
	}
	initializeRegs(regs);
	if ((cfg = parseConfig(filesFd[CFG], line)) == 0) {
		freeSolution(filesFd, line, cfg, 0, 0, instQueue);
		return 0;
	}
	if ((functionalUnit = createFunctionalUnit(cfg)) == 0) {
		freeSolution(filesFd, line, cfg, 0, functionalUnit, instQueue);
		return 0;
	}
	while (1) {
		cc++;
		printTraceunitFile(filesFd[TRACEUNIT], functionalUnit, resultTypes, resultIndexes, cc);
		Instruction* instruction = createInstruction();
		if (!instruction) {
			freeSolution(filesFd, line, cfg, instruction, functionalUnit, instQueue);
			return 0;
		}
		if (!haltSet) {
			parseInstruction(instruction, memory[instNum]);
			if (instruction->opcode == HALT) {
				haltSet = Yes;
				canRun = No;
				for (int i = 0; i < NUM_OF_REGISTERS; i++) {
					if (resultTypes[i] != -1) {
						canRun = Yes;
					}
				}
			}
			instQueueIndex = addInstructionToInstructionQueue(instQueue, instruction);
			if (instQueueIndex != -1) {
				instNum++;
				instruction->fetchedTime = cc;
			}
			if (cc == 0) {
				continue;
			}
			tempNumOfWorkingInst = issue(functionalUnit, instQueue, resultTypes, resultIndexes, cc);
			if (tempNumOfWorkingInst == 0) {
				removeInstructionFromInstructionQueue(instQueue, instQueueIndex);
				instNum--;
				instruction->fetchedTime = -1;
			}
			else {
				numOfWorkingUnits += tempNumOfWorkingInst;
			}
		}
		else {
			canRun = No;
			for (int i = 0; i < NUM_OF_REGISTERS; i++) {
				if (resultTypes[i] != -1) {
					canRun = Yes;
				}
			}
			if (!canRun && numOfWorkingUnits == 0) {
				cc--;
				break;
			}
			if (numOfWorkingUnits > 0) {
				numOfWorkingUnits += issue(functionalUnit, instQueue, resultTypes, resultIndexes, instQueueIndex, cc);
			}

		}
		performCommand(functionalUnit, regs, cc, memory, filesFd, instQueue, resultTypes, resultIndexes);
	}

	finalize(filesFd, memory, regs, line, cfg, functionalUnit, instQueue);
	return 0;
}

void performCommand(FunctionalUnit* functionalUnit, double  regs[16], int cc, int  memory[4096], FILE* filesFd[6], InstructionQueue* instQueue, int  resultTypes[16], int  resultIndexes[16])
{
	readOperand(functionalUnit, regs, cc);
	execution(functionalUnit, memory, regs, cc);
	writeResult(filesFd, functionalUnit, instQueue, memory, resultTypes, resultIndexes, regs, cc);

	cleanAndWriteToFiles(filesFd[TRACEINST], functionalUnit, instQueue);
}

void finalize(FILE* filesFd[6], int  memory[4096], double  regs[16], char* line, cfg* cfg, FunctionalUnit* functionalUnit, InstructionQueue* instQueue)
{
	printUnitsToTraceInstFile(filesFd[TRACEINST]);

	printMemoutFile(filesFd[MEMOUT], memory, MEMORY_LENGTH);

	printRegoutFile(filesFd[REGOUT], regs);

	freeSolution(filesFd, line, cfg, 0, functionalUnit, instQueue);
}

int readMemin(FILE* meminFd, char* line, int* memory) {
	int numOfLines = 0;
	int maxLine = 0;
	char currentLine[MAX_LINE_LENGTH];
	while (fgets(line, MAX_LINE_LENGTH, meminFd) != 0) {
		if (sscanf(line, "%s", currentLine)) {
			if (strcmp(currentLine, "0x00000000") != 0) {
				maxLine = numOfLines;
			}
			if (sscanf(currentLine, "%x", &memory[numOfLines]))
				numOfLines++;
			else return 0;

		}
		else return 0;
	}
	return maxLine;
}

void initializeRegs(double* regs) {
	for (int i = 0; i < NUM_OF_REGISTERS; i++) {
		regs[i] = i / 1.0;
	}
}

int issue(FunctionalUnit* functionalUnit, InstructionQueue* queue, int* resultTypes, int* resultIndexes, int cc) {
	int instUnitIndex = -1;
	for (int i = 0; i < NUM_OF_INSTRUCTION_IN_QUEUE; i++) {
		instUnitIndex += attachInstructionToUnit(functionalUnit, queue->queue[i], resultTypes, resultIndexes, cc);
		if (instUnitIndex == 0) {
			return 1;
		}
	}
	return 0;
}

int attachInstructionToUnit(FunctionalUnit* functionalUnits, Instruction* instruction, int* resultTypes, int* resultIndexes, int cc) {
	if (!instruction || instruction->isEmpty == Yes) {
		return 0;
	}
	int type = instruction->instType;
	/*Halt instruction*/
	if (type < 0 || type > 5) {
		return 0;
	}
	if (functionalUnits->functionalUnit[type]->canInsert) {
		for (int i = 0; i < functionalUnits->functionalUnit[type]->numOfTotalUnits; i++) {
			if (!functionalUnits->functionalUnit[type]->units[i] || functionalUnits->functionalUnit[type]->units[i]->isEmpty) {
				if (!functionalUnits->functionalUnit[type]->units[i]->busy && instruction->fetchedTime != cc && instruction->stateCC[ISSUE] == -1) {
					if ((resultTypes[instruction->dst] == -1 && type != UNIT_ST) || (type == UNIT_ST)) {
						setUnitFields(functionalUnits, type, i, instruction, resultIndexes, resultTypes, cc);
						return 1;
					}
				}
			}
		}
	}
	return 0;
}

void setUnitFields(FunctionalUnit* functionalUnits, int type, int i, Instruction* instruction, int* resultIndexes, int* resultTypes, int cc)
{
	functionalUnits->functionalUnit[type]->units[i]->busy = Yes;
	functionalUnits->functionalUnit[type]->units[i]->instruction = instruction;
	functionalUnits->functionalUnit[type]->units[i]->op = instruction->opcode;

	functionalUnits->functionalUnit[type]->units[i]->Fi = instruction->dst;
	functionalUnits->functionalUnit[type]->units[i]->Fj = instruction->src0;
	functionalUnits->functionalUnit[type]->units[i]->Fk = instruction->src1;

	functionalUnits->functionalUnit[type]->units[i]->QjIdx = resultIndexes[instruction->src0];
	functionalUnits->functionalUnit[type]->units[i]->QkIdx = resultIndexes[instruction->src1];

	functionalUnits->functionalUnit[type]->units[i]->QjType = resultTypes[instruction->src0];
	functionalUnits->functionalUnit[type]->units[i]->QkType = resultTypes[instruction->src1];

	functionalUnits->functionalUnit[type]->units[i]->Rj = (resultTypes[instruction->src0] == -1) ? Yes : No;
	functionalUnits->functionalUnit[type]->units[i]->Rk = (resultTypes[instruction->src1] == -1) ? Yes : No;

	functionalUnits->functionalUnit[type]->numOfActiveUnits++;
	if (functionalUnits->functionalUnit[type]->numOfActiveUnits == functionalUnits->functionalUnit[type]->numOfTotalUnits) {
		functionalUnits->functionalUnit[type]->canInsert = No;
	}
	functionalUnits->functionalUnit[type]->units[i]->isEmpty = No;

	functionalUnits->functionalUnit[type]->units[i]->instruction->stateCC[ISSUE] = cc;

	if (functionalUnits->functionalUnit[type]->units[i]->type != UNIT_ST) {
		resultTypes[instruction->dst] = functionalUnits->functionalUnit[type]->units[i]->type;
		resultIndexes[instruction->dst] = functionalUnits->functionalUnit[type]->units[i]->unitNum;
	}

	functionalUnits->functionalUnit[type]->units[i]->instruction->instIndex = i;
}

void readOperand(FunctionalUnit* functionalUnit, double* regs, int cc) {
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		for (int j = 0; j < functionalUnit->functionalUnit[i]->numOfTotalUnits; j++) {
			if (!functionalUnit->functionalUnit[i]->units[j]->isEmpty && !functionalUnit->functionalUnit[i]->units[j]->instruction->isEmpty) {
				if (functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[ISSUE] < cc) {
					if (functionalUnit->functionalUnit[i]->units[j]->Rj == Yes && functionalUnit->functionalUnit[i]->units[j]->Rk == Yes) {
						if (functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[READ_OPERAND] == -1) {
							functionalUnit->functionalUnit[i]->units[j]->FjVal = regs[functionalUnit->functionalUnit[i]->units[j]->Fj];
							functionalUnit->functionalUnit[i]->units[j]->FkVal = regs[functionalUnit->functionalUnit[i]->units[j]->Fk];

							functionalUnit->functionalUnit[i]->units[j]->Rj = No;
							functionalUnit->functionalUnit[i]->units[j]->Rk = No;
							functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[READ_OPERAND] = cc;
							if (functionalUnit->functionalUnit[i]->delay == 1) {
								functionalUnit->functionalUnit[i]->units[j]->instruction->executionTime = cc + functionalUnit->functionalUnit[i]->delay;
							}
							else {
								functionalUnit->functionalUnit[i]->units[j]->instruction->executionTime = cc + functionalUnit->functionalUnit[i]->delay - 1;
							}
						}
					}
				}
			}
		}
	}
}

void execution(FunctionalUnit* functionalUnit, int* mem, double* regs, int cc) {
	for (int i = 0; i < NUM_OF_UNITS; i++) {
		for (int j = 0; j < functionalUnit->functionalUnit[i]->numOfTotalUnits; j++) {
			if (!functionalUnit->functionalUnit[i]->units[j]->isEmpty) {
				if (functionalUnit->functionalUnit[i]->units[j]->instruction->executionTime >= 0 &&
					0 < functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[READ_OPERAND] &&
					functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[READ_OPERAND] < cc) {

					if (functionalUnit->functionalUnit[i]->units[j]->instruction->executionTime == cc) {
						if (functionalUnit->functionalUnit[i]->units[j]->instruction->instRes == -1) {
							executionInstruction(functionalUnit, mem, regs, functionalUnit->functionalUnit[i]->units[j]->instruction->opcode, i, j, cc);

						}
						functionalUnit->functionalUnit[i]->units[j]->instruction->stateCC[EXECUTION] = cc;
					}
				}
			}
		}
	}
}

void executionInstruction(FunctionalUnit* functionalUnit, int* mem, double* regs, unsigned int instOp, int i, int j, int cc) {
	switch (instOp) {
	case ADD:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = functionalUnit->functionalUnit[i]->units[j]->FjVal + functionalUnit->functionalUnit[i]->units[j]->FkVal;
		break;
	case SUB:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = functionalUnit->functionalUnit[i]->units[j]->FjVal - functionalUnit->functionalUnit[i]->units[j]->FkVal;
		break;
	case MULT:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = functionalUnit->functionalUnit[i]->units[j]->FjVal * functionalUnit->functionalUnit[i]->units[j]->FkVal;
		break;
	case DIV:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = functionalUnit->functionalUnit[i]->units[j]->FjVal / functionalUnit->functionalUnit[i]->units[j]->FkVal;
		break;
	case LD:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = singlePrecisionToFloat(mem[functionalUnit->functionalUnit[i]->units[j]->instruction->imm]);
		break;
	case ST:
		functionalUnit->functionalUnit[i]->units[j]->instruction->instRes = functionalUnit->functionalUnit[i]->units[j]->FkVal;
		checkIfLdAndStCollide(functionalUnit, functionalUnit->functionalUnit[i]->units[j]->instruction, cc);
		break;
	}
}

void checkIfLdAndStCollide(FunctionalUnit* functionalUnit, Instruction* stInst, int cc) {
	for (int i = 0; i < functionalUnit->functionalUnit[LD_UNIT]->numOfTotalUnits; i++) {
		if (!functionalUnit->functionalUnit[LD_UNIT]->units[i]->isEmpty) {
			if (functionalUnit->functionalUnit[LD_UNIT]->units[i]->instruction->imm == stInst->imm) {
				if (functionalUnit->functionalUnit[LD_UNIT]->units[i]->instruction->stateCC[ISSUE] < stInst->stateCC[ISSUE]) {
					if (functionalUnit->functionalUnit[LD_UNIT]->units[i]->instruction->stateCC[EXECUTION] <= cc &&
						stInst->executionTime == cc) {
						stInst->executionTime++;
					}
				}
			}
		}
	}
}

void writeResult(FILE** fds, FunctionalUnit* functionalUnit, InstructionQueue* queue, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc) {
	int canWriteresultTypes = 0, numOfUnits;
	for (int x = 0; x < NUM_OF_UNITS; x++) {
		numOfUnits = 0;
		for (int y = 0; y < functionalUnit->functionalUnit[x]->numOfTotalUnits; y++) {
			if (functionalUnit->functionalUnit[x]->units[y]->isEmpty) {
				continue;
			}
			writeResultFunctionalUnit(fds, functionalUnit, functionalUnit->functionalUnit[x]->units[y], mem, resultTypes, resultIndexes, regs, cc);
		}
	}
}

void writeResultFunctionalUnit(FILE** fds, FunctionalUnit* functionalUnits, Unit* unit, int* mem, int* resultTypes, int* resultIndexes, double* regs, int cc) {
	int canWriteresultTypes = 0;
	if (!unit->canWriteResult) {
		for (int x = 0; x < NUM_OF_UNITS; x++) {
			for (int y = 0; y < functionalUnits->functionalUnit[x]->numOfTotalUnits; y++) {
				if (functionalUnits->functionalUnit[x]->units[y]->isEmpty) {
					continue;
				}
				if ((functionalUnits->functionalUnit[x]->units[y]->Fj != unit->Fi || functionalUnits->functionalUnit[x]->units[y]->Rj == No) &&
					(functionalUnits->functionalUnit[x]->units[y]->Fk != unit->Fi || functionalUnits->functionalUnit[x]->units[y]->Rk == No)) {
					canWriteresultTypes++;
				}
			}
		}
		if (canWriteresultTypes != numOfWorkingUnits || numOfWorkingUnits == 0) {
			return;
		}
		else {
			unit->canWriteResult = Yes;
		}
	}
	else {
		canWriteresultTypes = 0;
		for (int x = 0; x < NUM_OF_UNITS; x++) {
			for (int y = 0; y < functionalUnits->functionalUnit[x]->numOfTotalUnits; y++) {
				if (functionalUnits->functionalUnit[x]->units[y]->isEmpty) {
					continue;
				}
				if (0 < unit->instruction->stateCC[EXECUTION] && unit->instruction->stateCC[EXECUTION] < cc) {
					if (areUnitsEqual(functionalUnits, functionalUnits->functionalUnit[x]->units[y]->QjType, functionalUnits->functionalUnit[x]->units[y]->QjIdx, unit, 1)) {
						functionalUnits->functionalUnit[x]->units[y]->QjIdx = -1;
						functionalUnits->functionalUnit[x]->units[y]->QjType = -1;
						functionalUnits->functionalUnit[x]->units[y]->Rj = Yes;
					}
					else if (areUnitsEqual(functionalUnits, functionalUnits->functionalUnit[x]->units[y]->QkType, functionalUnits->functionalUnit[x]->units[y]->QkIdx, unit, 0)) {
						functionalUnits->functionalUnit[x]->units[y]->QkIdx = -1;
						functionalUnits->functionalUnit[x]->units[y]->QkType = -1;
						functionalUnits->functionalUnit[x]->units[y]->Rk = Yes;
					}
				}
			}
		}
		if (0 < unit->instruction->stateCC[EXECUTION] && unit->instruction->stateCC[EXECUTION] < cc) {
			switch (unit->type) {
			case UNIT_ST:
				mem[unit->instruction->imm] = floatToSinglePrecision(unit->instruction->instRes);
				break;
			case UNIT_LD:
				regs[unit->Fi] = unit->instruction->instRes;
				break;
			default:
				regs[unit->Fi] = unit->instruction->instRes;
				break;
			}
			numOfWorkingUnits--;
			unit->instruction->stateCC[WRITE_RESULT] = cc;
			resultTypes[unit->Fi] = -1;
			resultIndexes[unit->Fi] = -1;
			unit->busy = No;
		}
	}
}

void freeSolution(FILE** fds, char* line, cfg* cfg, Instruction* cmd, FunctionalUnit* fus, InstructionQueue* queue) {
	for (int i = 0; i < NUM_OF_FILES; i++) {
		fclose(fds[i]);
	}
	if (line) { free(line); }
	freeInstruction(cmd);
	freeConfig(cfg);
	freeFunctionalUnit(fus);
	freeInstructionQueue(queue);
}