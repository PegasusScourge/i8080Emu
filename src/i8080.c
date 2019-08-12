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

uint8_t readMemory(i8080State* state, uint16_t index) {
	// The bounds checking function raises any necessary flags in case of error
	if (boundsCheckMemIndex(state, index)) {
		return state->memory[index];
	}
	log_error("Attempted to read memory location %i (out of bounds)", index);
	return 0;
}

void writeMemory(i8080State* state, uint16_t index, uint8_t val) {
	// The bounds checking function raises any necessary flags in case of error
	if (boundsCheckMemIndex(state, index)) {
		state->memory[index] = val;
	}
	else {
		log_error("Attempted to set memory location %i (out of bounds) to %i", index, val);
	}
}

void setPC(i8080State* state, uint16_t v) {
	if (boundsCheckMemIndex(state, v)) {
		state->pc = v;
	}
	else {
		log_error("Attempted to set PC to %i which is out of bounds", v);
	}
}

void setSP(i8080State* state, uint16_t v) {
	if (boundsCheckMemIndex(state, v)) {
		state->sp = v;
	}
	else {
		log_error("Attempted to set SP to %i which is out of bounds", v);
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
	uint8_t store8_2;

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
		putBC(state, byte2, byte1);
		break;
	case STAX_B: // write value of A to memory[BC]
		log_trace("[%04X] STAX_B(%02X) BC:%04X A:%02X", state->pc, STAX_B, getBC(state), state->a);
		writeMemory(state, getBC(state), state->a);
		break;
	case INX_B: // Increment BC
		store16_1 = 1 + getBC(state);
		log_trace("[%04X] INX_B(%02X)", state->pc, INX_B);
		putBC(state, (store16_1 & 0xFF00) >> 8, (store16_1 & 0x00FF));
		break;
	case INR_B: // Increment B
		log_trace("[%04X] INR_B(%02X)", state->pc, INR_B);
		state->b = state->b + 1;
		setZSPAC(state, state->b);
		break;
	case DCR_B: // Decrement B
		log_trace("[%04X] DCR_B(%02X)", state->pc, DCR_B);
		state->b = state->b - 1;
		setZSPAC(state, state->b);
		break;
	case MVI_B: // Put byte1 into B
		log_trace("[%04X] MVI_B(%02X) %02X", state->pc, MVI_B, byte1);
		state->b = byte1;
		break;
	case RLC: // Bitshift and place the dropped bit in the carry flag and bit 0 of the new number
		log_trace("[%04X] RLC(%02X) %02X", state->pc, RLC, state->a);
		state->a = rotateBitwiseLeft(state, state->a);
		break;
	case DAD_B: // HL += BC
		log_trace("[%04X] DAD_B(%02X) %04X", state->pc, DAD_B, getBC(state));
		putHL16(state, addCarry16(state, getHL(state), getBC(state)));
		break;
	case LDAX_B: // Load memory pointed to by BC into A
		log_trace("[%04X] LDAX_B(%02X) %02X", state->pc, LDAX_B, readMemory(state, getBC(state)));
		state->a = readMemory(state, getBC(state));
		break;
	case DCX_B: // Decrement BC by 1
		log_trace("[%04X] DCX_B(%02X) %04X", state->pc, DCX_B, getBC(state));
		putBC16(state, getBC(state) - 1);
		break;
	case INR_C: // Increment C by 1
		log_trace("[%04X] INR_C(%02X) %02X", state->pc, INR_C, state->c);
		state->c = state->c + 1;
		setZSPAC(state, state->c);
		break;
	case DCR_C: // Decrement C by 1
		log_trace("[%04X] DCR_C(%02X) %02X", state->pc, DCR_C, state->c);
		state->c = state->c - 1;
		setZSPAC(state, state->c);
		break;
	case MVI_C: // Put byte1 into C
		log_trace("[%04X] MVI_C(%02X) %02X", state->pc, MVI_C, byte1);
		state->c = byte1;
		break;
	case RRC: // Bitshift and place the dropped bit in the carry flag and bit 7 of the new number
		log_trace("[%04X] RRC(%02X) %02X", state->pc, RRC, state->a);
		state->a = rotateBitwiseRight(state, state->a);
		break;
	case LXI_D: // put in DE D16
		log_trace("[%04X] LXI_D(%02X) D:%02X E:%02X", state->pc, LXI_D, byte2, byte1);
		putDE(state, byte2, byte1);
		break;
	case STAX_D: // write value of A to memory[DE]
		log_trace("[%04X] STAX_D(%02X) BC:%04X A:%02X", state->pc, STAX_D, getDE(state), state->a);
		writeMemory(state, getDE(state), state->a);
		break;
	case INX_D: // Increment DE
		log_trace("[%04X] INX_D(%02X)", state->pc, INX_D);
		putDE16(state, 1 + getDE(state));
		break;
	case INR_D: // Increment D
		log_trace("[%04X] INR_D(%02X)", state->pc, INR_D);
		state->d = state->d + 1;
		setZSPAC(state, state->d);
		break;
	case DCR_D: // Decrement D
		log_trace("[%04X] DCR_D(%02X)", state->pc, DCR_D);
		state->d = state->d - 1;
		setZSPAC(state, state->d);
		break;
	case MVI_D: // Put byte1 into D
		log_trace("[%04X] MVI_D(%02X) %02X", state->pc, MVI_D, byte1);
		state->d = byte1;
		break;
	case RAL: // Bitshift left 1 and use CY as bit 0, and store dropped bit 7 in CY after
		log_trace("[%04X] RAL(%02X) %02X", state->pc, RAL, state->a);
		store8_1 = (state->a >> 7); // Get the 7th bit
		state->a = (state->a << 1) | state->f.c; // Store the CY in 0th bit
		state->f.c = store8_1; // Store 7th bit the carry flag
		break;
	case DAD_D: // HL += DE
		log_trace("[%04X] DAD_D(%02X) %04X", state->pc, DAD_D, getDE(state));
		putHL16(state, addCarry16(state, getHL(state), getDE(state)));
		break;
	case LDAX_D: // Load memory pointed to by DE into A
		log_trace("[%04X] LDAX_D(%02X) %02X", state->pc, LDAX_D, readMemory(state, getDE(state)));
		state->a = readMemory(state, getDE(state));
		break;
	case DCX_D: // Decrement DE by 1
		log_trace("[%04X] DCX_D(%02X) %04X", state->pc, DCX_D, getBC(state));
		putDE16(state, getDE(state) - 1);
		break;
	case INR_E: // Increment E by 1
		log_trace("[%04X] INR_E(%02X) %02X", state->pc, INR_E, state->e);
		state->e = state->e + 1;
		setZSPAC(state, state->e);
		break;
	case DCR_E: // Decrement E by 1
		log_trace("[%04X] DCR_E(%02X) %02X", state->pc, DCR_E, state->e);
		state->e = state->e - 1;
		setZSPAC(state, state->e);
		break;
	case MVI_E: // Put byte1 into C
		log_trace("[%04X] MVI_E(%02X) %02X", state->pc, MVI_E, byte1);
		state->e = byte1;
		break;
	case RAR: // Bitshift and place the dropped bit in the carry flag, set bit 7 to old bit 7
		log_trace("[%04X] RRC(%02X) %02X", state->pc, RRC, state->a);
		store8_1 = state->a & 0x01; // Get the 0th bit
		store8_2 = state->a & 0x80; // Get the 7th bit
		state->f.c = store8_1; // Store it in the carry flag
		state->a = (state->a >> 1) | store8_2; // Store the 7th bit in position
		break;
	case LXI_H: // put in HL D16
		log_trace("[%04X] LXI_H(%02X) H:%02X L:%02X", state->pc, LXI_H, byte2, byte1);
		putHL(state, byte2, byte1);
		break;
	case SHLD: // write value of HL to memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] SHLD(%02X) %04X HL:%04X", state->pc, SHLD, store16_1, getHL(state));
		writeMemory(state, store16_1, state->l);
		writeMemory(state, store16_1 + 1, state->h);
		break;
	case INX_H: // Increment HL
		log_trace("[%04X] INX_H(%02X)", state->pc, INX_H);
		putHL16(state, 1 + getHL(state));
		break;
	case INR_H: // Increment H
		log_trace("[%04X] INR_H(%02X)", state->pc, INR_H);
		state->h = state->h + 1;
		setZSPAC(state, state->h);
		break;
	case DCR_H: // Decrement D
		log_trace("[%04X] DCR_H(%02X)", state->pc, DCR_H);
		state->h = state->h - 1;
		setZSPAC(state, state->h);
		break;
	case MVI_H: // Put byte1 into H
		log_trace("[%04X] MVI_H(%02X) %02X", state->pc, MVI_H, byte1);
		state->h = byte1;
		break;
	case DAA:
		// Special, throw a warning but NOP
		log_warn("[%04X] DAA(%02X) Special code: not implemented", state->pc, DAA);
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
