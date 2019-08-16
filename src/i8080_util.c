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
	case STAX_B: return "STAX B"; break;
	case INX_B: return "INX B"; break;
	case INR_B: return "INR B"; break;
	case DCR_B: return "DCR B"; break;
	case MVI_B: return "MVI B"; break;
	case RLC: return "RLC"; break;
	case DAD_B: return "DAD B"; break;
	case LDAX_B: return "LDAX B"; break;
	case DCX_B: return "DCX B"; break;
	case DCR_C: return "DCR C"; break;
	case MVI_C: return "MVI C"; break;
	case RRC: return "RRC"; break;
	case LXI_D: return "LXI D"; break;
	case STAX_D: return "STAX D"; break;
	case INX_D: return "INX D"; break;
	case INR_D: return "INR D"; break;
	case DCR_D: return "DCR D"; break;
	case MVI_D: return "MVI D"; break;
	case RAL: return "RAL"; break;
	case DAD_D: return "DAD D"; break;
	case LDAX_D: return "LDAX D"; break;
	case DCX_D: return "DCX D"; break;
	case INR_E: return "INR E"; break;
	case DCR_E: return "DCR E"; break;
	case MVI_E: return "MVI E"; break;
	case RAR: return "RAR"; break;
	case LXI_H: return "LXI H"; break;
	case SHLD: return "SHLD"; break;
	case INX_H: return "INX H"; break;
	case INR_H: return "INR H"; break;
	case DCR_H: return "DCR H"; break;
	case MVI_H: return "MVI H"; break;
	case DAA: return "DAA"; break;
	case DAD_H: return "DAD H"; break;
	case LHLD: return "LHLD"; break;
	case DCX_H: return "DCX H"; break;
	case DCR_L: return "DCR L"; break;
	case INR_L: return "INR L"; break;
	case MVI_L: return "MVI L"; break;
	case CMA: return "CMA"; break;
	case LXI_SP: return "LXI SP"; break;
	case STA: return "STA"; break;
	case INX_SP: return "INX SP"; break;
	case INR_M: return "INR M"; break;
	case DCR_M: return "DCR M"; break;
	case MVI_M: return "MVI M"; break;
	case STC: return "STC"; break;
	case DAD_SP: return "DAD SP"; break;
	case LDA: return "LDA"; break;
	case DCX_SP: return "DCX SP"; break;
	case INR_A: return "INR A"; break;
	case MVI_A: return "MVI A"; break;
	case CMC: return "CMC"; break;
	case MOV_BB: return "MOV B,B"; break;
	case MOV_BC: return "MOV B,C"; break;
	case MOV_BD: return "MOV B,D"; break;
	case MOV_BE: return "MOV B,E"; break;
	case MOV_BH: return "MOV B,H"; break;
	case MOV_BL: return "MOV B,L"; break;
	case MOV_BM: return "MOV B,M"; break;
	case MOV_BA: return "MOV B,A"; break;
	case MOV_CB: return "MOV C,B"; break;
	case MOV_CC: return "MOV C,C"; break;
	case MOV_CD: return "MOV C,D"; break;
	case MOV_CE: return "MOV C,E"; break;
	case MOV_CH: return "MOV C,H"; break;
	case MOV_CL: return "MOV C,L"; break;
	case MOV_CM: return "MOV C,M"; break;
	case MOV_CA: return "MOV C,A"; break;
	case MOV_DB: return "MOV D,B"; break;
	case MOV_DC: return "MOV D,C"; break;
	case MOV_DD: return "MOV D,D"; break;
	case MOV_DE: return "MOV D,E"; break;
	case MOV_DH: return "MOV D,H"; break;
	case MOV_DL: return "MOV D,L"; break;
	case MOV_DM: return "MOV D,M"; break;
	case MOV_DA: return "MOV D,A"; break;
	case MOV_EB: return "MOV E,B"; break;
	case MOV_EC: return "MOV E,C"; break;
	case MOV_ED: return "MOV E,D"; break;
	case MOV_EE: return "MOV E,E"; break;
	case MOV_EH: return "MOV E,H"; break;
	case MOV_EL: return "MOV E,L"; break;
	case MOV_EM: return "MOV E,M"; break;
	case MOV_EA: return "MOV E,A"; break;
	case MOV_HB: return "MOV H,B"; break;
	case MOV_HC: return "MOV H,C"; break;
	case MOV_HD: return "MOV H,D"; break;
	case MOV_HE: return "MOV H,E"; break;
	case MOV_HH: return "MOV H,H"; break;
	case MOV_HL: return "MOV H,L"; break;
	case MOV_HM: return "MOV H,M"; break;
	case MOV_HA: return "MOV H,A"; break;
	case MOV_LB: return "MOV L,B"; break;
	case MOV_LC: return "MOV L,C"; break;
	case MOV_LD: return "MOV L,D"; break;
	case MOV_LE: return "MOV L,E"; break;
	case MOV_LH: return "MOV L,H"; break;
	case MOV_LL: return "MOV L,L"; break;
	case MOV_LM: return "MOV L,M"; break;
	case MOV_LA: return "MOV L,A"; break;
	case MOV_MB: return "MOV M,B"; break;
	case MOV_MC: return "MOV M,C"; break;
	case MOV_MD: return "MOV M,D"; break;
	case MOV_ME: return "MOV M,E"; break;
	case MOV_MH: return "MOV M,H"; break;
	case MOV_ML: return "MOV M,L"; break;
	case HLT: return "HLT"; break;
	case MOV_MA: return "MOV M,A"; break;
	case MOV_AB: return "MOV A,B"; break;
	case MOV_AC: return "MOV A,C"; break;
	case MOV_AD: return "MOV A,D"; break;
	case MOV_AE: return "MOV A,E"; break;
	case MOV_AH: return "MOV A,H"; break;
	case MOV_AL: return "MOV A,L"; break;
	case MOV_AM: return "MOV A,M"; break;
	case MOV_AA: return "MOV A,A"; break;
	case ADD_B: return "ADD A,B"; break;
	case ADD_C: return "ADD A,C"; break;
	case ADD_D: return "ADD A,D"; break;
	case ADD_E: return "ADD A,E"; break;
	case ADD_H: return "ADD A,H"; break;
	case ADD_L: return "ADD A,L"; break;
	case ADD_M: return "ADD A,M"; break;
	case ADD_A: return "ADD A,A"; break;
	case ADC_B: return "ADC A,B"; break;
	case ADC_C: return "ADC A,C"; break;
	case ADC_D: return "ADC A,D"; break;
	case ADC_E: return "ADC A,E"; break;
	case ADC_H: return "ADC A,H"; break;
	case ADC_L: return "ADC A,L"; break;
	case ADC_M: return "ADC A,M"; break;
	case ADC_A: return "ADC A,A"; break;
	case SUB_B: return "SUB A,B"; break;
	case SUB_C: return "SUB A,C"; break;
	case SUB_D: return "SUB A,D"; break;
	case SUB_E: return "SUB A,E"; break;
	case SUB_H: return "SUB A,H"; break;
	case SUB_L: return "SUB A,L"; break;
	case SUB_M: return "SUB A,M"; break;
	case SUB_A: return "SUB A,A"; break;
	case SBB_B: return "SBB A,B"; break;
	case SBB_C: return "SBB A,C"; break;
	case SBB_D: return "SBB A,D"; break;
	case SBB_E: return "SBB A,E"; break;
	case SBB_H: return "SBB A,H"; break;
	case SBB_L: return "SBB A,L"; break;
	case SBB_M: return "SBB A,M"; break;
	case SBB_A: return "SBB A,A"; break;
	case ANA_B: return "ANA A,B"; break;
	case ANA_C: return "ANA A,C"; break;
	case ANA_D: return "ANA A,D"; break;
	case ANA_E: return "ANA A,E"; break;
	case ANA_H: return "ANA A,H"; break;
	case ANA_L: return "ANA A,L"; break;
	case ANA_M: return "ANA A,M"; break;
	case ANA_A: return "ANA A,A"; break;
	case XRA_B: return "XRA A,B"; break;
	case XRA_C: return "XRA A,C"; break;
	case XRA_D: return "XRA A,D"; break;
	case XRA_E: return "XRA A,E"; break;
	case XRA_H: return "XRA A,H"; break;
	case XRA_L: return "XRA A,L"; break;
	case XRA_M: return "XRA A,M"; break;
	case XRA_A: return "XRA A,A"; break;
	case ORA_B: return "ORA A,B"; break;
	case ORA_C: return "ORA A,C"; break;
	case ORA_D: return "ORA A,D"; break;
	case ORA_E: return "ORA A,E"; break;
	case ORA_H: return "ORA A,H"; break;
	case ORA_L: return "ORA A,L"; break;
	case ORA_M: return "ORA A,M"; break;
	case ORA_A: return "ORA A,A"; break;
	case CMP_B: return "CMP A,B"; break;
	case CMP_C: return "CMP A,C"; break;
	case CMP_D: return "CMP A,D"; break;
	case CMP_E: return "CMP A,E"; break;
	case CMP_H: return "CMP A,H"; break;
	case CMP_L: return "CMP A,L"; break;
	case CMP_M: return "CMP A,M"; break;
	case CMP_A: return "CMP A,A"; break;
	case RNZ: return "RNZ"; break;
	case POP_B: return "POP B"; break;
	case JNZ: return "JNZ"; break;
	case JMP: return "JMP"; break;
	case CNZ: return "CNZ"; break;
	case PUSH_B: return "PUSH B"; break;
	case ADI: return "ADI"; break;
	case RST_0: return "RST 0"; break;
	case RZ: return "RZ"; break;
	case RET: return "RET"; break;
	case JZ: return "JZ"; break;
	case CZ: return "CZ"; break;
	case CALL: return "CALL"; break;
	case ACI: return "ACI"; break;
	case RST_1: return "RST 1"; break;
	case RNC: return "RNC"; break;
	case POP_D: return "POP D"; break;
	case JNC: return "JNC"; break;
	case OUT: return "OUT"; break;
	case CNC: return "CNC"; break;
	case PUSH_D: return "PUSH D"; break;
	case SUI: return "SUI"; break;
	case RST_2: return "RST 2"; break;
	case RC: return "RC"; break;
	case JC: return "JC"; break;
	case IN: return "IN"; break;
	case CC: return "CC"; break;
	case SBI: return "SBI"; break;
	case RST_3: return "RST 3"; break;
	case RPO: return "RPO"; break;
	case POP_H: return "POP H"; break;
	case JPO: return "JPO"; break;
	case XTHL: return "XTHL"; break;
	case CPO: return "CPO"; break;
	case PUSH_H: return "PUSH H"; break;
	case ANI: return "ANI"; break;
	case RST_4: return "RST 4"; break;
	case RPE: return "RPE"; break;
	case PCHL: return "PCHL"; break;
	case JPE: return "JPE"; break;
	case XCHG: return "XCHG"; break;
	case CPE: return "CPE"; break;
	case XRI: return "XRI"; break;
	case RST_5: return "RST 5"; break;
	case RP: return "RP"; break;
	case POP_PSW: return "POP PSW"; break;
	case JP: return "JP"; break;
	case DI: return "DI"; break;
	case CP: return "CP"; break;
	case PUSH_PSW: return "PUSH PSW"; break;
	case ORI: return "ORI"; break;
	case RST_6: return "RST 6"; break;
	case RM: return "RM"; break;
	case EI: return "EI"; break;
	case CM: return "CM"; break;
	case CPI: return "CPI"; break;
	case RST_7: return "RST 7"; break;
	case JM: return "JM"; break;
	}
	return "unknown";
}

const char* getModeStr(int mode) {
	switch (mode) {
	case MODE_HLT:
		return "HLT"; break;
	case MODE_NORMAL:
		return "NORMAL"; break;
	case MODE_PAUSED:
		return "PAUSED"; break;
	case MODE_PANIC:
		return "PANIC"; break;
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

void breakpoint(i8080State* state, const char* reason) {
	if (state->mode != MODE_NORMAL)
		return;
	state->mode = MODE_PAUSED;
	log_warn("BREAKPOINT TRIGGERED: %s", reason);
}

void i8080_stateCheck(i8080State* state) {
	if (state == NULL) {
		log_fatal("NULL state");
		exit(-1);
	}
}

void init8080(i8080State* state) {
	state->mode = MODE_HLT; // set valid

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
	state->f.ien = 0; // Interrupts are disabled by default
	state->f.isi = 0;

	// Set the video memory flags
	state->vid.startAddress = 0;
	state->vid.height = 64;
	state->vid.width = 64;

	state->cyclesExecuted = 0;

	state->statusString = "";

	// Reset the previousInstructions
	for (int i = 0; i < INSTRUCTION_TRACE_LEN; i++) {
		state->previousInstructions[i].cycleNum = 0;
		state->previousInstructions[i].opcode = 0;
		state->previousInstructions[i].b1 = 0;
		state->previousInstructions[i].b2 = 0;
		state->previousInstructions[i].pc = 0;
		state->previousInstructions[i].psw = 0;
		state->previousInstructions[i].statusString = "NOP";
		state->previousInstructions[i].topStack = 0;
	}

	// Reset the ports
	for (int i = 0; i < NUMBER_OF_PORTS; i++) {
		state->inPorts[i] = 0;
		// Buffered port
		state->outPorts[i].val = 0;
		state->outPorts[i].portFilled = false;
		for (int y = 0; y < BUFFERED_OUT_PORT_LEN; y++) {
			state->outPorts[i].buffer[y] = 0;
			if(y == BUFFERED_OUT_PORT_LEN - 1)
				state->outPorts[i].buffer[y] = '\0';
		}
	}
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

void i8080_acFlagSetAdd(i8080State* state, uint8_t n) {
	// Other possible implementation: ((c->a | val) & 0x08) != 0;
	state->f.ac = (n & 0xF) == 0;
}

void i8080_acFlagSetSub(i8080State* state, uint8_t n) {
	// Other possible implementation: ((c->a | val) & 0x08) != 0;
	i8080_acFlagSetAdd(state, n);
	state->f.ac = !state->f.ac;
}

void i8080_acFlagSetInc(i8080State* state, uint8_t n) {
	state->f.ac = (n & 0xF) == 0;
}

void i8080_acFlagSetDcr(i8080State* state, uint8_t n) {
	state->f.ac = !((n & 0xF) == 0xF);
}

void i8080_acFlagSetAna(i8080State* state, uint8_t n) {
	state->f.ac = ((state->a | n) & 0x08) != 0;
}

void i8080_acFlagSetCmp(i8080State* state, uint8_t n) {
	uint16_t result = state->a - n;
	state->f.ac = ~(state->a ^ result ^ n) & 0x10;
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
