/*

i8080.c

CPU file

*/
#include "i8080.h"

void i8080_cpuTick(i8080State* state) {
	i8080_stateCheck(state); // verify the state is ok

	if (state->waitCycles == 0) {
		// We don't need to wait cycles
		uint8_t opcode = i8080op_readMemory(state, state->pc);

		// Increment opcode use
		state->opcodeUse[opcode] = 1;
		// set the status string
		state->statusString = i8080_decompile(opcode);

		// Register for instruction tracing
		if (opcode != NOP) {
			for (int i = INSTRUCTION_TRACE_LEN - 1; i > 0; i--) {
				state->previousInstructions[i] = state->previousInstructions[i - 1];
			}
			state->previousInstructions[0].cycleNum = state->cyclesExecuted;
			state->previousInstructions[0].opcode = opcode;
			state->previousInstructions[0].b1 = i8080op_readMemory(state, state->pc + 1);
			state->previousInstructions[0].b2 = i8080op_readMemory(state, state->pc + 2);
			state->previousInstructions[0].pc = state->pc;
			state->previousInstructions[0].psw = i8080op_getPSW(state);
			state->previousInstructions[0].statusString = state->statusString;
			state->previousInstructions[0].topStack = i8080op_peakStack(state);
		}

		// Get the result of the opcode execution to determine the number of clock cycles we need to take
		bool success = i8080_executeOpcode(state, opcode);

		// Put the correct wait time in clock cycles
		state->waitCycles = success ? i8080_getInstructionClockCycles(opcode) : i8080_getFailedInstructionClockCycles(opcode);
	}
	else {
		state->waitCycles--;

		// Do a bounds check in case the waitCycles variable goes out of range
		if (state->waitCycles < 0) {
			log_error("Somehow the waitCycles variable of the state went below 0");
			state->waitCycles = 0;
		}
	}

	state->cyclesExecuted++;
}

void i8080_panic(i8080State* state) {
	log_fatal("i8080 PANIC has occured, cycle %ul", state->cyclesExecuted);
	state->mode = MODE_HLT; // set invalid

	i8080_dump(state);
}

void i8080_dump(i8080State* state) {
	// Dump the state
	FILE* dumpFile = fopen("i8080_dump.txt", "w");
	if (dumpFile == NULL) {
		log_error("Failed to create dump file for state and previous functions");
		return;
	}

	fprintf(dumpFile, "Registers:\nB:%02X C:%02X\nD:%02X E:%02X\nH:%02X L:%02X\nPSW:%04X (%s)\n", state->b, state->c, state->d, state->e, state->h, state->l, i8080op_getPSW(state), i8080_decToBin(i8080op_getPSW(state)));
	fprintf(dumpFile, "PC: %04X\nSP: %04X\n", state->pc, state->sp);
	fprintf(dumpFile, "------\nInstruction trace (newest instruction first):\n");

	// Print the last instructions
	for (int i = 0; i < INSTRUCTION_TRACE_LEN; i++) {
		fprintf(dumpFile, "{-%i}[%ul][PC:%04X] %s(%02X) (PSW:%04X, %s)(TS:%04X) B1:%02X B2:%02X\n", i, state->previousInstructions[i].cycleNum, state->previousInstructions[i].pc, state->previousInstructions[i].statusString, state->previousInstructions[i].opcode, state->previousInstructions[i].psw, i8080_decToBin(state->previousInstructions[i].psw), state->previousInstructions[i].topStack, state->previousInstructions[i].b1, state->previousInstructions[i].b2);
	}

	fprintf(dumpFile, "------\n\nMemory dump in file 'mem.dump'");
	fclose(dumpFile);

	// Create the memory dump file
	FILE* memDump = fopen("mem.dump", "w");
	if (memDump == NULL) {
		log_error("Failed to create dump file for memory");
		return;
	}

	fprintf(memDump, "Memory contents:");
	for (int i = 0; i < i8080_MEMORY_SIZE; i++) {
		if (i % 16 == 0) {
			fprintf(memDump, "\n[%04X] ", i);
		}
		fprintf(memDump, "%02X ", state->memory[i]);
	}
	fprintf(memDump, "\n\nEnd memory\n");
	fclose(memDump);
}

uint8_t i8080op_readMemory(i8080State* state, uint16_t index) {
	//breakpoint(state); // pause here to inspect state

	// The bounds checking function raises any necessary flags in case of error
	if (i8080_boundsCheckMemIndex(state, index)) {
		while (index > 0x3fff)
			index -= 0x4000;
		return state->memory[index];
	}
	log_error("Attempted to read memory location %i (out of bounds)", index);
	return 0;
}

void i8080op_writeMemory(i8080State* state, uint16_t index, uint8_t val) {
	//breakpoint(state); // pause here to inspect state

	// The bounds checking function raises any necessary flags in case of error
	if (i8080_boundsCheckMemIndex(state, index)) {
		while (index > 0x3fff)
			index -= 0x4000;
		state->memory[index] = val;
	}
	else {
		log_error("Attempted to set memory location %i (out of bounds) to %i", index, val);
	}
}

void i8080op_setPC(i8080State* state, uint16_t v) {
	//breakpoint(state); // pause here to inspect state

	if (i8080_boundsCheckMemIndex(state, v)) {
		state->pc = v;
	}
	else {
		log_error("Attempted to set PC to %i which is out of bounds", v);
	}
}

void i8080op_setSP(i8080State* state, uint16_t v) {
	if (i8080_boundsCheckMemIndex(state, v)) {
		state->sp = v;
	}
	else {
		log_error("Attempted to set SP to %i which is out of bounds", v);
	}
}

void i8080op_pushStack(i8080State* state, uint16_t v) {
	i8080op_writeMemory(state, state->sp - 1, (v & 0xFF00) >> 8);
	i8080op_writeMemory(state, state->sp - 2, v & 0x00FF);
	i8080op_setSP(state, state->sp - 2);
}

uint16_t i8080op_peakStack(i8080State* state) {
	return ((uint8_t)i8080op_readMemory(state, state->sp + 1) << 8) + i8080op_readMemory(state, state->sp);
}

uint16_t i8080op_popStack(i8080State* state) {
	uint16_t stckVal = i8080op_peakStack(state);
	i8080op_setSP(state, state->sp + 2);
	return stckVal;
}

uint8_t port_in(i8080State* state, uint8_t port) {

	//log_info("[%04X] IN(%02X) port==%i", state->pc, IN, port);

	if (port == 1) {
		return 0x01;
	}
	else if (port == 2) {
		return 0x00;
	}

	// If we don't understand, just return 0
	return 0x00;
}

void port_out(i8080State* state, uint8_t port, uint8_t value) {
	//log_info("[%04X] OUT(%02X) port==%i w/val %i", state->pc, OUT, port, value);
}

void i8080op_executeRET(i8080State* state) {
	//breakpoint(state); // pause here to inspect state

	uint16_t stckVal = i8080op_popStack(state);
	//uint8_t returningOpcode = i8080op_readMemory(state, stckVal);
	uint8_t returningOpcode = 0;
	//uint16_t pcInc = i8080_getInstructionLength(returningOpcode);
	uint16_t pcInc = 0;

	log_debug("i8080op_executeRET; stckVal: %04X, retOpcode: %02X, pcInc: %04X", stckVal, returningOpcode, pcInc);

	i8080op_setPC(state, stckVal + pcInc);

	if (state->f.isi)
		log_trace("--- END INTERRUPT ---");
	state->f.isi = 0; // clear isInterrupted bit
}

void i8080op_executeCALL(i8080State* state, uint16_t address) {
	//breakpoint(state); // pause here to inspect state

	uint8_t returningOpcode = i8080op_readMemory(state, state->pc);
	uint16_t pcInc = i8080_getInstructionLength(returningOpcode);
	i8080op_pushStack(state, state->pc + pcInc);
	i8080op_setPC(state, address); // Set the pc to address
}

void i8080op_executeInterrupt(i8080State* state, uint16_t address) {
	//breakpoint(state); // pause here to inspect state

	if (state->f.isi == 0 && state->f.ien) {
		log_trace("--- INTERRUPT %i ---", address);
		state->f.isi = address; // set isInterrupted bit
		i8080op_pushStack(state, state->pc);
		i8080op_setPC(state, address); // Set the pc to address
	}
}

bool i8080_executeOpcode(i8080State* state, uint8_t opcode) {
	bool success = true;
	bool pcShouldIncrement = true;

	int byteLen = i8080_getInstructionLength(opcode);

	uint8_t byte1 = 0;
	uint8_t byte2 = 0;

	uint16_t store16_1;
	uint8_t store8_1;
	uint8_t store8_2;

	// Load the extra bytes
	byte1 = i8080op_readMemory(state, state->pc + 1);
	byte2 = i8080op_readMemory(state, state->pc + 2);

	switch (opcode) {
	case NOP: // Do nothing
		log_trace("[%04X] NOP(%02X)", state->pc, NOP);
		break;
	case LXI_B: // put in BC D16
		log_trace("[%04X] LXI_B(%02X) B:%02X C:%02X", state->pc, LXI_SP, byte2, byte1);
		i8080op_putBC8(state, byte2, byte1);
		break;
	case STAX_B: // write value of A to memory[BC]
		log_trace("[%04X] STAX_B(%02X) BC:%04X A:%02X", state->pc, STAX_B, i8080op_getBC(state), state->a);
		i8080op_writeMemory(state, i8080op_getBC(state), state->a);
		break;
	case INX_B: // Increment BC
		store16_1 = 1 + i8080op_getBC(state);
		log_trace("[%04X] INX_B(%02X)", state->pc, INX_B);
		i8080op_putBC8(state, (store16_1 & 0xFF00) >> 8, (store16_1 & 0x00FF));
		break;
	case INR_B: // Increment B
		log_trace("[%04X] INR_B(%02X)", state->pc, INR_B);
		state->b = state->b + 1;
		i8080op_setZSPAC(state, state->b);
		break;
	case DCR_B: // Decrement B
		log_trace("[%04X] DCR_B(%02X)", state->pc, DCR_B);
		state->b = state->b - 1;
		i8080op_setZSPAC(state, state->b);
		break;
	case MVI_B: // Put byte1 into B
		log_trace("[%04X] MVI_B(%02X) %02X", state->pc, MVI_B, byte1);
		state->b = byte1;
		break;
	case RLC: // Bitshift and place the dropped bit in the carry flag and bit 0 of the new number
		log_trace("[%04X] RLC(%02X) %02X", state->pc, RLC, state->a);
		state->a = i8080op_rotateBitwiseLeft(state, state->a);
		break;
	case DAD_B: // HL += BC
		log_trace("[%04X] DAD_B(%02X) %04X", state->pc, DAD_B, i8080op_getBC(state));
		i8080op_i8080op_putHL16(state, i8080op_addCarry16(state, i8080op_getHL(state), i8080op_getBC(state)));
		break;
	case LDAX_B: // Load memory pointed to by BC into A
		log_trace("[%04X] LDAX_B(%02X) %02X", state->pc, LDAX_B, i8080op_readMemory(state, i8080op_getBC(state)));
		state->a = i8080op_readMemory(state, i8080op_getBC(state));
		break;
	case DCX_B: // Decrement BC by 1
		log_trace("[%04X] DCX_B(%02X) %04X", state->pc, DCX_B, i8080op_getBC(state));
		i8080op_putBC16(state, i8080op_getBC(state) - 1);
		break;
	case INR_C: // Increment C by 1
		log_trace("[%04X] INR_C(%02X) %02X", state->pc, INR_C, state->c);
		state->c = state->c + 1;
		i8080op_setZSPAC(state, state->c);
		break;
	case DCR_C: // Decrement C by 1
		log_trace("[%04X] DCR_C(%02X) %02X", state->pc, DCR_C, state->c);
		state->c = state->c - 1;
		i8080op_setZSPAC(state, state->c);
		break;
	case MVI_C: // Put byte1 into C
		log_trace("[%04X] MVI_C(%02X) %02X", state->pc, MVI_C, byte1);
		state->c = byte1;
		break;
	case RRC: // Bitshift and place the dropped bit in the carry flag and bit 7 of the new number
		log_trace("[%04X] RRC(%02X) %02X", state->pc, RRC, state->a);
		state->a = i8080op_rotateBitwiseRight(state, state->a);
		break;
	case LXI_D: // put in DE D16
		log_trace("[%04X] LXI_D(%02X) D:%02X E:%02X", state->pc, LXI_D, byte2, byte1);
		i8080op_putDE8(state, byte2, byte1);
		break;
	case STAX_D: // write value of A to memory[DE]
		log_trace("[%04X] STAX_D(%02X) BC:%04X A:%02X", state->pc, STAX_D, i8080op_getDE(state), state->a);
		i8080op_writeMemory(state, i8080op_getDE(state), state->a);
		break;
	case INX_D: // Increment DE
		log_trace("[%04X] INX_D(%02X)", state->pc, INX_D);
		i8080op_putDE16(state, 1 + i8080op_getDE(state));
		break;
	case INR_D: // Increment D
		log_trace("[%04X] INR_D(%02X)", state->pc, INR_D);
		state->d = state->d + 1;
		i8080op_setZSPAC(state, state->d);
		break;
	case DCR_D: // Decrement D
		log_trace("[%04X] DCR_D(%02X)", state->pc, DCR_D);
		state->d = state->d - 1;
		i8080op_setZSPAC(state, state->d);
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
		log_trace("[%04X] DAD_D(%02X) %04X", state->pc, DAD_D, i8080op_getDE(state));
		i8080op_i8080op_putHL16(state, i8080op_addCarry16(state, i8080op_getHL(state), i8080op_getDE(state)));
		break;
	case LDAX_D: // Load memory pointed to by DE into A
		log_trace("[%04X] LDAX_D(%02X) %02X", state->pc, LDAX_D, i8080op_readMemory(state, i8080op_getDE(state)));
		state->a = i8080op_readMemory(state, i8080op_getDE(state));
		break;
	case DCX_D: // Decrement DE by 1
		log_trace("[%04X] DCX_D(%02X) %04X", state->pc, DCX_D, i8080op_getDE(state));
		i8080op_putDE16(state, i8080op_getDE(state) - 1);
		break;
	case INR_E: // Increment E by 1
		log_trace("[%04X] INR_E(%02X) %02X", state->pc, INR_E, state->e);
		state->e = state->e + 1;
		i8080op_setZSPAC(state, state->e);
		break;
	case DCR_E: // Decrement E by 1
		log_trace("[%04X] DCR_E(%02X) %02X", state->pc, DCR_E, state->e);
		state->e = state->e - 1;
		i8080op_setZSPAC(state, state->e);
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
		i8080op_putHL8(state, byte2, byte1);
		break;
	case SHLD: // write value of HL to memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] SHLD(%02X) %04X HL:%04X", state->pc, SHLD, store16_1, i8080op_getHL(state));
		i8080op_writeMemory(state, store16_1, state->l);
		i8080op_writeMemory(state, store16_1 + 1, state->h);
		break;
	case INX_H: // Increment HL
		log_trace("[%04X] INX_H(%02X)", state->pc, INX_H);
		i8080op_i8080op_putHL16(state, 1 + i8080op_getHL(state));
		break;
	case INR_H: // Increment H
		log_trace("[%04X] INR_H(%02X)", state->pc, INR_H);
		state->h = state->h + 1;
		i8080op_setZSPAC(state, state->h);
		break;
	case DCR_H: // Decrement D
		log_trace("[%04X] DCR_H(%02X)", state->pc, DCR_H);
		state->h = state->h - 1;
		i8080op_setZSPAC(state, state->h);
		break;
	case MVI_H: // Put byte1 into H
		log_trace("[%04X] MVI_H(%02X) %02X", state->pc, MVI_H, byte1);
		state->h = byte1;
		break;
	case DAA:
		// Special, throw a warning but NOP
		log_trace("[%04X] DAA(%02X)", state->pc, DAA);
		if ((state->a & 0xF) > 0x9 || state->f.ac == 1)
			state->a = state->a + 6;
		state->f.ac = i8080_shouldACFlag(state->a);
		if ((state->a & 0xF0) >> 8 > 0x9 || state->f.c == 1)
			state->a = i8080op_addCarry8(state, state->a, 0x60);
		break;
	case DAD_H: // HL += HL
		log_trace("[%04X] DAD_H(%02X) %04X", state->pc, DAD_H, i8080op_getHL(state));
		i8080op_i8080op_putHL16(state, i8080op_addCarry16(state, i8080op_getHL(state), i8080op_getHL(state)));
		break;
	case LHLD: // Load L with memory[store16_1] and H with memory[store16_1 + 1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] LHLD(%02X) %04X", state->pc, LHLD, store16_1);
		state->l = i8080op_readMemory(state, store16_1);
		state->h = i8080op_readMemory(state, store16_1 + 1);
		break;
	case DCX_H: // Decrement HL by 1
		log_trace("[%04X] DCX_H(%02X) %04X", state->pc, DCX_H, i8080op_getHL(state));
		i8080op_i8080op_putHL16(state, i8080op_getHL(state) - 1);
		break;
	case INR_L: // Increment L by 1
		log_trace("[%04X] INR_L(%02X) %02X", state->pc, INR_L, state->e);
		state->l = state->l + 1;
		i8080op_setZSPAC(state, state->l);
		break;
	case DCR_L: // Decrement L by 1
		log_trace("[%04X] DCR_L(%02X) %02X", state->pc, DCR_L, state->e);
		state->l = state->l - 1;
		i8080op_setZSPAC(state, state->l);
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
		i8080op_setSP(state, store16_1); // Set the sp to D16
		break;
	case STA: // write value of A to memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] STA(%02X) %04X A:%02X", state->pc, STA, store16_1, state->a);
		i8080op_writeMemory(state, store16_1, state->a);
		break;
	case INX_SP:
		log_trace("[%04X] INX_SP(%02X) %04X", state->pc, INX_SP, state->sp);
		i8080op_setSP(state, state->sp + 1);
		break;
	case INR_M:
		store8_1 = i8080op_readMemory(state, i8080op_getHL(state));
		log_trace("[%04X] INR_M(%02X) [%04X]%02X", state->pc, INR_M, i8080op_getHL(state), store8_1);
		store8_1 += 1;
		i8080op_setZSPAC(state, store8_1);
		i8080op_writeMemory(state, i8080op_getHL(state), store8_1);
		break;
	case DCR_M:
		store8_1 = i8080op_readMemory(state, i8080op_getHL(state));
		log_trace("[%04X] DCR_M(%02X) [%04X]%02X", state->pc, DCR_M, i8080op_getHL(state), store8_1);
		store8_1 -= 1;
		i8080op_setZSPAC(state, store8_1);
		i8080op_writeMemory(state, i8080op_getHL(state), store8_1);
		break;
	case MVI_M: // Put byte1 into memory[HL]
		log_trace("[%04X] MVI_M(%02X) %02X --> [%04X]", state->pc, MVI_M, byte1, i8080op_getHL(state));
		i8080op_writeMemory(state, i8080op_getHL(state), byte1);
		break;
	case STC:
		log_trace("[%04X] STC(%02X) 1", state->pc, STC);
		state->f.c = 1;
		break;
	case DAD_SP: // HL += SP
		log_trace("[%04X] DAD_SP(%02X) %04X", state->pc, DAD_SP, state->sp);
		i8080op_i8080op_putHL16(state, i8080op_addCarry16(state, i8080op_getHL(state), state->sp));
		break;
	case LDA: // load value of A from memory[store16_1]
		store16_1 = (uint16_t)byte1 + (((uint16_t)byte2) << 8);
		log_trace("[%04X] LDA(%02X) %04X", state->pc, LDA, store16_1);
		state->a = i8080op_readMemory(state, store16_1);
		break;
	case DCX_SP:
		log_trace("[%04X] DCX_SP(%02X) %04X", state->pc, DCX_SP, state->sp);
		i8080op_setSP(state, state->sp - 1);
		break;
	case INR_A:
		log_trace("[%04X] INR_A(%02X) %02X", state->pc, INR_A, state->a);
		state->a = state->a + 1;
		i8080op_setZSPAC(state, state->a);
		break;
	case DCR_A:
		log_trace("[%04X] DCR_A(%02X) %02X", state->pc, DCR_A, state->a);
		state->a = state->a - 1;
		i8080op_setZSPAC(state, state->a);
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
		log_trace("[%04X] MOV_BM(%02X) %04X", state->pc, MOV_BM, i8080op_getHL(state));
		state->b = i8080op_readMemory(state, i8080op_getHL(state));
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
		log_trace("[%04X] MOV_CM(%02X) %04X", state->pc, MOV_CM, i8080op_getHL(state));
		state->c = i8080op_readMemory(state, i8080op_getHL(state));
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
		log_trace("[%04X] MOV_DM(%02X) %04X", state->pc, MOV_DM, i8080op_getHL(state));
		state->d = i8080op_readMemory(state, i8080op_getHL(state));
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
		log_trace("[%04X] MOV_EM(%02X) %04X", state->pc, MOV_EM, i8080op_getHL(state));
		state->e = i8080op_readMemory(state, i8080op_getHL(state));
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
		log_trace("[%04X] MOV_HM(%02X) %04X", state->pc, MOV_HM, i8080op_getHL(state));
		state->h = i8080op_readMemory(state, i8080op_getHL(state));
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
		log_trace("[%04X] MOV_LM(%02X) %04X", state->pc, MOV_LM, i8080op_getHL(state));
		state->l = i8080op_readMemory(state, i8080op_getHL(state));
		break;
	case MOV_LA:
		log_trace("[%04X] MOV_LA(%02X) (la la la la la la la)", state->pc, MOV_LA);
		state->l = state->a;
		break;
	case MOV_MB:
		log_trace("[%04X] MOV_MB(%02X)", state->pc, MOV_MB);
		i8080op_writeMemory(state, i8080op_getHL(state), state->b);
		break;
	case MOV_MC:
		log_trace("[%04X] MOV_MC(%02X)", state->pc, MOV_MC);
		i8080op_writeMemory(state, i8080op_getHL(state), state->c);
		break;
	case MOV_MD:
		log_trace("[%04X] MOV_MD(%02X)", state->pc, MOV_MD);
		i8080op_writeMemory(state, i8080op_getHL(state), state->d);
		break;
	case MOV_ME:
		log_trace("[%04X] MOV_ME(%02X)", state->pc, MOV_ME);
		i8080op_writeMemory(state, i8080op_getHL(state), state->e);
		break;
	case MOV_MH:
		log_trace("[%04X] MOV_MH(%02X)", state->pc, MOV_MH);
		i8080op_writeMemory(state, i8080op_getHL(state), state->h);
		break;
	case MOV_ML:
		log_trace("[%04X] MOV_ML(%02X)", state->pc, MOV_ML);
		i8080op_writeMemory(state, i8080op_getHL(state), state->h);
		break;
	case MOV_MA:
		log_trace("[%04X] MOV_MA(%02X)", state->pc, MOV_MA);
		i8080op_writeMemory(state, i8080op_getHL(state), state->a);
		break;
	case HLT:
		// HALT THE PROGRAM?
		log_info("[%04X] HLT(%02X)", state->pc, HLT);
		state->mode = MODE_HLT;
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
		log_trace("[%04X] MOV_AM(%02X) %04X", state->pc, MOV_AM, i8080op_getHL(state));
		state->l = i8080op_readMemory(state, i8080op_getHL(state));
		break;
	case MOV_AA:
		log_trace("[%04X] MOV_AA(%02X) (British car recovery joke or screaming?)", state->pc, MOV_AA);
		state->a = state->a;
		break;
	case ADD_B: // Adds B to A
		log_trace("[%04X] ADD_B(%02X)", state->pc, ADD_B);
		state->a = i8080op_addCarry8(state, state->a, state->b);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_C: // Adds C to A
		log_trace("[%04X] ADD_C(%02X)", state->pc, ADD_C);
		state->a = i8080op_addCarry8(state, state->a, state->c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_D: // Adds D to A
		log_trace("[%04X] ADD_D(%02X)", state->pc, ADD_D);
		state->a = i8080op_addCarry8(state, state->a, state->d);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_E: // Adds E to A
		log_trace("[%04X] ADD_E(%02X)", state->pc, ADD_E);
		state->a = i8080op_addCarry8(state, state->a, state->e);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_H: // Adds H to A
		log_trace("[%04X] ADD_H(%02X)", state->pc, ADD_H);
		state->a = i8080op_addCarry8(state, state->a, state->h);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_L: // Adds L to A
		log_trace("[%04X] ADD_L(%02X)", state->pc, ADD_L);
		state->a = i8080op_addCarry8(state, state->a, state->l);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_M: // Adds memory[HL] to A
		log_trace("[%04X] ADD_M(%02X)", state->pc, ADD_M);
		state->a = i8080op_addCarry8(state, state->a, i8080op_readMemory(state, i8080op_getHL(state)));
		i8080op_setZSPAC(state, state->a);
		break;
	case ADD_A: // Adds A to A
		log_trace("[%04X] ADD_A(%02X)", state->pc, ADD_A);
		state->a = i8080op_addCarry8(state, state->a, state->a);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_B: // Adds B to A
		log_trace("[%04X] ADC_B(%02X)", state->pc, ADC_B);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->b), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_C: // Adds C to A
		log_trace("[%04X] ADC_C(%02X)", state->pc, ADC_C);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->c), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_D: // Adds D to A
		log_trace("[%04X] ADC_D(%02X)", state->pc, ADC_D);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->d), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_E: // Adds E to A
		log_trace("[%04X] ADC_E(%02X)", state->pc, ADC_E);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->e), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_H: // Adds H to A
		log_trace("[%04X] ADC_H(%02X)", state->pc, ADC_H);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->h), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_L: // Adds L to A
		log_trace("[%04X] ADC_L(%02X)", state->pc, ADC_L);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->l), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_M: // Adds memory[HL] to A
		log_trace("[%04X] ADC_M(%02X)", state->pc, ADC_M);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, i8080op_readMemory(state, i8080op_getHL(state))), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ADC_A: // Adds A to A
		log_trace("[%04X] ADC_A(%02X)", state->pc, ADC_A);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, state->a), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_B: // takes B from A
		log_trace("[%04X] SUB_B(%02X)", state->pc, SUB_B);
		state->a = i8080op_subCarry8(state, state->a, state->b);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_C: // takes C from A
		log_trace("[%04X] SUB_C(%02X)", state->pc, SUB_C);
		state->a = i8080op_subCarry8(state, state->a, state->c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_D: // takes D from A
		log_trace("[%04X] SUB_D(%02X)", state->pc, SUB_D);
		state->a = i8080op_subCarry8(state, state->a, state->d);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_E: // takes E from A
		log_trace("[%04X] SUB_E(%02X)", state->pc, SUB_E);
		state->a = i8080op_subCarry8(state, state->a, state->e);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_H: // takes H from A
		log_trace("[%04X] SUB_H(%02X)", state->pc, SUB_H);
		state->a = i8080op_subCarry8(state, state->a, state->h);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_L: // takes L from A
		log_trace("[%04X] SUB_L(%02X)", state->pc, SUB_L);
		state->a = i8080op_subCarry8(state, state->a, state->l);
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_M: // takes memory[HL] from A
		log_trace("[%04X] SUB_M(%02X)", state->pc, SUB_M);
		state->a = i8080op_subCarry8(state, state->a, i8080op_readMemory(state, i8080op_getHL(state)));
		i8080op_setZSPAC(state, state->a);
		break;
	case SUB_A: // takes A from A
		log_trace("[%04X] SUB_A(%02X)", state->pc, SUB_A);
		state->a = i8080op_subCarry8(state, state->a, state->a);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_B:
		log_trace("[%04X] SBB_B(%02X)", state->pc, SBB_B);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->b), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_C:
		log_trace("[%04X] SBB_C(%02X)", state->pc, SBB_C);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->c), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_D:
		log_trace("[%04X] SBB_D(%02X)", state->pc, SBB_D);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->d), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_E:
		log_trace("[%04X] SBB_E(%02X)", state->pc, SBB_E);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->e), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_H:
		log_trace("[%04X] SBB_H(%02X)", state->pc, SBB_H);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->h), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_L:
		log_trace("[%04X] SBB_L(%02X)", state->pc, SBB_L);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->l), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_M:
		log_trace("[%04X] SBB_M(%02X)", state->pc, SBB_M);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, i8080op_readMemory(state, i8080op_getHL(state))), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case SBB_A:
		log_trace("[%04X] SBB_A(%02X)", state->pc, SBB_A);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, state->a), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_B:
		log_trace("[%04X] ANA_B(%02X)", state->pc, ANA_B);
		state->a = state->a & state->b;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_C:
		log_trace("[%04X] ANA_C(%02X)", state->pc, ANA_C);
		state->a = state->a & state->c;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_D:
		log_trace("[%04X] ANA_D(%02X)", state->pc, ANA_D);
		state->a = state->a & state->d;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_E:
		log_trace("[%04X] ANA_E(%02X)", state->pc, ANA_E);
		state->a = state->a & state->e;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_H:
		log_trace("[%04X] ANA_H(%02X)", state->pc, ANA_H);
		state->a = state->a & state->h;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_L:
		log_trace("[%04X] ANA_L(%02X)", state->pc, ANA_L);
		state->a = state->a & state->l;
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_M:
		log_trace("[%04X] ANA_L(%02X)", state->pc, ANA_M);
		state->a = state->a & i8080op_readMemory(state, i8080op_getHL(state));
		i8080op_setZSPAC(state, state->a);
		break;
	case ANA_A:
		log_trace("[%04X] ANA_A(%02X)", state->pc, ANA_A);
		state->a = state->a & state->a;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_B:
		log_trace("[%04X] XRA_B(%02X)", state->pc, XRA_B);
		state->a = state->a ^ state->b;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_C:
		log_trace("[%04X] XRA_C(%02X)", state->pc, XRA_C);
		state->a = state->a ^ state->c;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_D:
		log_trace("[%04X] XRA_D(%02X)", state->pc, XRA_D);
		state->a = state->a ^ state->d;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_E:
		log_trace("[%04X] XRA_E(%02X)", state->pc, XRA_E);
		state->a = state->a ^ state->e;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_H:
		log_trace("[%04X] XRA_H(%02X)", state->pc, XRA_H);
		state->a = state->a ^ state->h;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_L:
		log_trace("[%04X] XRA_L(%02X)", state->pc, XRA_L);
		state->a = state->a ^ state->l;
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_M:
		log_trace("[%04X] XRA_M(%02X)", state->pc, XRA_M);
		state->a = state->a ^ i8080op_readMemory(state, i8080op_getHL(state));
		i8080op_setZSPAC(state, state->a);
		break;
	case XRA_A:
		log_trace("[%04X] XRA_A(%02X)", state->pc, XRA_A);
		state->a = state->a ^ state->a;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_B:
		log_trace("[%04X] ORA_B(%02X)", state->pc, ORA_B);
		state->a = state->a | state->b;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_C:
		log_trace("[%04X] ORA_C(%02X)", state->pc, ORA_C);
		state->a = state->a | state->c;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_D:
		log_trace("[%04X] ORA_D(%02X)", state->pc, ORA_D);
		state->a = state->a | state->d;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_E:
		log_trace("[%04X] ORA_E(%02X)", state->pc, ORA_E);
		state->a = state->a | state->e;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_H:
		log_trace("[%04X] ORA_H(%02X)", state->pc, ORA_H);
		state->a = state->a | state->h;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_L:
		log_trace("[%04X] ORA_L(%02X)", state->pc, ORA_L);
		state->a = state->a | state->l;
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_M:
		log_trace("[%04X] ORA_M(%02X)", state->pc, ORA_M);
		state->a = state->a | i8080op_readMemory(state, i8080op_getHL(state));
		i8080op_setZSPAC(state, state->a);
		break;
	case ORA_A:
		log_trace("[%04X] ORA_A(%02X)", state->pc, ORA_A);
		state->a = state->a | state->a;
		i8080op_setZSPAC(state, state->a);
		break;
	case CMP_B: // takes B from A
		log_trace("[%04X] CMP_B(%02X)", state->pc, CMP_B);
		store8_1 = i8080op_subCarry8(state, state->a, state->b);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_C: // takes C from A
		log_trace("[%04X] CMP_C(%02X)", state->pc, CMP_C);
		store8_1 = i8080op_subCarry8(state, state->a, state->c);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_D: // takes D from A
		log_trace("[%04X] CMP_D(%02X)", state->pc, CMP_D);
		store8_1 = i8080op_subCarry8(state, state->a, state->d);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_E: // takes E from A
		log_trace("[%04X] CMP_E(%02X)", state->pc, CMP_E);
		store8_1 = i8080op_subCarry8(state, state->a, state->e);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_H: // takes H from A
		log_trace("[%04X] CMP_H(%02X)", state->pc, CMP_H);
		store8_1 = i8080op_subCarry8(state, state->a, state->h);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_L: // takes L from A
		log_trace("[%04X] CMP_L(%02X)", state->pc, CMP_L);
		store8_1 = i8080op_subCarry8(state, state->a, state->l);
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_M: // takes memory[HL] from A
		log_trace("[%04X] CMP_M(%02X)", state->pc, CMP_M);
		store8_1 = i8080op_subCarry8(state, state->a, i8080op_readMemory(state, i8080op_getHL(state)));
		i8080op_setZSPAC(state, store8_1);
		break;
	case CMP_A: // takes A from A
		log_trace("[%04X] CMP_A(%02X)", state->pc, CMP_A);
		store8_1 = i8080op_subCarry8(state, state->a, state->a);
		i8080op_setZSPAC(state, store8_1);
		break;
	case RNZ:
		log_trace("[%04X] RNZ(%02X)", state->pc, RNZ);
		if (!state->f.z) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = !state->f.z;
		break;
	case POP_B:
		log_trace("[%04X] POP_B(%02X)", state->pc, POP_B);
		i8080op_putBC16(state, i8080op_popStack(state));
		break;
	case JNZ:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JNZ(%02X) %04X (%02X %02X) : %i", state->pc, JNZ, store16_1, byte1, byte2, !state->f.z);
		if (!state->f.z) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case JMP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JMP(%02X) %04X (%02X %02X)", state->pc, JMP, store16_1, byte1, byte2);
		i8080op_setPC(state, store16_1); // Set the pc to jmpPos
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case CNZ:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CNZ(%02X) %04X (%02X %02X) : %i", state->pc, CNZ, store16_1, byte1, byte2, !state->f.z);
		if (!state->f.z) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = !state->f.z;
		break;
	case PUSH_B:
		log_trace("[%04X] PUSH_B(%02X) %04X", state->pc, PUSH_B, i8080op_getBC(state));
		i8080op_pushStack(state, i8080op_getBC(state));
		break;
	case ADI: // Adds D8 to A
		log_trace("[%04X] ADI(%02X) %02X", state->pc, ADI, byte1);
		state->a = i8080op_addCarry8(state, state->a, byte1);
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_0: // Call $0x0
		log_trace("[%04X] RST_0(%02X)", state->pc, RST_0);
		i8080op_executeCALL(state, INTERRUPT_0);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RZ:
		log_trace("[%04X] RZ(%02X)", state->pc, RZ);
		if (state->f.z) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = state->f.z;
		break;
	case RET:
		log_trace("[%04X] RET(%02X)", state->pc, RET);
		pcShouldIncrement = false;
		i8080op_executeRET(state);
		break;
	case JZ:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JZ(%02X) %04X (%02X %02X) : %i", state->pc, JZ, store16_1, byte1, byte2, !state->f.z);
		if (state->f.z) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case CZ:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CNZ(%02X) %04X (%02X %02X) : %i", state->pc, CNZ, store16_1, byte1, byte2, state->f.z);
		if (state->f.z) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = state->f.z;
		break;
	case CALL:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CALL(%02X) %04X (%02X %02X)", state->pc, CALL, store16_1, byte1, byte2);
		i8080op_executeCALL(state, store16_1);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case ACI: // Adds D8 to A
		log_trace("[%04X] ACI(%02X) %02X", state->pc, ACI, byte1);
		state->a = i8080op_addCarry8(state, i8080op_addCarry8(state, state->a, byte1), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_1:
		log_trace("[%04X] RST_1(%02X)", state->pc, RST_1);
		i8080op_executeCALL(state, INTERRUPT_1);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RNC:
		log_trace("[%04X] RNC(%02X)", state->pc, RNC);
		if (!state->f.c) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = !state->f.c;
		break;
	case POP_D:
		log_trace("[%04X] POP_D(%02X)", state->pc, POP_D);
		i8080op_putDE16(state, i8080op_popStack(state));
		break;
	case JNC:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JNC(%02X) %04X (%02X %02X) : %i", state->pc, JNC, store16_1, byte1, byte2, !state->f.c);
		if (!state->f.c) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case OUT:
		log_trace("[%04X] OUT(%02X): %c", state->pc, OUT, byte1);
		port_out(state, byte1, state->a);
		break;
	case CNC:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CNC(%02X) %04X (%02X %02X) : %i", state->pc, CNC, store16_1, byte1, byte2, !state->f.z);
		if (!state->f.c) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = !state->f.c;
		break;
	case PUSH_D:
		log_trace("[%04X] PUSH_D(%02X) %04X", state->pc, PUSH_D, i8080op_getDE(state));
		i8080op_pushStack(state, i8080op_getDE(state));
		break;
	case SUI: // takes D8 from A
		log_trace("[%04X] SUI(%02X) %02X", state->pc, SUI, byte1);
		state->a = i8080op_subCarry8(state, state->a, byte1);
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_2:
		log_trace("[%04X] RST_2(%02X)", state->pc, RST_2);
		i8080op_executeCALL(state, INTERRUPT_2);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RC:
		log_trace("[%04X] RC(%02X)", state->pc, RC);
		if (state->f.c) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = state->f.c;
		break;
	case JC:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JC(%02X) %04X (%02X %02X) : %i", state->pc, JC, store16_1, byte1, byte2, state->f.c);
		if (state->f.c) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case IN: // TODO
		state->a = port_in(state, byte1);
		break;
	case CC:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CC(%02X) %04X (%02X %02X) : %i", state->pc, CC, store16_1, byte1, byte2, state->f.c);
		if (state->f.c) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = state->f.c;
		break;
	case SBI: // takes D8 from A
		log_trace("[%04X] SUI(%02X) %02X", state->pc, SUI, byte1);
		state->a = i8080op_subCarry8(state, i8080op_subCarry8(state, state->a, byte1), state->f.c);
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_3:
		log_trace("[%04X] RST_3(%02X)", state->pc, RST_3);
		i8080op_executeCALL(state, INTERRUPT_3);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RPO:
		log_trace("[%04X] RPO(%02X)", state->pc, RPO);
		if (!state->f.p) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = !state->f.p;
		break;
	case POP_H:
		log_trace("[%04X] POP_H(%02X)", state->pc, POP_H);
		i8080op_i8080op_putHL16(state, i8080op_popStack(state));
		break;
	case JPO:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JPO(%02X) %04X (%02X %02X) : %i", state->pc, JPO, store16_1, byte1, byte2, !state->f.p);
		if (!state->f.p) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case XTHL:
		log_trace("[%04X] XTHL(%02X)", state->pc, XTHL);
		store16_1 = i8080op_popStack(state);
		i8080op_pushStack(state, i8080op_getHL(state));
		i8080op_i8080op_putHL16(state, store16_1);
		break;
	case CPO:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CPO(%02X) %04X (%02X %02X) : %i", state->pc, CPO, store16_1, byte1, byte2, !state->f.p);
		if (!state->f.p) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = state->f.p;
		break;
	case PUSH_H:
		log_trace("[%04X] PUSH_H(%02X) %04X", state->pc, PUSH_H, i8080op_getHL(state));
		i8080op_pushStack(state, i8080op_getHL(state));
		break;
	case ANI:
		log_trace("[%04X] ANI(%02X) %02X", state->pc, ANI, byte1);
		state->a = state->a & byte1;
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_4:
		log_trace("[%04X] RST_4(%02X)", state->pc, RST_4);
		i8080op_executeCALL(state, INTERRUPT_4);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RPE:
		log_trace("[%04X] RPE(%02X)", state->pc, RPE);
		if (state->f.p) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = state->f.p;
		break;
	case PCHL:
		log_trace("[%04X] PCHL(%02X)", state->pc, PCHL);
		i8080op_setPC(state, i8080op_getHL(state));
		pcShouldIncrement = false;
		break;
	case JPE:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JPE(%02X) %04X (%02X %02X) : %i", state->pc, JPE, store16_1, byte1, byte2, state->f.p);
		if (state->f.p) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case XCHG:
		log_trace("[%04X] XCHG(%02X)", state->pc, XCHG);
		store16_1 = i8080op_getDE(state);
		i8080op_putDE16(state, i8080op_getHL(state));
		i8080op_i8080op_putHL16(state, store16_1);
		break;
	case CPE:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CPE(%02X) %04X (%02X %02X) : %i", state->pc, CPE, store16_1, byte1, byte2, state->f.p);
		if (state->f.p) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = state->f.p;
		break;
	case XRI:
		log_trace("[%04X] XRI(%02X) %02X", state->pc, XRI, byte1);
		state->a = state->a ^ byte1;
		i8080op_setZSPAC(state, state->a);
		break;
	case RST_5:
		log_trace("[%04X] RST_5(%02X)", state->pc, RST_5);
		i8080op_executeCALL(state, INTERRUPT_5);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RP:
		log_trace("[%04X] RP(%02X)", state->pc, RP);
		if (!state->f.s) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = !state->f.s;
		break;
	case POP_PSW:
		log_trace("[%04X] POP_PSW(%02X)", state->pc, POP_PSW);
		store16_1 = i8080op_popStack(state);
		state->a = (store16_1 & 0xFF00) >> 8;
		i8080op_putFlags(state, store16_1 & 0xFF);
		break;
	case JP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JP(%02X) %04X (%02X %02X) : %i", state->pc, JP, store16_1, byte1, byte2, !state->f.s);
		if (!state->f.s) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case DI:
		log_info("[%04X] DI(%02X)", state->pc, DI);
		state->f.ien = 0;
		break;
	case CP:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CP(%02X) %04X (%02X %02X) : %i", state->pc, CP, store16_1, byte1, byte2, !state->f.s);
		if (!state->f.s) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = !state->f.s;
		break;
	case PUSH_PSW:
		log_trace("[%04X] PUSH_PSW(%02X) %04X", state->pc, PUSH_PSW, i8080op_getPSW(state));
		i8080op_pushStack(state, i8080op_getPSW(state));
		break;
	case ORI:
		log_trace("[%04X] ORI(%02X) %02X", state->pc, ORI, byte1);
		state->a = state->a | byte1;
		i8080op_setZSPAC(state, state->a);
	case RST_6:
		log_trace("[%04X] RST_6(%02X)", state->pc, RST_6);
		i8080op_executeCALL(state, INTERRUPT_6);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	case RM:
		log_trace("[%04X] RM(%02X)", state->pc, RM);
		if (state->f.s) {
			pcShouldIncrement = false;
			i8080op_executeRET(state);
		}
		success = state->f.s;
		break;
	case SPHL:
		log_trace("[%04X] SPHL(%02X)", state->pc, SPHL);
		i8080op_setSP(state, i8080op_getHL(state));
		break;
	case JM:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // jmpPos
		log_trace("[%04X] JM(%02X) %04X (%02X %02X) : %i", state->pc, JM, store16_1, byte1, byte2, state->f.s);
		if (state->f.s) {
			i8080op_setPC(state, store16_1); // Set the pc to jmpPos
			pcShouldIncrement = false; // Stop the auto increment
		}
		break;
	case EI:
		log_info("[%04X] EI(%02X)", state->pc, EI);
		state->f.ien = 1;
		break;
	case CM:
		store16_1 = ((uint16_t)byte2 << 8) + byte1; // address
		log_trace("[%04X] CM(%02X) %04X (%02X %02X) : %i", state->pc, CM, store16_1, byte1, byte2, state->f.s);
		if (state->f.s) {
			i8080op_executeCALL(state, store16_1);
			pcShouldIncrement = false; // Stop the auto increment
		}
		success = state->f.s;
		break;
	case CPI:
		log_trace("[%04X] CMP_A(%02X) %02X", state->pc, CMP_A, byte1);
		store8_1 = i8080op_subCarry8(state, state->a, byte1);
		i8080op_setZSPAC(state, store8_1);
		break;
	case RST_7:
		log_trace("[%04X] RST_7(%02X)", state->pc, RST_7);
		i8080op_executeCALL(state, INTERRUPT_7);
		pcShouldIncrement = false; // Stop the auto increment
		break;
	default:
		unimplementedOpcode(state, opcode);
		break;
	}

	// Determine how we increment our pc
	if (pcShouldIncrement) {
		state->pc += byteLen;
	}

	return success;
}

void unimplementedOpcode(i8080State* state, uint8_t opcode) {
	log_warn("Unimplemented opcode %02X, i8080 PANIC!", opcode);
	i8080_panic(state);
}

uint16_t i8080op_getPSW(i8080State* state) {
	uint16_t res = 0;
	res += state->a << 8;
	res += state->f.s << 7;
	res += state->f.z << 6;
	res += state->f.zero << 5;
	res += state->f.ac << 4;
	res += state->f.zero << 3;
	res += state->f.p << 2;
	res += state->f.one << 1;
	return res;
}

uint16_t i8080op_getBC(i8080State* state) {
	return ((uint16_t)state->b << 8) + state->c;
}

uint16_t i8080op_getDE(i8080State* state) {
	return ((uint16_t)state->d << 8) + state->e;
}

uint16_t i8080op_getHL(i8080State* state) {
	return ((uint16_t)state->h << 8) + state->l;
}

void i8080op_putBC8(i8080State* state, uint8_t ubyte, uint8_t lbyte) {
	state->b = ubyte;
	state->c = lbyte;
}
void i8080op_putBC16(i8080State* state, uint16_t b) {
	state->b = b >> 8;
	state->c = b & 0xFF;
}

void i8080op_putDE8(i8080State* state, uint8_t ubyte, uint8_t lbyte) {
	state->d = ubyte;
	state->e = lbyte;
}
void i8080op_putDE16(i8080State* state, uint16_t b) {
	state->d = b >> 8;
	state->e = b & 0xFF;
}

void i8080op_putHL8(i8080State* state, uint8_t ubyte, uint8_t lbyte) {
	state->h = ubyte;
	state->l = lbyte;
}
void i8080op_i8080op_putHL16(i8080State* state, uint16_t b) {
	state->h = b >> 8;
	state->l = b & 0xFF;
}

void i8080op_setZSPAC(i8080State* state, uint8_t v) {
	state->f.z = i8080_isZero(v);
	state->f.s = i8080_isNegative(v);
	state->f.p = i8080_isParityEven(v);
	state->f.ac = i8080_shouldACFlag(v);
}

void i8080op_putFlags(i8080State* state, uint8_t fv) {
	state->f.s = (fv & 0x80) >> 7;
	state->f.z = (fv & 0x40) >> 6;
	state->f.ac = (fv & 0x10) >> 4;
	state->f.p = (fv & 0x04) >> 2;
	state->f.c = (fv & 0x01);
}

uint8_t i8080op_rotateBitwiseLeft(i8080State* state, uint8_t v) {
	uint8_t store8_1 = (v >> 7); // Get the 7th bit
	state->f.c = store8_1; // Store it in the carry flag
	return (v << 1) | store8_1; // Store the 0th bit in position
}

uint8_t i8080op_rotateBitwiseRight(i8080State* state, uint8_t v) {
	uint8_t store8_1 = v & 0x01; // Get the 0th bit
	state->f.c = store8_1; // Store it in the carry flag
	return (v >> 1) | (store8_1 << 7); // Store the 7th bit in position
}

uint16_t i8080op_addCarry16(i8080State* state, uint16_t a, uint16_t b) {
	uint32_t store32_1 = (uint32_t)a + (uint32_t)b;
	state->f.c = (store32_1 & 0xFFFF0000) >> 16;
	//log_debug("carry:%i, val:%08X", state->f.c, store32_1);
	return store32_1 & 0xFFFF;
}

uint16_t i8080op_subCarry16(i8080State* state, uint16_t a, uint16_t b) {
	state->f.c = a < b;
	uint16_t store16_1 = a + ~b + state->f.c;
	log_debug("sub carry:%i, val:%08X", state->f.c, store16_1);
	return store16_1;
}

uint8_t i8080op_addCarry8(i8080State* state, uint8_t a, uint8_t b) {
	uint16_t store16_1 = (uint16_t)a + (uint16_t)b;
	state->f.c = (store16_1 & 0xFF00) >> 8;
	//log_debug("carry:%i, val:%08X", state->f.c, store32_1);
	return store16_1 & 0xFF;
}

uint8_t i8080op_subCarry8(i8080State* state, uint8_t a, uint8_t b) {
	state->f.c = a < b;
	uint8_t store8_1 = a + ~b + state->f.c;
	log_debug("sub carry:%i, val:%08X", state->f.c, store8_1);
	return store8_1;
}
