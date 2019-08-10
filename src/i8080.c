/*

i8080.c

CPU file

*/
#include "i8080.h"

void cpuTick(i8080State* state) {
	stateCheck(state); // verify the state is ok

	if (state->waitCycles == 0) {
		// We don't need to wait cycles
		uint8_t opcode = readMemory(state, state->pc);
		int byteLen = getInstructionLength(opcode);

		// Get the result of the opcode execution to determine the number of clock cycles we need to take
		bool success = executeOpcode(state, opcode, byteLen);

		// Put the correct wait time in clock cycles
		state->waitCycles = success ? getInstructionClockCycles(opcode) : getFailedInstructionClockCycles(opcode);
	}
	else {
		state->waitCycles--;

		// Do a bounds check in case the waitCycles variable goes out of range
		if (state->waitCycles < 0) {
			log_error("Somehow the waitCycles variable of the state went below 0");
			state->waitCycles = 0;
		}
	}
}

uint8_t readMemory(i8080State* state, int index) {
	// The bounds checking function raises any necessary flags in case of error
	if (boundsCheckMemIndex(state, index)) {
		return state->memory[index];
	}
	return 0;
}

void writeMemory(i8080State* state, int index, uint8_t val) {
	// The bounds checking function raises any necessary flags in case of error
	if (boundsCheckMemIndex(state, index)) {
		state->memory[index] = val;
	}
}

void setPC(i8080State* state, uint16_t v) {
	if (boundsCheckMemIndex(state, v)) {
		state->pc = v;
	}
	else {
		// ERROR
	}
}

bool executeOpcode(i8080State* state, uint8_t opcode, int byteLen) {
	bool success = true;
	bool pcShouldIncrement = true;

	uint8_t byte1 = 0;
	uint8_t byte2 = 0;

	uint16_t store16_1;

	// Load the extra bytes as needed
	if (byteLen >= 2) {
		byte1 = readMemory(state, state->pc + 1);
	}
	if (byteLen >= 3) {
		byte2 = readMemory(state, state->pc + 2);
	}

	switch (opcode) {
	case NOP: // Do nothing
		break;
	case JMP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JMP(%02X) %04X (%02X %02X)", state->pc, JMP, store16_1, byte1, byte2);
		setPC(state, store16_1); // Set the pc to jmpPos
		pcShouldIncrement = false; // Stop the auto increment
		break;
	default:
		unimplementedOpcode(opcode);
		break;
	}

	// Determine how we increment our pc
	if (pcShouldIncrement) {
		state->pc += byteLen;
	}

	return success;
}

void unimplementedOpcode(uint8_t opcode) {
	log_warn("Unimplemented instruction %02X", opcode);
}
