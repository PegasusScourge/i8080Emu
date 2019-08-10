/*

i8080.h

CPU file

*/
#include "i8080_util.h"
#include "log.h"

#include <stdio.h>

// Process one cpu cycle of time length state->clockFreqMHz
void cpuTick(i8080State* state);

// Executes an opcode and changes the state accordingly
bool executeOpcode(i8080State* state, uint8_t opcode, int byteLen);

// Outputs to the log that we have an unimplemented opcode
void unimplementedOpcode(uint8_t opcode);

// Read the memory at index
uint8_t readMemory(i8080State* state, int index);

// Writes to memory at the index
void writeMemory(i8080State* state, int index, uint8_t value);

// Sets the PC
void setPC(i8080State* state, uint16_t v);