#include "i8080_test.h"

void utilTest() {
	i8080State* state = malloc(sizeof(i8080State));

	if (state == NULL) {
		log_fatal("Test protocol failure: unable to allocate for state");
		exit(-1);
	}

	FILE* testLog = fopen("i8080_test.log", "w");
	bool success = false;

	int failedTests = 0;

	// reset the state
	init8080(state);

	sfClock* timer = sfClock_create();
	fprintf(testLog, "i8080 Test protocol.\n");

	fprintf(testLog, "--- function tests ---\n");

	success = isZero(0);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test isZero(0x00)=%i\t\t\t\t\t: [%s]\n", isZero(0), success ? "OK" : "FAIL");

	success = !isZero(1);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test isZero(0x01)=%i\t\t\t\t\t: [%s]\n", isZero(1), success ? "OK" : "FAIL");

	success = isNegative(0xFF);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test isNegative(0xFF)=%i\t\t\t\t: [%s]\n", isNegative(0xFF), success ? "OK" : "FAIL");

	success = isParityEven(0xFF);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test isParityEven(0xFF)=%i\t\t\t: [%s]\n", isParityEven(0xFF), success ? "OK" : "FAIL");

	success = !isParityEven(0xFE);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test isParityEven(0xFE)=%i\t\t\t: [%s]\n", isParityEven(0xFE), success ? "OK" : "FAIL");

	addCarry16(state, 0xFFFF, 0x0001);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test addCarry16(0xFFFF, 0x01)=%i\t\t: [%s]\n", addCarry16(state, 0xFFFF, 0x0001), success ? "OK" : "FAIL");

	subCarry16(state, 0x02, 0x03);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test subCarry16(0x02, 0x03)=%i\t: [%s]\n", subCarry16(state, 0x02, 0x03), success ? "OK" : "FAIL");

	fprintf(testLog, "\n--- instruction tests ---\n");

	utilTest_prepNext(state, LXI_B, 0x0F, 0xF0); // Setup the command
	cpuTick(state); // Execute command
	success = state->c == 0x0F && state->b == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_B\t(%02X)\t\t: [%s]\n", LXI_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STAX_B, 0x00, 0x00); state->a = 128; // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0xF00F) == 128;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STAX_B\t(%02X)\t\t: [%s]\n", STAX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_B, 0x00, 0x00); state->a = 128; // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0xF0 && state->c == 0x10;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_B\t(%02X)\t\t: [%s]\n", INX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_B, 0x00, 0x00); state->b = 255; state->a = 128; // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 1 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_B\t(%02X)\t\t: [%s]\n", INR_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_B, 0x00, 0x00); state->a = 128; // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_B\t(%02X)\t\t: [%s]\n", DCR_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_B, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_B\t(%02X)\t\t: [%s]\n", MVI_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RLC, 0x00, 0x00); state->a = 0xF0; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0xE1 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RLC\t(%02X)\t\t: [%s]\n", RLC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_B, 0x00, 0x00); putBC16(state, 0x1); putHL16(state, 0xFFFF); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_B\t(%02X)\t\t: [%s]\n", DAD_B, success ? "OK" : "FAIL"); // Print the result of the test

	// Output statistics
	float elapsedTimeMs = sfTime_asMilliseconds(sfClock_getElapsedTime(timer));
	float elapsedTimeSec = elapsedTimeMs / 1000.0f;
	fprintf(testLog, "--------------------------------------------------\nTest complete!\nTime to complete test: %f ms\nTests failed: %i\n", elapsedTimeSec, failedTests);

	// free the state
	free(state);
	// cloe test file
	fclose(testLog);
}

void utilTest_prepNext(i8080State* state, uint8_t opcode, uint8_t byte1, uint8_t byte2) {
	state->pc = 0;
	state->waitCycles = 0;
	state->memory[0] = opcode;
	state->memory[1] = byte1;
	state->memory[2] = byte2;
}