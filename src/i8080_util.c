/*

i8080_util.c

Utility functions and types

*/
#include "i8080_util.h"

#include <stdlib.h>
#include <stdio.h>

const uint8_t instructionParams[0x100][3] = {
	{1, 4, 0},{3, 10,0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},{1, 4, 0},{1, 10,0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},
	{1, 4, 0},{3, 10,0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},{1, 4, 0},{1, 10,0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},
	{1, 4, 0},{3, 10,0},{3, 16,0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},{1, 4, 0},{1, 10,0},{3, 16,0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},
	{1, 4, 0},{3, 10,0},{3, 13,0},{1, 5, 0},{1, 10,0},{1, 10,0},{2, 10,0},{1, 4, 0},{1, 4, 0},{1, 10,0},{3, 13,0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{2, 7, 0},{1, 4, 0},
	{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},
	{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},
	{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},
	{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 7, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 5, 0},{1, 7, 0},{1, 5, 0},
	{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},
	{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},
	{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},
	{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 4, 0},{1, 7, 0},{1, 4, 0},
	{1, 11,5},{1, 10,0},{3, 10,0},{3, 10,0},{3,17,11},{1, 11,0},{2, 7, 0},{1, 11,0},{1, 11,5},{1, 10,0},{3, 10,0},{3, 10,0},{3,17,11},{3, 17,0},{2, 7, 0},{1, 11,0},
	{1, 11,5},{1, 10,0},{3, 10,0},{2, 10,0},{3,17,11},{1, 11,0},{2, 7, 0},{1, 11,0},{1, 11,5},{1, 10,0},{3, 10,0},{2, 10,0},{3,17,11},{3, 17,0},{2, 7, 0},{1, 11,0},
	{1, 11,5},{1, 10,0},{3, 10,0},{1, 18,0},{3,17,11},{1, 11,0},{2, 7, 0},{1, 11,0},{1, 11,5},{1, 10,0},{3, 10,0},{1, 5, 0},{3,17,11},{3, 17,0},{2, 7, 0},{1, 11,0},
	{1, 11,5},{1, 10,0},{3, 10,0},{1, 4, 0},{3,17,11},{1, 11,0},{2, 7, 0},{1, 11,0},{1, 11,5},{1, 10,0},{3, 10,0},{1, 4, 0},{3,17,11},{3, 17,0},{2, 7, 0},{1, 11,0},
};

void loadFile(const char* file, unsigned char* buffer, int bufferSize, int offset) {
	FILE* f = fopen(file, "rb");
	if (f == NULL)
	{
		log_error("Couldn't open %s", file);
		return;
	}
	log_debug("Opened file %s", file);

	//Get the file size and read it into a memory buffer    
	fseek(f, 0L, SEEK_END);
	int fsize = ftell(f);
	fseek(f, 0L, SEEK_SET);

	log_debug("Size of file: %i", fsize);

	// Check to see if the buffer is big enough
	if (bufferSize < (offset + fsize)) {
		// Too small
		log_error("Buffer not big enough for file: bufferSize=%i : fileSize=%i", bufferSize, fsize);
		fclose(f);
		return;
	}

	// Write to the buffer at buffer + offset
	fread(buffer + offset, fsize, 1, f);

	log_debug("Read file");

	fclose(f);
}

void stateCheck(i8080State* state) {
	if (state == NULL) {
		log_fatal("NULL state");
		exit(-1);
	}
}

void init8080(i8080State* state) {
	state->valid = true; // set valid

	// Init the memory
	state->memory = malloc(i8080_MEMORY_SIZE * sizeof(uint8_t));
	if (state->memory == NULL) {
		log_fatal("Failed to allocate memory for i8080");
		exit(-1);
	}
	log_info("Init: memory allocated");
	state->memorySize = i8080_MEMORY_SIZE;

	// Reset the state
	reset8080(state);
}

void reset8080(i8080State* state) {
	stateCheck(state);

	// Iterate the memory and clear it
	for (int i = 0; i < i8080_MEMORY_SIZE; i++) {
		state->memory[i] = 0;
	}

	state->a = 0;
	state->b = 0;
	state->c = 0;
	state->d = 0;
	state->e = 0;
	state->h = 0;
	state->l = 0;
	state->sp = 0;
	state->pc = 0;
	state->clockFreqMHz = 0.005;
	state->waitCycles = 0;
}

int getConsoleLine(char* buf, int bufLen) {
	char c;
	int i = 0;
	while (i < bufLen - 1) {
		c = getchar();
		if (c == EOF || c == '\n' || c == '\r') {
			// if we get an EOF or a new line character, break
			break;
		}

		buf[i] = c;
		i++;
	}
	buf[i] = 0; // Set the next position to a null character

	return i; // Return the length of the string got (0th char can techincally be the null character we just appended)
}

bool boundsCheckMemIndex(i8080State* state, int index) {
	stateCheck(state);
	if (index < 0) {
		// Underflow
		return false;
	}
	if (index >= state->memorySize) {
		// Overflow
		return false;
	}
	return true;
}

uint8_t getInstructionLength(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_BYTE_LEN];
	}
	return 1;
}

uint8_t getInstructionClockCycles(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_CLOCK_LEN];
	}
	return 1;
}

uint8_t getFailedInstructionClockCycles(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_FCLOCK_LEN];
	}
	return 0;
}

void setPC(i8080State* state, uint16_t v) {
	if (boundsCheckMemIndex(state, v)) {
		state->pc = v;
	}
	else {
		// ERROR
	}
}
