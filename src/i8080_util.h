#pragma once
/*

i8080_util.h

Utility functions and types

*/

#include "log.h"

#define INSTRUCTION_TRACE_LEN 200

//Memory size of the i8080
#define i8080_MEMORY_SIZE 65536

// Boolean info
#define true 1
#define false 0
#define bool unsigned char

// Const defs
#define MHZ 1000000.0f
#define MICROSECONDS_IN_SECOND 1000000.0f
#define PARAMS_BYTE_LEN 0
#define PARAMS_CLOCK_LEN 1
#define PARAMS_FCLOCK_LEN 2
#define NUMBER_OF_PORTS 10
#define BUFFERED_OUT_PORT_LEN 43

// Error bit declarations
#define ERRBIT_MEM_OUT_OF_BOUNDS_UNDERFLW		0b10000000
#define ERRBIT_MEM_OUT_OF_BOUNDS_OVERFLW		0b01000000

// Interrupt defs
#define INTERRUPT_0 0x0000
#define INTERRUPT_1 0x0008
#define INTERRUPT_2 0x0010
#define INTERRUPT_3 0x0018
#define INTERRUPT_4 0x0020
#define INTERRUPT_5 0x0028
#define INTERRUPT_6 0x0030
#define INTERRUPT_7 0x0038

// typedefs
#define UINT8_MAX 0xFF
typedef unsigned char uint8_t;
typedef signed char int8_t;

#define UINT16_MAX 0xFFFF
typedef unsigned short uint16_t;
typedef signed short int16_t;

typedef unsigned int uint32_t;
typedef int int32_t;

typedef struct prevInstruction {
	uint8_t opcode;
	uint8_t b1;
	uint8_t b2;
	uint16_t pc;
	uint16_t psw;
	uint16_t topStack;
	unsigned long cycleNum;
	char* statusString;
} prevInstruction;

typedef struct bufferedPort {
	uint8_t val;
	bool portFilled;
	uint8_t buffer[BUFFERED_OUT_PORT_LEN];
} bufferedPort;

typedef struct flagRegister {
	unsigned int s : 1; // sign flag
	unsigned int z : 1; // zero flag
	unsigned int ac : 1; // auxiliary carry flag
	unsigned int p : 1; // parity flag
	unsigned int c : 1; // carry flag
	unsigned int zero : 1; // always zero, bits 3 and 5
	unsigned int one : 1; // always one, bit 1
	unsigned int ien : 1; // Is the interrupt system enabled?
	unsigned int isi; // Are we currently interrupted?
	unsigned int rx : 1; // Are we reading this tick?
	unsigned int tx : 1; // are we transmitting this tick?
} flagRegister;
typedef struct videoMemoryInfo {
	uint16_t startAddress;
	uint16_t width;
	uint16_t height;
} videoMemoryInfo;
typedef struct i8080State {
	// registers
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
	// memory
	uint8_t* memory;
	int memorySize;
	// timing
	float clockFreqMHz;
	int waitCycles;
	//status
	int mode;
	char* statusString;
	// structs
	struct flagRegister f;
	struct videoMemoryInfo vid;
	// ports
	uint8_t inPorts[NUMBER_OF_PORTS];
	bufferedPort outPorts[NUMBER_OF_PORTS];
	// debug
	unsigned long cyclesExecuted;
	unsigned int opcodeUse[0x100];
	prevInstruction previousInstructions[INSTRUCTION_TRACE_LEN];
	
} i8080State;

enum i8080Mode {
	MODE_NORMAL,
	MODE_PAUSED,
	MODE_HLT,
	MODE_PANIC
};

enum i8080Opcode {
	// Instr	Code
	NOP			= 0x00,
	LXI_B		= 0x01,
	STAX_B		= 0x02,
	INX_B		= 0x03,
	INR_B		= 0x04,
	DCR_B		= 0x05,
	MVI_B		= 0x06,
	RLC			= 0x07,
	DAD_B		= 0x09,
	LDAX_B		= 0x0a,
	DCX_B		= 0x0b,
	INR_C		= 0x0c,
	DCR_C		= 0x0d,
	MVI_C		= 0x0e,
	RRC			= 0x0f,
	LXI_D		= 0x11,
	STAX_D		= 0x12,
	INX_D		= 0x13,
	INR_D		= 0x14,
	DCR_D		= 0x15,
	MVI_D		= 0x16,
	RAL			= 0x17,
	DAD_D		= 0x19,
	LDAX_D		= 0x1a,
	DCX_D		= 0x1b,
	INR_E		= 0x1c,
	DCR_E		= 0x1d,
	MVI_E		= 0x1e,
	RAR			= 0x1f,
	LXI_H		= 0x21,
	SHLD		= 0x22,
	INX_H		= 0x23,
	INR_H		= 0x24,
	DCR_H		= 0x25,
	MVI_H		= 0x26,
	DAA			= 0x27,
	DAD_H		= 0x29,
	LHLD		= 0x2a,
	DCX_H		= 0x2b,
	INR_L		= 0x2c,
	DCR_L		= 0x2d,
	MVI_L		= 0x2e,
	CMA			= 0x2f,
	LXI_SP		= 0x31,
	STA			= 0x32,
	INX_SP		= 0x33,
	INR_M		= 0x34,
	DCR_M		= 0x35,
	MVI_M		= 0x36,
	STC			= 0x37,
	DAD_SP		= 0x39,
	LDA			= 0x3a,
	DCX_SP		= 0x3b,
	INR_A		= 0x3c,
	DCR_A		= 0x3d,
	MVI_A		= 0x3e,
	CMC			= 0x3f,
	MOV_BB		= 0x40,
	MOV_BC		= 0x41,
	MOV_BD		= 0x42,
	MOV_BE		= 0x43,
	MOV_BH		= 0x44,
	MOV_BL		= 0x45,
	MOV_BM		= 0x46,
	MOV_BA		= 0x47,
	MOV_CB		= 0x48,
	MOV_CC		= 0x49,
	MOV_CD		= 0x4a,
	MOV_CE		= 0x4b,
	MOV_CH		= 0x4c,
	MOV_CL		= 0x4d,
	MOV_CM		= 0x4e,
	MOV_CA		= 0x4f,
	MOV_DB		= 0x50,
	MOV_DC		= 0x51,
	MOV_DD		= 0x52,
	MOV_DE		= 0x53,
	MOV_DH		= 0x54,
	MOV_DL		= 0x55,
	MOV_DM		= 0x56,
	MOV_DA		= 0x57,
	MOV_EB		= 0x58,
	MOV_EC		= 0x59,
	MOV_ED		= 0x5a,
	MOV_EE		= 0x5b,
	MOV_EH		= 0x5c,
	MOV_EL		= 0x5d,
	MOV_EM		= 0x5e,
	MOV_EA		= 0x5f,
	MOV_HB		= 0x60,
	MOV_HC		= 0x61,
	MOV_HD		= 0x62,
	MOV_HE		= 0x63,
	MOV_HH		= 0x64,
	MOV_HL		= 0x65,
	MOV_HM		= 0x66,
	MOV_HA		= 0x67,
	MOV_LB		= 0x68,
	MOV_LC		= 0x69,
	MOV_LD		= 0x6a,
	MOV_LE		= 0x6b,
	MOV_LH		= 0x6c,
	MOV_LL		= 0x6d,
	MOV_LM		= 0x6e,
	MOV_LA		= 0x6f,
	MOV_MB		= 0x70,
	MOV_MC		= 0x71,
	MOV_MD		= 0x72,
	MOV_ME		= 0x73,
	MOV_MH		= 0x74,
	MOV_ML		= 0x75,
	HLT			= 0x76,
	MOV_MA		= 0x77,
	MOV_AB		= 0x78,
	MOV_AC		= 0x79,
	MOV_AD		= 0x7a,
	MOV_AE		= 0x7b,
	MOV_AH		= 0x7c,
	MOV_AL		= 0x7d,
	MOV_AM		= 0x7e,
	MOV_AA		= 0x7f,
	ADD_B		= 0x80,
	ADD_C		= 0x81,
	ADD_D		= 0x82,
	ADD_E		= 0x83,
	ADD_H		= 0x84,
	ADD_L		= 0x85,
	ADD_M		= 0x86,
	ADD_A		= 0x87,
	ADC_B		= 0x88,
	ADC_C		= 0x89,
	ADC_D		= 0x8a,
	ADC_E		= 0x8b,
	ADC_H		= 0x8c,
	ADC_L		= 0x8d,
	ADC_M		= 0x8e,
	ADC_A		= 0x8f,
	SUB_B		= 0x90,
	SUB_C		= 0x91,
	SUB_D		= 0x92,
	SUB_E		= 0x93,
	SUB_H		= 0x94,
	SUB_L		= 0x95,
	SUB_M		= 0x96,
	SUB_A		= 0x97,
	SBB_B		= 0x98,
	SBB_C		= 0x99,
	SBB_D		= 0x9a,
	SBB_E		= 0x9b,
	SBB_H		= 0x9c,
	SBB_L		= 0x9d,
	SBB_M		= 0x9e,
	SBB_A		= 0x9f,
	ANA_B		= 0xa0,
	ANA_C		= 0xa1,
	ANA_D		= 0xa2,
	ANA_E		= 0xa3,
	ANA_H		= 0xa4,
	ANA_L		= 0xa5,
	ANA_M		= 0xa6,
	ANA_A		= 0xa7,
	XRA_B		= 0xa8,
	XRA_C		= 0xa9,
	XRA_D		= 0xaa,
	XRA_E		= 0xab,
	XRA_H		= 0xac,
	XRA_L		= 0xad,
	XRA_M		= 0xae,
	XRA_A		= 0xaf,
	ORA_B		= 0xb0,
	ORA_C		= 0xb1,
	ORA_D		= 0xb2,
	ORA_E		= 0xb3,
	ORA_H		= 0xb4,
	ORA_L		= 0xb5,
	ORA_M		= 0xb6,
	ORA_A		= 0xb7,
	CMP_B		= 0xb8,
	CMP_C		= 0xb9,
	CMP_D		= 0xba,
	CMP_E		= 0xbb,
	CMP_H		= 0xbc,
	CMP_L		= 0xbd,
	CMP_M		= 0xbe,
	CMP_A		= 0xbf,
	RNZ			= 0xc0,
	POP_B		= 0xc1,
	JNZ			= 0xc2,
	JMP			= 0xc3,
	CNZ			= 0xc4,
	PUSH_B		= 0xc5,
	ADI			= 0xc6,
	RST_0		= 0xc7,
	RZ			= 0xc8,
	RET			= 0xc9,
	JZ			= 0xca,
	CZ			= 0xcc,
	CALL		= 0xcd,
	ACI			= 0xce,
	RST_1		= 0xcf,
	RNC			= 0xd0,
	POP_D		= 0xd1,
	JNC			= 0xd2,
	OUT			= 0xd3,
	CNC			= 0xd4,
	PUSH_D		= 0xd5,
	SUI			= 0xd6,
	RST_2		= 0xd7,
	RC			= 0xd8,
	JC			= 0xda,
	IN			= 0xdb,
	CC			= 0xdc,
	SBI			= 0xde,
	RST_3		= 0xdf,
	RPO			= 0xe0,
	POP_H		= 0xe1,
	JPO			= 0xe2,
	XTHL		= 0xe3,
	CPO			= 0xe4,
	PUSH_H		= 0xe5,
	ANI			= 0xe6,
	RST_4		= 0xe7,
	RPE			= 0xe8,
	PCHL		= 0xe9,
	JPE			= 0xea,
	XCHG		= 0xeb,
	CPE			= 0xec,
	XRI			= 0xee,
	RST_5		= 0xef,
	RP			= 0xf0,
	POP_PSW		= 0xf1,
	JP			= 0xf2,
	DI			= 0xf3,
	CP			= 0xf4,
	PUSH_PSW	= 0xf5,
	ORI			= 0xf6,
	RST_6		= 0xf7,
	RM			= 0xf8,
	SPHL		= 0xf9,
	JM			= 0xfa,
	EI			= 0xfb,
	CM			= 0xfc,
	CPI			= 0xfe,
	RST_7		= 0xff
};

// Instruction set paramaters: { <byte length of instruction> , <cycle length of instruction> , <cycle length of failed instruction> }
extern const uint8_t instructionParams[0x100][3];

// Loads the contents of a file into the buffer at offset given. Assumes buffer already exists
void loadFile(const char* file, unsigned char* buffer, int bufferSize, int offset);

// Resets the state
void reset8080(i8080State* state);

// Checks the state and exits if it is incorrect
void i8080_stateCheck(i8080State* state);

// Init the 8080
void init8080(i8080State* state);

// Get a line from the console
int getConsoleLine(char* buf, int bufLen);

// Returns the MODE in a human-readable format
const char* getModeStr(int mode);

// Checks if a memory index is in range of the memory buffer
bool i8080_boundsCheckMemIndex(i8080State* state, int index);

// Returns the number of bytes the instruction occupies (including the opcode)
uint8_t i8080_getInstructionLength(uint8_t opcode);

// Returns the number of clock cycles an instruction takes
uint8_t i8080_getInstructionClockCycles(uint8_t opcode);

// Returns the number of clock cycles an instruction takes if it fails
uint8_t i8080_getFailedInstructionClockCycles(uint8_t opcode);

// Returns if the parity of a number is even
bool i8080_isParityEven(uint16_t n);

// Returns if the number is zero
bool i8080_isZero(uint16_t n);

// Returns if the number is negative
bool i8080_isNegative(int16_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetInc(i8080State* state, uint8_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetDcr(i8080State* state, uint8_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetAna(i8080State* state, uint8_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetCmp(i8080State* state, uint8_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetAdd(i8080State* state, uint8_t n);

// Returns if the ac flag should be set
void i8080_acFlagSetSub(i8080State* state, uint8_t n);

// Takes a 16bit value and converts to binary
char* i8080_decToBin(uint16_t n);

// Fills the buffer with the mneumonic for the opcode
const char* i8080_decompile(uint8_t opcode);

// Triggers a breakpoint condition
void breakpoint(i8080State* state, const char* reason);