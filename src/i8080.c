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
		log_trace("[%04X] DCX_D(%02X) %04X", state->pc, DCX_D, getDE(state));
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
	case DAD_H: // HL += HL
		log_trace("[%04X] DAD_H(%02X) %04X", state->pc, DAD_H, getHL(state));
		putHL16(state, addCarry16(state, getHL(state), getHL(state)));
		break;
	case LHLD: // Load L with memory[store16_1] and H with memory[store16_1 + 1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] LHLD(%02X) %04X", state->pc, LHLD, store16_1);
		state->l = readMemory(state, store16_1);
		state->h = readMemory(state, store16_1 + 1);
		break;
	case DCX_H: // Decrement HL by 1
		log_trace("[%04X] DCX_H(%02X) %04X", state->pc, DCX_H, getHL(state));
		putHL16(state, getHL(state) - 1);
		break;
	case INR_L: // Increment L by 1
		log_trace("[%04X] INR_L(%02X) %02X", state->pc, INR_L, state->e);
		state->l = state->l + 1;
		setZSPAC(state, state->l);
		break;
	case DCR_L: // Decrement L by 1
		log_trace("[%04X] DCR_L(%02X) %02X", state->pc, DCR_L, state->e);
		state->l = state->l - 1;
		setZSPAC(state, state->l);
		break;
	case MVI_L: // Put byte1 into L
		log_trace("[%04X] MVI_L(%02X) %02X", state->pc, MVI_L, byte1);
		state->l = byte1;
		break;
	case CMA: // a set to bitwise not of a
		log_trace("[%04X] CMA(%02X) %02X", state->pc, CMA, state->a);
		state->a = ~state->a;
		break;
	case LXI_SP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // D16
		log_trace("[%04X] LXI_SP(%02X) %04X (%02X %02X)", state->pc, LXI_SP, store16_1, byte1, byte2);
		setSP(state, store16_1); // Set the sp to D16
		break;
	case STA: // write value of A to memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] STA(%02X) %04X A:%02X", state->pc, STA, store16_1, state->a);
		writeMemory(state, store16_1, state->a);
		break;
	case INX_SP:
		log_trace("[%04X] INX_SP(%02X) %04X", state->pc, INX_SP, state->sp);
		setSP(state, state->sp + 1);
		break;
	case INR_M:
		store8_1 = readMemory(state, getHL(state));
		log_trace("[%04X] INR_M(%02X) [%04X]%02X", state->pc, INR_M, getHL(state), store8_1);
		store8_1 += 1;
		setZSPAC(state, store8_1);
		writeMemory(state, getHL(state), store8_1);
		break;
	case DCR_M:
		store8_1 = readMemory(state, getHL(state));
		log_trace("[%04X] DCR_M(%02X) [%04X]%02X", state->pc, DCR_M, getHL(state), store8_1);
		store8_1 -= 1;
		setZSPAC(state, store8_1);
		writeMemory(state, getHL(state), store8_1);
		break;
	case MVI_M: // Put byte1 into memory[HL]
		log_trace("[%04X] MVI_M(%02X) %02X --> [%04X]", state->pc, MVI_M, byte1, getHL(state));
		writeMemory(state, getHL(state), byte1);
		break;
	case STC:
		log_trace("[%04X] STC(%02X) 1", state->pc, STC);
		state->f.c = 1;
		break;
	case DAD_SP: // HL += SP
		log_trace("[%04X] DAD_SP(%02X) %04X", state->pc, DAD_SP, state->sp);
		putHL16(state, addCarry16(state, getHL(state), state->sp));
		break;
	case LDA: // load value of A from memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] LDA(%02X) %04X", state->pc, LDA, store16_1);
		state->a = readMemory(state, store16_1);
		break;
	case DCX_SP:
		log_trace("[%04X] DCX_SP(%02X) %04X", state->pc, DCX_SP, state->sp);
		setSP(state, state->sp - 1);
		break;
	case INR_A:
		log_trace("[%04X] INR_A(%02X) %02X", state->pc, INR_A, state->a);
		state->a = state->a + 1;
		setZSPAC(state, state->a);
		break;
	case DCR_A:
		log_trace("[%04X] DCR_A(%02X) %02X", state->pc, DCR_A, state->a);
		state->a = state->a - 1;
		setZSPAC(state, state->a);
		break;
	case MVI_A: // Put byte1 into A
		log_trace("[%04X] MVI_A(%02X) %02X", state->pc, MVI_A, byte1);
		state->a = byte1;
		break;
	case CMC:
		log_trace("[%04X] CMC(%02X)", state->pc, CMC);
		state->f.c = ~state->f.c;
		break;
	case MOV_BB:
		log_trace("[%04X] MOV_BB(%02X)", state->pc, MOV_BB);
		state->b = state->b;
		break;
	case MOV_BC:
		log_trace("[%04X] MOV_BC(%02X)", state->pc, MOV_BC);
		state->b = state->c;
		break;
	case MOV_BD:
		log_trace("[%04X] MOV_BD(%02X)", state->pc, MOV_BD);
		state->b = state->d;
		break;
	case MOV_BE:
		log_trace("[%04X] MOV_BE(%02X)", state->pc, MOV_BE);
		state->b = state->e;
		break;
	case MOV_BH:
		log_trace("[%04X] MOV_BH(%02X)", state->pc, MOV_BH);
		state->b = state->h;
		break;
	case MOV_BL:
		log_trace("[%04X] MOV_BL(%02X)", state->pc, MOV_BL);
		state->b = state->l;
		break;
	case MOV_BM:
		log_trace("[%04X] MOV_BM(%02X) %04X", state->pc, MOV_BM, getHL(state));
		state->b = readMemory(state, getHL(state));
		break;
	case MOV_BA:
		log_trace("[%04X] MOV_BA(%02X)", state->pc, MOV_BA);
		state->b = state->a;
		break;
	case MOV_CB:
		log_trace("[%04X] MOV_CB(%02X)", state->pc, MOV_CB);
		state->c = state->b;
		break;
	case MOV_CC:
		log_trace("[%04X] MOV_CC(%02X)", state->pc, MOV_CC);
		state->c = state->c;
		break;
	case MOV_CD:
		log_trace("[%04X] MOV_CD(%02X)", state->pc, MOV_CD);
		state->c = state->d;
		break;
	case MOV_CE:
		log_trace("[%04X] MOV_CE(%02X)", state->pc, MOV_CE);
		state->c = state->e;
		break;
	case MOV_CH:
		log_trace("[%04X] MOV_CH(%02X)", state->pc, MOV_CH);
		state->c = state->h;
		break;
	case MOV_CL:
		log_trace("[%04X] MOV_CL(%02X)", state->pc, MOV_CL);
		state->c = state->l;
		break;
	case MOV_CM:
		log_trace("[%04X] MOV_CM(%02X) %04X", state->pc, MOV_CM, getHL(state));
		state->c = readMemory(state, getHL(state));
		break;
	case MOV_CA:
		log_trace("[%04X] MOV_CA(%02X)", state->pc, MOV_CA);
		state->c = state->a;
		break;
	case MOV_DB:
		log_trace("[%04X] MOV_DB(%02X)", state->pc, MOV_DB);
		state->d = state->b;
		break;
	case MOV_DC:
		log_trace("[%04X] MOV_DC(%02X)", state->pc, MOV_DC);
		state->d = state->c;
		break;
	case MOV_DD:
		log_trace("[%04X] MOV_CD(%02X)", state->pc, MOV_DD);
		state->d = state->d;
		break;
	case MOV_DE:
		log_trace("[%04X] MOV_DE(%02X)", state->pc, MOV_DE);
		state->d = state->e;
		break;
	case MOV_DH:
		log_trace("[%04X] MOV_DH(%02X)", state->pc, MOV_DH);
		state->d = state->h;
		break;
	case MOV_DL:
		log_trace("[%04X] MOV_DL(%02X)", state->pc, MOV_DL);
		state->d = state->l;
		break;
	case MOV_DM:
		log_trace("[%04X] MOV_DM(%02X) %04X", state->pc, MOV_DM, getHL(state));
		state->d = readMemory(state, getHL(state));
		break;
	case MOV_DA:
		log_trace("[%04X] MOV_DA(%02X)", state->pc, MOV_DA);
		state->d = state->a;
		break;
	case MOV_EB:
		log_trace("[%04X] MOV_EB(%02X)", state->pc, MOV_EB);
		state->e = state->b;
		break;
	case MOV_EC:
		log_trace("[%04X] MOV_EC(%02X)", state->pc, MOV_EC);
		state->e = state->c;
		break;
	case MOV_ED:
		log_trace("[%04X] MOV_ED(%02X)", state->pc, MOV_ED);
		state->e = state->d;
		break;
	case MOV_EE:
		log_trace("[%04X] MOV_EE(%02X)", state->pc, MOV_EE);
		state->e = state->e;
		break;
	case MOV_EH:
		log_trace("[%04X] MOV_EH(%02X)", state->pc, MOV_EH);
		state->e = state->h;
		break;
	case MOV_EL:
		log_trace("[%04X] MOV_EL(%02X)", state->pc, MOV_EL);
		state->e = state->l;
		break;
	case MOV_EM:
		log_trace("[%04X] MOV_EM(%02X) %04X", state->pc, MOV_EM, getHL(state));
		state->e = readMemory(state, getHL(state));
		break;
	case MOV_EA:
		log_trace("[%04X] MOV_EA(%02X) (REEEEEE)", state->pc, MOV_EA);
		state->e = state->a;
		break;
	case MOV_HB:
		log_trace("[%04X] MOV_HB(%02X)", state->pc, MOV_HB);
		state->h = state->b;
		break;
	case MOV_HC:
		log_trace("[%04X] MOV_HC(%02X)", state->pc, MOV_HC);
		state->h = state->c;
		break;
	case MOV_HD:
		log_trace("[%04X] MOV_HD(%02X)", state->pc, MOV_HD);
		state->h = state->d;
		break;
	case MOV_HE:
		log_trace("[%04X] MOV_HE(%02X)", state->pc, MOV_HE);
		state->h = state->e;
		break;
	case MOV_HH:
		log_trace("[%04X] MOV_HH(%02X)", state->pc, MOV_HH);
		state->h = state->h;
		break;
	case MOV_HL:
		log_trace("[%04X] MOV_HL(%02X)", state->pc, MOV_HL);
		state->h = state->l;
		break;
	case MOV_HM:
		log_trace("[%04X] MOV_HM(%02X) %04X", state->pc, MOV_HM, getHL(state));
		state->h = readMemory(state, getHL(state));
		break;
	case MOV_HA:
		log_trace("[%04X] MOV_HA(%02X) (HA!)", state->pc, MOV_HA);
		state->e = state->a;
		break;
	case MOV_LB:
		log_trace("[%04X] MOV_LB(%02X)", state->pc, MOV_LB);
		state->l = state->b;
		break;
	case MOV_LC:
		log_trace("[%04X] MOV_LC(%02X)", state->pc, MOV_LC);
		state->l = state->c;
		break;
	case MOV_LD:
		log_trace("[%04X] MOV_LD(%02X)", state->pc, MOV_LD);
		state->l = state->d;
		break;
	case MOV_LE:
		log_trace("[%04X] MOV_LE(%02X)", state->pc, MOV_LE);
		state->l = state->e;
		break;
	case MOV_LH:
		log_trace("[%04X] MOV_LH(%02X)", state->pc, MOV_LH);
		state->l = state->h;
		break;
	case MOV_LL:
		log_trace("[%04X] MOV_LL(%02X)", state->pc, MOV_LL);
		state->l = state->l;
		break;
	case MOV_LM:
		log_trace("[%04X] MOV_LM(%02X) %04X", state->pc, MOV_LM, getHL(state));
		state->l = readMemory(state, getHL(state));
		break;
	case MOV_LA:
		log_trace("[%04X] MOV_LA(%02X) (la la la la la la la)", state->pc, MOV_LA);
		state->l = state->a;
		break;
	case MOV_MB:
		log_trace("[%04X] MOV_MB(%02X)", state->pc, MOV_MB);
		writeMemory(state, getHL(state), state->b);
		break;
	case MOV_MC:
		log_trace("[%04X] MOV_MC(%02X)", state->pc, MOV_MC);
		writeMemory(state, getHL(state), state->c);
		break;
	case MOV_MD:
		log_trace("[%04X] MOV_MD(%02X)", state->pc, MOV_MD);
		writeMemory(state, getHL(state), state->d);
		break;
	case MOV_ME:
		log_trace("[%04X] MOV_ME(%02X)", state->pc, MOV_ME);
		writeMemory(state, getHL(state), state->e);
		break;
	case MOV_MH:
		log_trace("[%04X] MOV_MH(%02X)", state->pc, MOV_MH);
		writeMemory(state, getHL(state), state->h);
		break;
	case MOV_ML:
		log_trace("[%04X] MOV_ML(%02X)", state->pc, MOV_ML);
		writeMemory(state, getHL(state), state->h);
		break;
	case MOV_MA:
		log_trace("[%04X] MOV_MA(%02X)", state->pc, MOV_MA);
		writeMemory(state, getHL(state), state->a);
		break;
	case HLT:
		// HALT THE PROGRAM?
		log_info("[%04X] HLT(%02X)", state->pc, HLT);
		state->valid = false;
		break;
	case MOV_AB:
		log_trace("[%04X] MOV_AB(%02X)", state->pc, MOV_AB);
		state->a = state->b;
		break;
	case MOV_AC:
		log_trace("[%04X] MOV_AC(%02X)", state->pc, MOV_AC);
		state->a = state->c;
		break;
	case MOV_AD:
		log_trace("[%04X] MOV_AD(%02X)", state->pc, MOV_AD);
		state->a = state->d;
		break;
	case MOV_AE:
		log_trace("[%04X] MOV_AE(%02X)", state->pc, MOV_AE);
		state->a = state->e;
		break;
	case MOV_AH:
		log_trace("[%04X] MOV_AH(%02X)", state->pc, MOV_AH);
		state->a = state->h;
		break;
	case MOV_AL:
		log_trace("[%04X] MOV_AL(%02X)", state->pc, MOV_AL);
		state->a = state->l;
		break;
	case MOV_AM:
		log_trace("[%04X] MOV_AM(%02X) %04X", state->pc, MOV_AM, getHL(state));
		state->l = readMemory(state, getHL(state));
		break;
	case MOV_AA:
		log_trace("[%04X] MOV_AA(%02X) (British car recovery joke or screaming?)", state->pc, MOV_AA);
		state->a = state->a;
		break;
	case ADD_B: // Adds B to A
		log_trace("[%04X] ADD_B(%02X)", state->pc, ADD_B);
		state->a = addCarry(state, state->a, state->b);
		setZSPAC(state, state->a);
		break;
	case ADD_C: // Adds C to A
		log_trace("[%04X] ADD_C(%02X)", state->pc, ADD_C);
		state->a = addCarry(state, state->a, state->c);
		setZSPAC(state, state->a);
		break;
	case ADD_D: // Adds D to A
		log_trace("[%04X] ADD_D(%02X)", state->pc, ADD_D);
		state->a = addCarry(state, state->a, state->d);
		setZSPAC(state, state->a);
		break;
	case ADD_E: // Adds E to A
		log_trace("[%04X] ADD_E(%02X)", state->pc, ADD_E);
		state->a = addCarry(state, state->a, state->e);
		setZSPAC(state, state->a);
		break;
	case ADD_H: // Adds H to A
		log_trace("[%04X] ADD_H(%02X)", state->pc, ADD_H);
		state->a = addCarry(state, state->a, state->h);
		setZSPAC(state, state->a);
		break;
	case ADD_L: // Adds L to A
		log_trace("[%04X] ADD_L(%02X)", state->pc, ADD_L);
		state->a = addCarry(state, state->a, state->l);
		setZSPAC(state, state->a);
		break;
	case ADD_M: // Adds memory[HL] to A
		log_trace("[%04X] ADD_M(%02X)", state->pc, ADD_M);
		state->a = addCarry(state, state->a, readMemory(state, getHL(state)));
		setZSPAC(state, state->a);
		break;
	case ADD_A: // Adds A to A
		log_trace("[%04X] ADD_A(%02X)", state->pc, ADD_A);
		state->a = addCarry(state, state->a, state->a);
		setZSPAC(state, state->a);
		break;
	case ADC_B: // Adds B to A
		log_trace("[%04X] ADC_B(%02X)", state->pc, ADC_B);
		state->a = addCarry(state, addCarry(state, state->a, state->b), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_C: // Adds C to A
		log_trace("[%04X] ADC_C(%02X)", state->pc, ADC_C);
		state->a = addCarry(state, addCarry(state, state->a, state->c), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_D: // Adds D to A
		log_trace("[%04X] ADC_D(%02X)", state->pc, ADC_D);
		state->a = addCarry(state, addCarry(state, state->a, state->d), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_E: // Adds E to A
		log_trace("[%04X] ADC_E(%02X)", state->pc, ADC_E);
		state->a = addCarry(state, addCarry(state, state->a, state->e), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_H: // Adds H to A
		log_trace("[%04X] ADC_H(%02X)", state->pc, ADC_H);
		state->a = addCarry(state, addCarry(state, state->a, state->h), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_L: // Adds L to A
		log_trace("[%04X] ADC_L(%02X)", state->pc, ADC_L);
		state->a = addCarry(state, addCarry(state, state->a, state->l), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_M: // Adds memory[HL] to A
		log_trace("[%04X] ADC_M(%02X)", state->pc, ADC_M);
		state->a = addCarry(state, addCarry(state, state->a, readMemory(state, getHL(state))), state->f.c);
		setZSPAC(state, state->a);
		break;
	case ADC_A: // Adds A to A
		log_trace("[%04X] ADC_A(%02X)", state->pc, ADC_A);
		state->a = addCarry(state, addCarry(state, state->a, state->a), state->f.c);
		setZSPAC(state, state->a);
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
