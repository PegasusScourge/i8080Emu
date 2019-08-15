/*

i8080.h

CPU file

*/
#include "i8080_util.h"
#include "log.h"

#include <stdio.h>

// Process one cpu cycle of time length state->clockFreqMHz
void i8080_cpuTick(i8080State* state);

// Executes an opcode and changes the state accordingly. Massive switch statement function
bool i8080_executeOpcode(i8080State* state, uint8_t opcode);

// Outputs to the log that we have an unimplemented opcode
void unimplementedOpcode(i8080State* state, uint8_t opcode);

// Check for interrupts
void checkInterrupts(i8080State* state);

// Causes the processor to panic and halt execution immediately
void i8080_panic(i8080State* state);

// Dump the current state for debugging purposes
void i8080_dump(i8080State* state);

// Read the memory at index
uint8_t i8080op_readMemory(i8080State* state, uint16_t index);

// Writes to memory at the index
void i8080op_writeMemory(i8080State* state, uint16_t index, uint8_t value);

// Sets the PC
void i8080op_setPC(i8080State* state, uint16_t v);

// Sets the stack pointer
void i8080op_setSP(i8080State* state, uint16_t v);

// Pushes a value onto the stack
void i8080op_pushStack(i8080State* state, uint16_t v);

// Pops a value from the stack
uint16_t i8080op_popStack(i8080State* state);

// Peaks at the top value from the stack but does not pop
uint16_t i8080op_peakStack(i8080State* state);

// Causes the processor to RETURN. DO NOT increment PC before/after calling
void i8080op_executeRET(i8080State* state);

// Causes the processor to execute a call. DO NOT increment PC before/after calling. Increments the value push to the stack to go for the next instruction
void i8080op_executeCALL(i8080State* state, uint16_t address);

// Causes the processor to execute a call, but does not increment the value pushed to the stack making sure no instructions are lost
void i8080op_executeInterrupt(i8080State* state, uint16_t address);

// Returns the PSW
uint16_t i8080op_getPSW(i8080State* state);

// returns the 16 bit register BC
uint16_t i8080op_getBC(i8080State* state);

// returns the 16 bit register DE
uint16_t i8080op_getDE(i8080State* state);

// returns the 16 bit register HL
uint16_t i8080op_getHL(i8080State* state);

// Puts a value into BC
void i8080op_putBC8(i8080State* state, uint8_t ubyte, uint8_t lbyte);
void i8080op_putBC16(i8080State* state, uint16_t);

// Puts a value into DE
void i8080op_putDE8(i8080State* state, uint8_t ubyte, uint8_t lbyte);
void i8080op_putDE16(i8080State* state, uint16_t);

// Puts a value into HL
void i8080op_putHL8(i8080State* state, uint8_t ubyte, uint8_t lbyte);
void i8080op_i8080op_putHL16(i8080State* state, uint16_t);

// Sets the Z, S, P flags accordingly
void i8080op_setZSP(i8080State* state, uint8_t v);

// Uses an 8 bit var to reconstruct the flags
void i8080op_putFlags(i8080State* state, uint8_t fv);

// Rotates the number left one bit and stores the dropped bit in bit 0 and the carry flag
uint8_t i8080op_rotateBitwiseLeft(i8080State* state, uint8_t v);

// Rotates the number right one bit and stores the dropped bit in bit 7 and the carry flag
uint8_t i8080op_rotateBitwiseRight(i8080State* state, uint8_t v);

// Adds two 16 bit numbers and sets the carry flag as appropriate
uint16_t i8080op_addCarry16(i8080State* state, uint16_t a, uint16_t b);

// Adds two 8 bit numbers and sets the carry flag as needed
uint8_t i8080op_addCarry8(i8080State* state, uint8_t a, uint8_t b);

// Subtracts two 16 bit numbers and sets the carry flag as appropriate
uint16_t i8080op_subCarry16(i8080State* state, uint16_t a, uint16_t b);

// Subtracts two 8 bit numbers and sets the carry flag as appropriate
uint8_t i8080op_subCarry8(i8080State* state, uint8_t a, uint8_t b);