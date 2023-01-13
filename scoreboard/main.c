#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include "helperFunctions.h"

int main(int argc, char** argv) {
	if (argc != FILES_NUMBER + 1) {
		printf("Invalid number of arguments.\n");
		return 0;
	}
	char* filesPaths[FILES_NUMBER + 1] = { argv[1], argv[2], argv[3], argv[4], argv[5], argv[6] };
	return simulator(filesPaths);
}