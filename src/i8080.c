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

		// Get the result of the opcode execution to determine the number of clock cycles we need to take
		bool success = executeOpcode(state, opcode);

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

void setSP(i8080State* state, uint16_t v) {
	if (boundsCheckMemIndex(state, v)) {
		state->sp = v;
	}
	else {
		// ERROR
	}
}

void pushStack(i8080State* state, uint16_t v) {
	writeMemory(state, state->sp, (v & 0xFF00) >> 8);
	writeMemory(state, state->sp - 1, v & 0x00FF);
	setSP(state, state->sp - 2);
}

uint16_t peakStack(i8080State* state) {
	return ((uint8_t)readMemory(state, state->sp + 2) << 8) + readMemory(state, state->sp + 1);
}

uint16_t popStack(i8080State* state) {
	uint16_t stckVal = peakStack(state);
	setSP(state, state->sp + 2);
	return stckVal;
}

bool executeOpcode(i8080State* state, uint8_t opcode) {
	bool success = true;
	bool pcShouldIncrement = true;

	int byteLen = getInstructionLength(opcode);

	uint8_t byte1 = 0;
	uint8_t byte2 = 0;

	uint16_t store16_1;
	uint8_t store8_1;

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
	case LXI_B: // put in BC D16
		log_trace("[%04X] LXI_B(%02X) B:%02X C:%02X", state->pc, LXI_SP, byte2, byte1);
		state->b = byte2;
		state->c = byte1;
		break;
	case STAX_B: // write value of A to memory[BC]
		store16_1 = (state->b << 8) + state->c;
		log_trace("[%04X] STAX_B(%02X) BC:%04X A:%02X", state->pc, STAX_B, store16_1, state->a);
		writeMemory(state, store16_1, state->a);
		break;
	case INX_B: // Increment BC
		store16_1 = 1 + (state->b << 8) + state->c;
		log_trace("[%04X] INX_B(%02X)", state->pc, INX_B);
		state->b = (store16_1 & 0xFF00) >> 8;
		state->c = (store16_1 & 0x00FF);
		break;
	case INR_B: // Increment B
		log_trace("[%04X] INR_B(%02X)", state->pc, INR_B);
		state->b = state->b + 1;
		state->f.z = isZero(state->b);
		state->f.s = isNegative(state->b);
		state->f.p = isParityEven(state->b);
		state->f.ac = shouldACFlag(state->b);
		break;
	case DCR_B: // Decrement B
		log_trace("[%04X] DCR_B(%02X)", state->pc, INR_B);
		state->b = state->b - 1;
		state->f.z = isZero(state->b);
		state->f.s = isNegative(state->b);
		state->f.p = isParityEven(state->b);
		state->f.ac = shouldACFlag(state->b);
		break;
	case MVI_B: // Put byte1 into B
		log_trace("[%04X] MVI_B(%02X) %02X", state->pc, MVI_B, byte1);
		state->b = byte1;
		break;
	case RLC: // Bitshift and place the dropped bit in the carry flag and bit 0 of the new number
		log_trace("[%04X] RLC(%02X) %02X", state->pc, RLC, state->a);
		store8_1 = (state->a >> 7); // Get the 7th bit
		state->f.c = store8_1; // Store it in the carry flag
		state->a = (state->a << 1) | store8_1; // Store the 7th bit in position
		break;
	case LXI_SP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // D16
		log_trace("[%04X] LXI_SP(%02X) %04X (%02X %02X)", state->pc, LXI_SP, store16_1, byte1, byte2);
		setSP(state, store16_1); // Set the sp to D16
		break;
	case JMP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JMP(%02X) %04X (%02X %02X)", state->pc, JMP, store16_1, byte1, byte2);
		setPC(state, store16_1); // Set the pc to jmpPos
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case CALL:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CALL(%02X) %04X (%02X %02X)", state->pc, CALL, store16_1, byte1, byte2);
		pushStack(state, state->pc);
		setPC(state, store16_1); // Set the pc to address
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
	log_warn("Unimplemented opcode %02X", opcode);
}
