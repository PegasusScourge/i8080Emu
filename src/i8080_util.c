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

const char* i8080_decompile(uint8_t opcode) {
	switch (opcode) {
	case NOP: return "NOP"; break;
	case LXI_B: return "LXI B"; break;
	}
	return "unknown";
}

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

void i8080_stateCheck(i8080State* state) {
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
	i8080_stateCheck(state);

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
	state->clockFreqMHz = 2.0;
	state->waitCycles = 0;

	// Set the flags
	state->f.ac = 0;
	state->f.c = 0;
	state->f.one = 1;
	state->f.p = 0;
	state->f.s = 0;
	state->f.z = 0;
	state->f.zero = 0;
	state->f.ien = 1; // Interrupts are enabled by default
	state->f.isi = 0;

	// Set the video memory flags
	state->vid.startAddress = 0;
	state->vid.height = 64;
	state->vid.width = 64;

	state->cyclesExecuted = 0;
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

bool i8080_boundsCheckMemIndex(i8080State* state, int index) {
	i8080_stateCheck(state);
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

uint8_t i8080_getInstructionLength(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_BYTE_LEN];
	}
	return 1;
}

uint8_t i8080_getInstructionClockCycles(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_CLOCK_LEN];
	}
	return 1;
}

uint8_t i8080_getFailedInstructionClockCycles(uint8_t opcode) {
	if (opcode >= 0x00 && opcode < 0x100) {
		return instructionParams[opcode][PARAMS_FCLOCK_LEN];
	}
	return 0;
}

bool i8080_isParityEven(uint16_t n) {
	int numOneBits = 0;
	for (int i = 0; i < 16; i++) {
		uint16_t v = (n >> i) & 0b0000000000000001;
		numOneBits += v;
	}
	//log_trace("numOneBits = %i", numOneBits);
	return numOneBits % 2 == 0; // If even, return true
}

bool i8080_isZero(uint16_t n) {
	return n == 0;
}

bool i8080_isNegative(int16_t n) {
	return (0x80 == (n & 0x80));
}

bool i8080_shouldACFlag(uint8_t n) {
	// Other possible implementation: ((c->a | val) & 0x08) != 0;
	return (n & 0xF) == 0;
}

char* i8080_decToBin(uint16_t n) {
	int c, d, count;
	char* pointer;

	count = 0;
	pointer = (char*)malloc(16 + 1);

	if (pointer == NULL)
		return NULL;

	for (c = 15; c >= 0; c--) {
		d = n >> c;

		if (d & 1)
			* (pointer + count) = 1 + '0';
		else
			*(pointer + count) = 0 + '0';

		count++;
	}
	*(pointer + count) = '\0';

	return  pointer;
}
