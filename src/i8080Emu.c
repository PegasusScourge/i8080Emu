/*

i8080Emu.c

Main file

*/

#include "i8080_test.h"
#include "i8080.h"

#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SFML/Graphics.h>
#include <SFML/System.h>

/* Function defs */
// Initialise the graphics
void initGraphics(unsigned int width, unsigned int height);
// Close the graphics
void closeGraphics();
// Handle the events
void handleEvent(const sfEvent* evt, i8080State* state);
// Polls the input
void pollInput(i8080State* state);
// Render the state info for the window
void renderStateInfo(i8080State* state, float frameTimeMillis);
// Update the video buffer
void updateVideoBuffer(i8080State* state, sfImage* img);
// Process the switches in the program args
void processSwitches(i8080State* state, int argc, char** argv);
// Processes the external shift register
void processExternShiftRegister(i8080State* state);

// var defs
sfRenderWindow* window = NULL; // window handle
sfEvent cEvent; // Event container
sfFont* font = NULL;
sfSprite* videoSprite = NULL;
sfImage* videoImg = NULL;

uint8_t shift0 = 0;
uint8_t shift1 = 0;
uint8_t shiftOffset = 0;

bool shouldClose = false;
bool showStats = false;

#define TEXT_SIZE 14

int main(int argc, char** argv) {
	// Open the log file
	FILE* logFile = fopen("i8080Emu.log", "w");
	if (logFile == NULL) {
		// Error
		printf("error: Failed to open 'i8080Emu.log' for writing\n");
		return -1;
	}
	// Pass the log file
	log_set_fp(logFile);

	// Set log level
	log_set_level(LOG_INFO);

	// Init the 8080
	i8080State* state = malloc(sizeof(i8080State));
	if (state == NULL) {
		log_fatal("Failed to allocate space for i8080 state");
	}

	init8080(state);
	
	processSwitches(state, argc, argv);

	// Init the graphics
	log_info("--- Init graphics ---");
	log_info("videoMemory: %04X, dimensions (%i, %i)", state->vid.startAddress, state->vid.width, state->vid.height);
	initGraphics(state->vid.width, state->vid.height);
	log_info("-- Graphics init complete ---");

	// Create the timer
	sfClock* timer = sfClock_create();
	sfTime time;

	// Timing variables
	float elapsedTime = 0;
	float cycle_accumulator = 0;

	log_info("Initial pc: %04X", state->pc);
	state->mode = MODE_PAUSED;

	// Set the inPort values needed
	state->inPorts[1] = 0x00;
	state->inPorts[2] = 0x80;

	// Do emulation
	while (!shouldClose) {
		// check events
		while (sfRenderWindow_pollEvent(window, &cEvent)) {
			handleEvent(&cEvent, state);
		}
		pollInput(state);

		// Calculate the time passed since the last loop
		time = sfClock_getElapsedTime(timer);
		sfClock_restart(timer);
		elapsedTime = (float)sfTime_asMicroseconds(time) / 1000.0f;
		cycle_accumulator += elapsedTime;

		unsigned long cyclesPerFrame = state->clockFreqMHz * MHZ * (elapsedTime / 1000.0f);
		
		// Loop only if we are in normal mode
		while (cyclesPerFrame > 0 && state->mode == MODE_NORMAL) {
			i8080_cpuTick(state);
			cyclesPerFrame--;
		}

		// Process the shift register specific to space invaders
		processExternShiftRegister(state);
		
		// Update the video buffer
		updateVideoBuffer(state, videoImg);

		if(state->mode != MODE_PANIC)
			sfRenderWindow_clear(window, sfColor_fromRGB(0, 0, 100));
		else
			sfRenderWindow_clear(window, sfColor_fromRGB(100, 10, 10));

		// Render the video buffer
		sfTexture* videoTexture = sfTexture_createFromImage(videoImg, NULL);
		sfSprite_setTexture(videoSprite, videoTexture, false);
		sfRenderWindow_drawSprite(window, videoSprite, NULL);
		sfTexture_destroy(videoTexture);

		renderStateInfo(state, elapsedTime);

		// Display the window
		sfRenderWindow_display(window);
	}

	// Output the opcodes that were used by the program
	FILE* fp = fopen("i8080_opcodeUse.log", "w");
	if (fp == NULL) {
		log_error("Unable to output opcodeUse table: failed to get file handle");
	}
	else {
		int codesUsed = 0;
		fprintf(fp, "Opcode use table\n------------------------------------------\n");
		for (int i = 0; i < 0x100; i++) {
			if (state->opcodeUse[i] == 1) {
				fprintf(fp, "%02X: %s\n", i, i8080_decompile(i));
				codesUsed++;
			}
		}
		fprintf(fp, "------------------------------------------\n");
		fprintf(fp, "Number used: %i / 256\n", codesUsed);
		fclose(fp);
	}

	// Destroy the timer
	sfClock_destroy(timer);

	// Close the SFML window
	closeGraphics();

	// Free the memory
	free(state->memory);
	free(state);

	// Close the log file
	fclose(logFile);

	return 0;
}

void renderStateInfo(i8080State* state, float frameTimeMillis) {
	sfText* renderText = sfText_create();
	if (renderText == NULL) {
		log_warn("Rendering text object failed to create");
		return;
	}
	sfText_setCharacterSize(renderText, TEXT_SIZE);
	sfText_setFont(renderText, font);
	sfColor normalC = sfColor_fromRGB(0, 255, 200);
	sfText_setFillColor(renderText, normalC);
	sfText_setOutlineColor(renderText, sfColor_fromRGB(0, 0, 0));
	sfText_setOutlineThickness(renderText, 2.0f);

	// Do the state rendering
	sfVector2f pos;
	int incY = TEXT_SIZE + 2;
	int xSpace = TEXT_SIZE * 14;

	pos.x = 16;
	pos.y = 525;
	sfText_setString(renderText, "[ESC] exit, [BACKSPACE] HLT, [P] pause, [O] normal, [S] step, [F1] debug dump, [F2] show stats, [SHIFT+F2] hide stats"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
	sfText_setString(renderText, ""); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
	sfText_setString(renderText, "Current mode:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 1.5;
	sfText_setString(renderText, getModeStr(state->mode)); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;

	if(showStats) {
		#define X_INIT_POS 16
		pos.x = X_INIT_POS;
		pos.y = TEXT_SIZE + 4;
		char buf[80];

		sfText_setString(renderText, "Registers:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;

		sfText_setString(renderText, "a:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->a, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "b:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->b, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "c:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->c, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "d:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->d, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "e:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->e, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "h:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->h, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "l:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->l, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "pc:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->pc, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "sp:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->sp, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "psw:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		sfText_setString(renderText, i8080_decToBin(i8080op_getPSW(state))); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
		sfText_setString(renderText, "[accumu]SZ0A0P1C"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		pos.y += incY;
		sfText_setString(renderText, "I_Enable:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->f.ien, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Interrupted:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->f.isi, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		pos.y += incY;
		sfText_setString(renderText, "Wait cycles:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->waitCycles, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Frame Time (ms):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_gcvt(frameTimeMillis, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Clock frequency (MHz):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_gcvt(state->clockFreqMHz, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;
		
		sfText_setString(renderText, "Cycles executed:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_ultoa(state->cyclesExecuted, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		pos.y += incY;
		uint8_t opcode = i8080op_readMemory(state, state->pc);
		sfText_setString(renderText, "Instruction:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(opcode, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Status string:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		sfText_setString(renderText, i8080_decompile(opcode)); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Byte1:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(i8080op_readMemory(state, state->pc + 1), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Byte2:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(i8080op_readMemory(state, state->pc + 2), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Instr len:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(i8080_getInstructionLength(i8080op_readMemory(state, state->pc)), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		pos.y += incY;
		sfText_setString(renderText, "Video Memory Loc:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->vid.startAddress, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		sfText_setString(renderText, "Video Memory Dims:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->vid.width, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 4;
		_itoa(state->vid.height, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		pos.y += incY;
		sfText_setString(renderText, "Extern shift reg:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
		_itoa(state->inPorts[3], buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

		#define X_POS_MEM_COL 400
		pos.x = X_POS_MEM_COL;
		pos.y = TEXT_SIZE + 4;

		sfText_setString(renderText, "Memory snippet:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
		// Display the memory content surrounding our current pc
		for (int i = state->pc - 10; i < state->pc + 10; i++) {
			if (i8080_boundsCheckMemIndex(state, i)) {
				if (i == state->pc) {
					sfText_setFillColor(renderText, sfColor_fromRGB(255, 0, 0)); // Highlight the current PC in red
				}
				else {
					sfText_setFillColor(renderText, normalC);
				}

				_itoa(i, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 2;
				_itoa(i8080op_readMemory(state, i), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x = X_POS_MEM_COL;
			}
			pos.y += incY;
		}

		pos.x = X_POS_MEM_COL;
		pos.y += incY * 2;

		sfColor activeGreen = sfColor_fromRGB(50, 255, 50);
		sfColor inactiveRed = sfColor_fromRGB(100, 0, 0);
		sfText_setFillColor(renderText, normalC);
		sfText_setString(renderText, "Port status:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
		sfText_setFillColor(renderText, state->f.rx ? activeGreen : inactiveRed);
		sfText_setString(renderText, "RX"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += (TEXT_SIZE * 1.5f) + 4;
		sfText_setFillColor(renderText, state->f.tx ? activeGreen : inactiveRed);
		sfText_setString(renderText, "TX"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
		sfText_setFillColor(renderText, normalC);

		#define X_POS_STACK_COL 550
		pos.x = X_POS_STACK_COL;
		pos.y = TEXT_SIZE + 4;

		sfText_setString(renderText, "Stack:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
		// Display the stack
		for (int i = state->sp - 2; i < state->sp + 20; i += 2) {
			if (i8080_boundsCheckMemIndex(state, i)) {
				if (i == state->sp) {
					sfText_setFillColor(renderText, sfColor_fromRGB(255, 0, 0)); // Highlight the top of the stack in red
				}
				else {
					sfText_setFillColor(renderText, normalC);
				}

				_itoa(i, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 2;
				_itoa(i8080op_readMemory(state, i) + (i8080op_readMemory(state, i + 1) << 8), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x = X_POS_STACK_COL;
			}
			pos.y += incY;
		}
		/*
		sfText_setCharacterSize(renderText, 10);

		#define X_POS_VRAM_COL 700
		pos.x = X_POS_VRAM_COL;
		pos.y = (10) + 4;

		uint32_t tPixels = state->vid.width * state->vid.height;

		sfText_setString(renderText, "VRAM:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
		for (uint32_t i = 1; i <= tPixels; i++) {
			//log_info("vram %i at %f,%f", i, pos.x, pos.y);
			//_itoa(i8080op_readMemory(state, i - 1 + state->vid.startAddress), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
			sfText_setString(renderText, i8080op_readMemory(state, i - 1 + state->vid.startAddress) ? "1": "0"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
			if (i % 32 == 0) {
				pos.x = X_POS_VRAM_COL;
				pos.y += incY / 2;
			}
			else {
				pos.x += (11);
			}
		}
		*/

		#define X_POS_INST_TRC_COL 700
		pos.x = X_POS_INST_TRC_COL;
		pos.y = TEXT_SIZE + 4;

		sfText_setFillColor(renderText, normalC);
		//sfText_setCharacterSize(renderText, 12);

		sfText_setString(renderText, "Instruction trace:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
		pos.y += incY;

		// Print the last instructions
		int maxInstructionTrace = INSTRUCTION_TRACE_LEN > 10 ? 10 : INSTRUCTION_TRACE_LEN;
		for (int i = 0; i < maxInstructionTrace; i++) {
			sprintf(buf, "{-%i}[PC:%04X] %s(%02X) (TS:%04X) B1:%02X B2:%02X", i, state->previousInstructions[i].pc, state->previousInstructions[i].statusString, state->previousInstructions[i].opcode, state->previousInstructions[i].topStack, state->previousInstructions[i].b1, state->previousInstructions[i].b2);
			sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
			pos.y += incY;
		}

		pos.y += incY * 2;
		pos.x = X_POS_INST_TRC_COL;

		sfText_setString(renderText, "Out ports:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
		pos.y += incY;
		for (int i = 0; i < NUMBER_OF_PORTS; i++) {
			_itoa(i, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 4;
			sfText_setString(renderText, state->outPorts[i].buffer); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL);
			pos.y += incY; pos.x = X_POS_INST_TRC_COL;
		}
	}

	sfText_destroy(renderText);
}

void updateVideoBuffer(i8080State* state, sfImage* img) {
	sfColor on = sfColor_fromRGB(255, 255, 255);
	sfColor off = sfColor_fromRGB(0, 0, 0);

	for (int y = 0; y < state->vid.height; y++) {
		for (int xByte = 0; xByte < state->vid.width / 8; xByte++) {
			uint8_t byte = i8080op_readMemory(state, state->vid.startAddress + xByte + (y * (state->vid.width / 8)));
			for (int xBit = 0; xBit < 8; xBit++) {
				unsigned int bit = byte >> (7 - xBit) & 1;
				sfImage_setPixel(img, (xByte * 8) + (8 - xBit), y, bit ? on : off);
			}
		}
	}
}

void processExternShiftRegister(i8080State* state) {
	// Get the value from the shift offset select
	if (state->outPorts[2].portFilled == true) {
		state->outPorts[2].portFilled = false;
		shiftOffset = state->outPorts[2].val & 0x7;
	}
	// Get the shift register input
	if (state->outPorts[4].portFilled == true) {
		state->outPorts[4].portFilled = false;
		shift0 = shift1;
		shift1 = state->outPorts[4].val;
	}
	// Output the value to the in port
	uint16_t v = (shift1 << 8) | shift0;
	state->inPorts[3] = ((v >> (8 - shiftOffset)) & 0xff);
}

void handleEvent(const sfEvent* evt, i8080State* state) {
	switch (evt->type) {
	case sfEvtClosed:
		// Close request
		shouldClose = true;
		break;

	case sfEvtKeyPressed:
		switch (evt->key.code) {
		case sfKeyBackspace:
			state->mode = MODE_HLT;
			break;
		case sfKeyP:
			state->mode = MODE_PAUSED;
			break;
		case sfKeyO:
			state->mode = MODE_NORMAL;
			break;
		case sfKeyS:
			// Step
			state->mode = MODE_PAUSED;
			state->waitCycles = 0;
			i8080_cpuTick(state);
			break;
		case sfKeyF1:
			state->mode = MODE_PAUSED;
			i8080_dump(state);
			break;
		case sfKeyF2:
			if (evt->key.shift) {
				showStats = false;
			}
			else {
				showStats = true;
			}
			break;
		case sfKeyEscape:
			shouldClose = true;
			break;
		}
		break;

	default:
		//printf("Got unknown event from window\n");
		break;
	}
}

void pollInput(i8080State* state) {

}

void initGraphics(unsigned int width, unsigned int height) {
	sfVideoMode videoMode;
	videoMode.width = 1152;
	videoMode.height = 600;

	sfContextSettings contextSettings;
	contextSettings.majorVersion = 3;
	contextSettings.minorVersion = 2;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 0;

	window = sfRenderWindow_create(videoMode, "i8080 Emulator", sfDefaultStyle, &contextSettings);

	sfRenderWindow_setFramerateLimit(window, 200);

	//font = sfFont_createFromFile("arial.ttf");
	font = sfFont_createFromFile("Consolas.ttf");
	if (font == NULL) {
		log_fatal("Failed to load font");
		exit(-1);
	}

	videoImg = sfImage_create(width, height);
	videoSprite = sfSprite_create();
	sfVector2f pos;
	pos.x = (((float)videoMode.width) / 2) - ((float)width);
	pos.y = (((float)videoMode.height) / 2) + ((float)height);
	sfSprite_setPosition(videoSprite, pos);
	pos.x = 2; pos.y = 2;
	sfSprite_setScale(videoSprite, pos);
	sfSprite_setRotation(videoSprite, -90.0f);
}

void closeGraphics() {
	sfRenderWindow_close(window);
	sfRenderWindow_destroy(window);

	sfFont_destroy(font);

	sfImage_destroy(videoImg);
	sfSprite_destroy(videoSprite);
}

void processSwitches(i8080State* state, int argc, char** argv) {
	if (argc > 1) {
		log_debug("Got argument # %i", argc);
		// Check for arguments
		for (int i = 0; i < argc; i++) {
			log_debug("Argument '%s' at index %i", argv[i], i);
			if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
				// Print the help text and exit
				printf("Usage:\ni8080.exe [switch [arg]]\n -h : Displays this message\n -l <filename> <memory index> : loads a rom into memory at memory index\n --help : alias for -h\n --load <filename> <memory index> : alias for -l\n");
				exit(0);
			}
			else if (strcmp("-l", argv[i]) == 0 || strcmp("--load", argv[i]) == 0) {
				// attempt to load a file to a position
				if ((i + 2) < argc) {
					// There are enough arguments to support this switch
					log_info("Loading file '%s' into memory index %s(%04X)", argv[i + 1], argv[i + 2], atoi(argv[i + 2]));
					loadFile(argv[i + 1], state->memory, i8080_MEMORY_SIZE, strtol(argv[i + 2], NULL, 10));
				}
				else {
					log_fatal("Invalid switch '%s': requires two arguments!", argv[i]);
					exit(-1);
				}
			}
			else if (strcmp("-s", argv[i]) == 0 || strcmp("--speed", argv[i]) == 0) {
				if ((i + 1) < argc) {
					float tgtFreq = atof(argv[i + 1]);
					if (tgtFreq <= 0) {
						log_error("Invalid switch %s: argument is 0 or negative", argv[i]);
					}
					else {
						state->clockFreqMHz = tgtFreq;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					exit(-1);
				}
			}
			else if (strcmp("-va", argv[i]) == 0 || strcmp("--video:address", argv[i]) == 0) {
				if ((i + 1) < argc) {
					uint16_t tgtAddress = atoi(argv[i + 1]);
					if (tgtAddress < 0 || tgtAddress > i8080_MEMORY_SIZE) {
						log_fatal("Invalid memory address, require between 0 and %i", i8080_MEMORY_SIZE);
						exit(-1);
					}
					else {
						state->vid.startAddress = tgtAddress;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					exit(-1);
				}
			}
			else if (strcmp("-vd", argv[i]) == 0 || strcmp("--video:dimensions", argv[i]) == 0) {
				if ((i + 2) < argc) {
					uint16_t x = atoi(argv[i + 1]);
					uint16_t y = atoi(argv[i + 2]);
					if (x < 0 || y < 0) {
						log_fatal("Invalid dimensions, cannot be less than 0! (%i, %i)", x, y);
						exit(-1);
					}
					else {
						state->vid.width = x;
						state->vid.height = y;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					exit(-1);
				}
			}
			else if (strcmp("--loglevel", argv[i]) == 0) {
				if ((i + 1) < argc) {
					uint16_t val = atoi(argv[i + 1]);
					if (val >= 0 && val < 6)
						log_set_level(val);
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					exit(-1);
				}
			}
			else if (strcmp("--test", argv[i]) == 0) {
				i8080_testProtocol(state);
				exit(0);
			}
		}
	}
	else {
		// Allow the user to enter into the command console the files they wish to load
		char consoleBuff[200];
		char memIndexBuff[20];
		int gotChars;

		// Only do this loop if we got no arguments
		while (true) {
			printf("Please enter the ROM file you wish to load:\n> ");
			gotChars = getConsoleLine(&consoleBuff, 200);
			printf("Enter the memory index this should be loaded into:\n> ");
			gotChars = getConsoleLine(&memIndexBuff, 20);
			int memIndex = strtol(&memIndexBuff, NULL, 10);
			if (i8080_boundsCheckMemIndex(&state, memIndex)) {
				// We are in range, load the rom
				loadFile(&consoleBuff, state->memory, i8080_MEMORY_SIZE, memIndex);
				printf("Loaded ROM\n");
			}
			else {
				printf("*********************************************************\nFailed to load ROM: specified memory index was outside the range 0 - %i\n", i8080_MEMORY_SIZE);
			}
			printf("Do you wish to load another ROM? (y/n)\n> ");
			gotChars = getConsoleLine(&consoleBuff, 200);
			if (strcmp(consoleBuff, "y") != 0)
				break;
		}
	}
}