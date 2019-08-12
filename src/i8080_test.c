/*

i8080_test.c

Test protocol

*/

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

	addCarry(state, 0xFF, 0x1);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test addCarry(0xFF, 0x01)=%i\t\t\t: [%s]\n", addCarry(state, 0xFF, 0x1), success ? "OK" : "FAIL");

	subCarry(state, 0x02, 0x03);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test subCarry(0x02, 0x03)=%i\t\t: [%s]\n", subCarry(state, 0x02, 0x03), success ? "OK" : "FAIL");

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

	utilTest_prepNext(state, INX_B, 0x00, 0x00); putBC16(state, 0x2); // Setup the command
	cpuTick(state); // Execute command
	success = getBC(state) == 0x3;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_B\t(%02X)\t\t: [%s]\n", INX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_B, 0x00, 0x00); state->b = 255; // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 1 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_B\t(%02X)\t\t: [%s]\n", INR_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_B, 0x00, 0x00); state->b = 0x0; // Setup the command
	cpuTick(state); // Execute command
	success = state->b == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
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

	utilTest_prepNext(state, LDAX_B, LDAX_B, 0x00); putBC16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = state->a == LDAX_B;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDAX_B\t(%02X)\t\t: [%s]\n", LDAX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_B, 0x00, 0x00); putBC16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = getBC(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_B\t(%02X)\t\t: [%s]\n", DCX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_C, 0x00, 0x00); state->c = 0x1; // Setup the command
	cpuTick(state); // Execute command
	success = state->c == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_C\t(%02X)\t\t: [%s]\n", INR_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_C, 0x00, 0x00); state->c = 0x2; // Setup the command
	cpuTick(state); // Execute command
	success = state->c == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_C\t(%02X)\t\t: [%s]\n", DCR_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_C, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->c == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_C\t(%02X)\t\t: [%s]\n", MVI_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RRC, 0x00, 0x00); state->a = 0x01; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x80 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RRC\t(%02X)\t\t: [%s]\n", RRC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_D, 0x0F, 0xF0); // Setup the command
	cpuTick(state); // Execute command
	success = state->e == 0x0F && state->d == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_D\t(%02X)\t\t: [%s]\n", LXI_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STAX_D, 0x00, 0x00); state->a = 128; // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0xF00F) == 128;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STAX_B\t(%02X)\t\t: [%s]\n", STAX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_D, 0x00, 0x00); putDE16(state, 0xF0); // Setup the command
	cpuTick(state); // Execute command
	success = getDE(state) == 0xF1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_D\t(%02X)\t\t: [%s]\n", INX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_D, 0x00, 0x00); state->d = 255; // Setup the command
	cpuTick(state); // Execute command
	success = state->d == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 1 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_D\t(%02X)\t\t: [%s]\n", INR_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_D, 0x00, 0x00); state->d = 0x0; // Setup the command
	cpuTick(state); // Execute command
	success = state->d == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("d:%i z:%i s:%i p:%i ac:%i", state->d, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_D\t(%02X)\t\t: [%s]\n", DCR_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_D, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->d == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_D\t(%02X)\t\t: [%s]\n", MVI_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RAL, 0x00, 0x00); state->a = 0x40; state->f.c = 1;// Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x81 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RAL\t(%02X)\t\t: [%s]\n", RAL, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_D, 0x00, 0x00); putDE16(state, 0x1); putHL16(state, 0xFFFF); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_D\t(%02X)\t\t: [%s]\n", DAD_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LDAX_D, LDAX_D, 0x00); putDE16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = state->a == LDAX_D;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDAX_D\t(%02X)\t\t: [%s]\n", LDAX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_D, 0x00, 0x00); putDE16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = getDE(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_D\t(%02X)\t\t: [%s]\n", DCX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_E, 0x00, 0x00); state->e = 0x1; // Setup the command
	cpuTick(state); // Execute command
	success = state->e == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_E\t(%02X)\t\t: [%s]\n", INR_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_E, 0x00, 0x00); state->e = 0x2; // Setup the command
	cpuTick(state); // Execute command
	success = state->e == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_E\t(%02X)\t\t: [%s]\n", DCR_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_E, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->e == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_E\t(%02X)\t\t: [%s]\n", MVI_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RAR, 0x00, 0x00); state->a = 0x81; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0xC0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RAR\t(%02X)\t\t: [%s]\n", RAR, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_H, 0x0F, 0xF0); // Setup the command
	cpuTick(state); // Execute command
	success = state->l == 0x0F && state->h == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_B\t(%02X)\t\t: [%s]\n", LXI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SHLD, 0xFF, 0x00); putHL16(state, 0xFFEE); // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0x00FF) == 0xEE && readMemory(state, 0x0100) == 0xFF;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test SHLD\t(%02X)\t\t: [%s]\n", SHLD, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_H, 0x00, 0x00); putHL16(state, 0x2); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x3;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_H\t(%02X)\t\t: [%s]\n", INX_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_H, 0x00, 0x00); state->h = 255; // Setup the command
	cpuTick(state); // Execute command
	success = state->h == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 1 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_H\t(%02X)\t\t: [%s]\n", INR_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_H, 0x00, 0x00); state->h = 0x0; // Setup the command
	cpuTick(state); // Execute command
	success = state->h == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_H\t(%02X)\t\t: [%s]\n", DCR_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_H, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->h == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_H\t(%02X)\t\t: [%s]\n", MVI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_H, 0x00, 0x00); putHL16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x2 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_H\t(%02X)\t\t: [%s]\n", DAD_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LHLD, 0xFF, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0xFFEE;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LHLD\t(%02X)\t\t: [%s]\n", LHLD, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_H, 0x00, 0x00); putHL16(state, 0x1); // Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_H\t(%02X)\t\t: [%s]\n", DCX_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_L, 0x00, 0x00); state->l = 0x1; // Setup the command
	cpuTick(state); // Execute command
	success = state->l == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_L\t(%02X)\t\t: [%s]\n", INR_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_L, 0x00, 0x00); state->l = 0x2; // Setup the command
	cpuTick(state); // Execute command
	success = state->l == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_L\t(%02X)\t\t: [%s]\n", DCR_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_L, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->l == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_L\t(%02X)\t\t: [%s]\n", MVI_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMA, 0x00, 0x00); state->a = 0xFF; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test CMA\t(%02X)\t\t: [%s]\n", CMA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_SP, 0x01, 0x24); // Setup the command
	cpuTick(state); // Execute command
	success = state->sp == 0x2401; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_SP\t(%02X)\t\t: [%s]\n", LXI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STA, 0xFF, 0x00); state->a = 0x42; // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0x00FF) == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STA\t(%02X)\t\t: [%s]\n", STA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_SP, 0x00, 0x00); state->sp = 0x2401; // Setup the command
	cpuTick(state); // Execute command
	success = state->sp == 0x2402; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_SP\t(%02X)\t\t: [%s]\n", INX_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_M, 0x00, 0x00); writeMemory(state, 0x00FF, 0xFF); putHL16(state, 0x00FF); // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0x00FF) == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 1 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_M\t(%02X)\t\t: [%s]\n", INR_M, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_M, 0x00, 0x00); writeMemory(state, 0x00FF, 0x00); putHL16(state, 0x00FF); // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0x00FF) == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_M\t(%02X)\t\t: [%s]\n", DCR_M, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_M, 0x42, 0x00); putHL16(state, 0x40); // Setup the command
	cpuTick(state); // Execute command
	success = readMemory(state, 0x40) == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_M\t(%02X)\t\t: [%s]\n", MVI_M, success ? "OK" : "FAIL"); // Print the result of the test

	// note

	utilTest_prepNext(state, DAD_SP, 0x00, 0x00); putHL16(state, 0x1); state->sp = 0x2;// Setup the command
	cpuTick(state); // Execute command
	success = getHL(state) == 0x3 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_SP\t(%02X)\t\t: [%s]\n", DAD_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LDA, 0x00, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->a == LDA;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDA\t(%02X)\t\t: [%s]\n", LDA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_SP, 0x00, 0x00); state->sp = 0x2; // Setup the command
	cpuTick(state); // Execute command
	success = state->sp == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_SP\t(%02X)\t\t: [%s]\n", DCX_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_A, 0x00, 0x00); state->a = 0x1; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_A\t(%02X)\t\t: [%s]\n", INR_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_A, 0x00, 0x00); state->a = 0x2; // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_A\t(%02X)\t\t: [%s]\n", DCR_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_A, 0x42, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->a == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_A\t(%02X)\t\t: [%s]\n", MVI_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMC, 0x00, 0x00); state->f.c = 1; // Setup the command
	cpuTick(state); // Execute command
	success = state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test CMC\t(%02X)\t\t: [%s]\n", CMC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STC, 0x00, 0x00); // Setup the command
	cpuTick(state); // Execute command
	success = state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STC\t(%02X)\t\t: [%s]\n", STC, success ? "OK" : "FAIL"); // Print the result of the test

	// Output statistics
	float elapsedTimeMs = sfTime_asMilliseconds(sfClock_getElapsedTime(timer));
	float elapsedTimeSec = elapsedTimeMs / 1000.0f;
	fprintf(testLog, "--------------------------------------------------\nTest complete!\nTime to complete test: %f seconds\nTests failed: %i\n", elapsedTimeSec, failedTests);

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