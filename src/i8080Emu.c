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
// Render the state info for the window
void renderStateInfo(i8080State* state, float accum, float accum2, float frameTimeMillis);
// Update the video buffer
void updateVideoBuffer(i8080State* state, sfImage* img);

// var defs
sfRenderWindow* window = NULL; // window handle
sfEvent cEvent; // Event container
sfFont* font = NULL;
sfSprite* videoSprite = NULL;
sfImage* videoImg = NULL;

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
	i8080State state;
	init8080(&state);
	
	if (argc > 1) {
		log_debug("Got argument # %i", argc);
		// Check for arguments
		for(int i = 0; i < argc; i++) {
			log_debug("Argument '%s' at index %i", argv[i], i);
			if (strcmp("-h", argv[i]) == 0 || strcmp("--help", argv[i]) == 0) {
				// Print the help text and exit
				printf("Usage:\ni8080.exe [switch [arg]]\n -h : Displays this message\n -l <filename> <memory index> : loads a rom into memory at memory index\n --help : alias for -h\n --load <filename> <memory index> : alias for -l\n");
				return 0;
			}
			else if (strcmp("-l", argv[i]) == 0 || strcmp("--load", argv[i]) == 0) {
				// attempt to load a file to a position
				if ((i + 2) < argc) {
					// There are enough arguments to support this switch
					log_info("Loading file '%s' into memory index %s", argv[i + 1], argv[i + 2]);
					loadFile(argv[i + 1], state.memory, i8080_MEMORY_SIZE, strtol(argv[i+2], NULL, 10));
				}
				else {
					log_fatal("Invalid switch '%s': requires two arguments!", argv[i]);
					return -1;
				}
			}
			else if (strcmp("-s", argv[i]) == 0 || strcmp("--speed", argv[i]) == 0) {
				if ((i + 1) < argc) {
					float tgtFreq = atof(argv[i + 1]);
					if (tgtFreq <= 0) {
						log_error("Invalid switch %s: argument is 0 or negative", argv[i]);
					}
					else {
						state.clockFreqMHz = tgtFreq;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					return -1;
				}
			}
			else if (strcmp("-va", argv[i]) == 0 || strcmp("--video:address", argv[i]) == 0) {
				if ((i + 1) < argc) {
					uint16_t tgtAddress = atoi(argv[i + 1]);
					if (tgtAddress < 0 || tgtAddress > i8080_MEMORY_SIZE) {
						log_fatal("Invalid memory address, require between 0 and %i", i8080_MEMORY_SIZE);
						return -1;
					}
					else {
						state.vid.startAddress = tgtAddress;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					return -1;
				}
			}
			else if (strcmp("-vd", argv[i]) == 0 || strcmp("--video:dimensions", argv[i]) == 0) {
				if ((i + 2) < argc) {
					uint16_t x = atoi(argv[i + 1]);
					uint16_t y = atoi(argv[i + 2]);
					if (x < 0 || y < 0) {
						log_fatal("Invalid dimensions, cannot be less than 0! (%i, %i)", x, y);
						return -1;
					}
					else {
						state.vid.width = x;
						state.vid.height = y;
					}
				}
				else {
					log_fatal("Invalid switch '%s': requires one argument!", argv[i]);
					return -1;
				}
			}
			else if (strcmp("--test", argv[i]) == 0) {
				utilTest();
				return 0;
			}
		}
	}

	// Allow the user to enter into the command console the files they wish to load
	char consoleBuff[200];
	char memIndexBuff[20];
	int gotChars;

	// Only do this loop if we got no arguments
	while (argc == 1) {
		printf("Please enter the ROM file you wish to load:\n> ");
		gotChars = getConsoleLine(&consoleBuff, 200);
		printf("Enter the memory index this should be loaded into:\n> ");
		gotChars = getConsoleLine(&memIndexBuff, 20);
		int memIndex = strtol(&memIndexBuff, NULL, 10);
		if (boundsCheckMemIndex(&state, memIndex)) {
			// We are in range, load the rom
			loadFile(&consoleBuff, state.memory, i8080_MEMORY_SIZE, memIndex);
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

	// Init the graphics
	log_info("--- Init graphics ---");
	log_info("videoMemory: %04X, dimensions (%i, %i)", state.vid.startAddress, state.vid.width, state.vid.height);
	initGraphics(state.vid.width, state.vid.height);
	log_info("-- Graphics init complete ---");

	// Create the timer
	sfClock* timer = sfClock_create();
	sfTime time;

	// Timing variables
	float cycleTime = 0; // time a single clock pulse takes in milliseconds
	float elapsedTime = 0;
	float cycle_accumulator = 0;
	float frame_accumulator = 0;

	// Do emulation
	while (state.valid) {
		// check events
		while (sfRenderWindow_pollEvent(window, &cEvent)) {
			handleEvent(&cEvent, &state);
		}

		// Calculate the time passed since the last loop
		cycleTime = 1.0f / (float)state.clockFreqMHz;
		time = sfClock_getElapsedTime(timer);
		sfClock_restart(timer);
		elapsedTime = (float)sfTime_asMicroseconds(time) / 1000.0f;
		cycle_accumulator += elapsedTime;
		frame_accumulator += elapsedTime;

		float accumCyclePrev = cycle_accumulator;

		while (true) {
			if (cycle_accumulator >= cycleTime) {
				cpuTick(&state);
				cycle_accumulator -= cycleTime;
			}
			else {
				break;
			}
		}

		float accumFramePrev = frame_accumulator;
		if (frame_accumulator >= 16.7f) {
			// Trigger CPU interrupt
			log_trace("-- VBlank begin interrupt");
			executeInterrupt(&state, INTERRUPT_1);
			frame_accumulator -= 16.7f;
		}

		// Update the video buffer
		updateVideoBuffer(&state, videoImg);

		sfRenderWindow_clear(window, sfColor_fromRGB(0, 0, 100));

		// Render the video buffer
		sfTexture* videoTexture = sfTexture_createFromImage(videoImg, NULL);
		sfSprite_setTexture(videoSprite, videoTexture, false);
		sfRenderWindow_drawSprite(window, videoSprite, NULL);
		sfTexture_destroy(videoTexture);

		renderStateInfo(&state, accumCyclePrev, accumFramePrev, elapsedTime);

		// Display the window
		sfRenderWindow_display(window);
	}

	// Destroy the timer
	sfClock_destroy(timer);

	// Close the SFML window
	closeGraphics();

	// Free the memory
	free(state.memory);

	// Close the log file
	fclose(logFile);

	return 0;
}

void renderStateInfo(i8080State* state, float accum, float accum2, float frameTimeMillis) {
	sfText* renderText = sfText_create();
	if (renderText == NULL) {
		log_warn("Rendering text object failed to create");
		return;
	}
	sfText_setCharacterSize(renderText, TEXT_SIZE);
	sfText_setFont(renderText, font);
	sfText_setFillColor(renderText, sfColor_fromRGB(255, 255, 255));

	// Do the state rendering
	sfVector2f pos;
	#define X_INIT_POS 16
	pos.x = X_INIT_POS;
	pos.y = TEXT_SIZE + 4;
	int incY = TEXT_SIZE + 2;
	int xSpace = TEXT_SIZE * 14;

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
	sfText_setString(renderText, decimal_to_binary(getPSW(state))); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
	sfText_setString(renderText, "[accumu]SZ0A0P1C"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	pos.y += incY;
	sfText_setString(renderText, "Wait cycles:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(state->waitCycles, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Cycle accumulator (ms):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_gcvt(accum, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Frame accumulator (ms):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_gcvt(accum2, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Frame Time (ms):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_gcvt(frameTimeMillis, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Clock frequency (MHz):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_gcvt(state->clockFreqMHz, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Cycle time (ms):"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_gcvt(1.0f / (float)state->clockFreqMHz, 8, buf); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;
	
	pos.y += incY;
	sfText_setString(renderText, "instruction:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(readMemory(state, state->pc), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "byte1:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(readMemory(state, state->pc + 1), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "byte2:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(readMemory(state, state->pc), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "instr len:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(getInstructionLength(readMemory(state, state->pc)), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	pos.y += incY;
	sfText_setString(renderText, "Video Memory Loc:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(state->vid.startAddress, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	sfText_setString(renderText, "Video Memory Dims:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace;
	_itoa(state->vid.width, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 4;
	_itoa(state->vid.height, buf, 10); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY; pos.x = X_INIT_POS;

	#define X_POS_MEM_COL 450
	pos.x = X_POS_MEM_COL;
	pos.y = TEXT_SIZE + 4;

	sfText_setString(renderText, "Memory snippet:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
	// Display the memory content surrounding our current pc
	for (int i = state->pc - 10; i < state->pc + 10; i++) {
		if (boundsCheckMemIndex(state, i)) {
			if (i == state->pc) {
				sfText_setFillColor(renderText, sfColor_fromRGB(255, 0, 0)); // Highlight the current PC in red
			}
			else {
				sfText_setFillColor(renderText, sfColor_fromRGB(255, 255, 255));
			}

			_itoa(i, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 2;
			_itoa(readMemory(state, i), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x = X_POS_MEM_COL;
		}
		pos.y += incY;
	}

	#define X_POS_STACK_COL 600
	pos.x = X_POS_STACK_COL;
	pos.y = TEXT_SIZE + 4;

	sfText_setString(renderText, "Stack:"); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.y += incY;
	// Display the stack
	for (int i = state->sp - 2; i < state->sp + 20; i+=2) {
		if (boundsCheckMemIndex(state, i)) {
			if (i == state->sp) {
				sfText_setFillColor(renderText, sfColor_fromRGB(255, 0, 0)); // Highlight the top of the stack in red
			}
			else {
				sfText_setFillColor(renderText, sfColor_fromRGB(255, 255, 255));
			}

			_itoa(i+2, buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x += xSpace / 2;
			_itoa(readMemory(state, i+1) + (readMemory(state, i+2) << 8), buf, 16); sfText_setString(renderText, buf); sfText_setPosition(renderText, pos); sfRenderWindow_drawText(window, renderText, NULL); pos.x = X_POS_STACK_COL;
		}
		pos.y += incY;
	}

	sfText_destroy(renderText);
}

void updateVideoBuffer(i8080State* state, sfImage* img) {
	sfColor on = sfColor_fromRGB(255, 255, 255);
	sfColor off = sfColor_fromRGB(0, 0, 0);

	for (int x = 0; x < state->vid.width; x++) {
		for (int y = 0; y < state->vid.height; y++) {
			sfImage_setPixel(img, x, y, readMemory(state, state->vid.startAddress + x + (y*state->vid.width)) ? on : off);
		}
	}
}

void handleEvent(const sfEvent* evt, i8080State* state) {
	switch (evt->type) {
	case sfEvtClosed:
		// Close request
		state->valid = false;
		break;

	default:
		//printf("Got unknown event from window\n");
		break;
	}
}

void initGraphics(unsigned int width, unsigned int height) {
	sfVideoMode videoMode;
	videoMode.width = 800;
	videoMode.height = 600;

	sfContextSettings contextSettings;
	contextSettings.majorVersion = 3;
	contextSettings.minorVersion = 2;
	contextSettings.depthBits = 24;
	contextSettings.stencilBits = 8;
	contextSettings.antialiasingLevel = 0;

	window = sfRenderWindow_create(videoMode, "i8080 Emulator", sfDefaultStyle, &contextSettings);

	//font = sfFont_createFromFile("arial.ttf");
	font = sfFont_createFromFile("Consolas.ttf");
	if (font == NULL) {
		log_fatal("Failed to load font");
		exit(-1);
	}

	videoImg = sfImage_create(width, height);
	videoSprite = sfSprite_create();
	sfVector2f pos;
	pos.x = (((float)videoMode.width) / 2) - (((float)width) / 2);
	pos.y = (((float)videoMode.height) / 2) - (((float)height) / 2);
	sfSprite_setPosition(videoSprite, pos);
}

void closeGraphics() {
	sfRenderWindow_close(window);
	sfRenderWindow_destroy(window);

	sfFont_destroy(font);

	sfImage_destroy(videoImg);
	sfSprite_destroy(videoSprite);
}