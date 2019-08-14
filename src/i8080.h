/*

i8080.h

CPU file

*/
#include "i8080_util.h"
#include "log.h"

#include <stdio.h>

// Process one cpu cycle of time length state->clockFreqMHz
void cpuTick(i8080State* state);

// Executes an opcode and changes the state accordingly. Massive switch statement function
bool executeOpcode(i8080State* state, uint8_t opcode);

// Outputs to the log that we have an unimplemented opcode
void unimplementedOpcode(uint8_t opcode);

// Read the memory at index
uint8_t readMemory(i8080State* state, uint16_t index);

// Writes to memory at the index
void writeMemory(i8080State* state, uint16_t index, uint8_t value);

// Sets the PC
void setPC(i8080State* state, uint16_t v);

// Sets the stack pointer
void setSP(i8080State* state, uint16_t v);

// Pushes a value onto the stack
void pushStack(i8080State* state, uint16_t v);

// Pops a value from the stack
uint16_t popStack(i8080State* state);

// Peaks at the top value from the stack but does not pop
uint16_t peakStack(i8080State* state);

// Causes the processor to RETURN. DO NOT increment PC before/after calling
void executeRET(i8080State* state);

// Causes the processor to execute a call. DO NOT increment PC before/after calling. Increments the value push to the stack to go for the next instruction
void executeCALL(i8080State* state, uint16_t address);

// Causes the processor to execute a call, but does not increment the value pushed to the stack making sure no instructions are lost
void executeInterrupt(i8080State* state, uint16_t address);