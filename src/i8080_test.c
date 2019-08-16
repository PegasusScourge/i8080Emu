/*

i8080_test.c

Test protocol function. Massive single opcode test for each opcode

*/

#include "i8080_test.h"

void i8080_testProtocol(i8080State* state) {

	FILE* testLog = fopen("i8080_test.log", "w");
	bool success = false;

	int failedTests = 0;

	// reset the state
	init8080(state);

	state->mode = MODE_TEST; // Set us to test mode

	sfClock* timer = sfClock_create();
	fprintf(testLog, "i8080 Test protocol.\n");

	fprintf(testLog, "--- function tests ---\n");

	success = i8080_isZero(0);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080_isZero(0x00)=%i\t\t\t\t\t: [%s]\n", i8080_isZero(0), success ? "OK" : "FAIL");

	success = !i8080_isZero(1);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080_isZero(0x01)=%i\t\t\t\t\t: [%s]\n", i8080_isZero(1), success ? "OK" : "FAIL");

	success = i8080_isNegative(0xFF);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080_isNegative(0xFF)=%i\t\t\t\t: [%s]\n", i8080_isNegative(0xFF), success ? "OK" : "FAIL");

	success = i8080_isParityEven(0xFF);
	if(!success) { failedTests++; }
	fprintf(testLog, "Test i8080_isParityEven(%02X)=%i\t\t\t\t: [%s]\n", 0xFF, i8080_isParityEven(0xFF), success ? "OK" : "FAIL");

	success = !i8080_isParityEven(0xFE);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080_isParityEven(0xFE)=%i\t\t\t\t: [%s]\n", i8080_isParityEven(0xFE), success ? "OK" : "FAIL");

	i8080op_addCarry16(state, 0xFFFF, 0x0001);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080op_addCarry16(0xFFFF, 0x01)=%i\t\t: [%s]\n", i8080op_addCarry16(state, 0xFFFF, 0x0001), success ? "OK" : "FAIL");

	i8080op_subCarry16(state, 0x02, 0x03);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080op_subCarry16(0x02, 0x03)=%i\t: [%s]\n", i8080op_subCarry16(state, 0x02, 0x03), success ? "OK" : "FAIL");

	i8080op_addCarry8(state, 0xFF, 0x1);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080op_addCarry8(0xFF, 0x01)=%i\t\t: [%s]\n", i8080op_addCarry8(state, 0xFF, 0x1), success ? "OK" : "FAIL");

	i8080op_subCarry8(state, 0x02, 0x03);
	success = state->f.c;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test i8080op_subCarry8(0x02, 0x03)=%i\t\t: [%s]\n", i8080op_subCarry8(state, 0x02, 0x03), success ? "OK" : "FAIL");

	fprintf(testLog, "\n--- instruction tests ---\n");

	utilTest_prepNext(state, LXI_B, 0x0F, 0xF0); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->c == 0x0F && state->b == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_B\t(%02X)\t\t: [%s]\n", LXI_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STAX_B, 0x00, 0x00); state->a = 128; i8080op_putBC16(state, 0xF00F); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0xF00F) == 128;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STAX_B\t(%02X)\t\t: [%s]\n", STAX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_B, 0x00, 0x00); i8080op_putBC16(state, 0x2); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getBC(state) == 0x3;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_B\t(%02X)\t\t: [%s]\n", INX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_B, 0x00, 0x00); state->b = 255; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->b == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 0 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_B\t(%02X)\t\t: [%s]\n", INR_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_B, 0x00, 0x00); state->b = 0x0; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->b == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_B\t(%02X)\t\t: [%s]\n", DCR_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_B, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->b == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_B\t(%02X)\t\t: [%s]\n", MVI_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RLC, 0x00, 0x00); state->a = 0xF0; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0xE1 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RLC\t(%02X)\t\t: [%s]\n", RLC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_B, 0x00, 0x00); i8080op_putBC16(state, 0x1); i8080op_putHL16(state, 0xFFFF); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_B\t(%02X)\t\t: [%s]\n", DAD_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LDAX_B, LDAX_B, 0x00); i8080op_putBC16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == LDAX_B;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDAX_B\t(%02X)\t\t: [%s]\n", LDAX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_B, 0x00, 0x00); i8080op_putBC16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getBC(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_B\t(%02X)\t\t: [%s]\n", DCX_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_C, 0x00, 0x00); state->c = 0x1; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->c == 0x02 && state->f.z == 0 && state->f.s == 0 && state->f.p == 0 && state->f.ac == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_C\t(%02X)\t\t: [%s]\n", INR_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_C, 0x00, 0x00); state->c = 0x2; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->c == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_C\t(%02X)\t\t: [%s]\n", DCR_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_C, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->c == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_C\t(%02X)\t\t: [%s]\n", MVI_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RRC, 0x00, 0x00); state->a = 0x01; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x80 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RRC\t(%02X)\t\t: [%s]\n", RRC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_D, 0x0F, 0xF0); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->e == 0x0F && state->d == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_D\t(%02X)\t\t: [%s]\n", LXI_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STAX_D, 0x00, 0x00); state->a = 128; i8080op_putBC16(state, 0xF00F); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0xF00F) == 128;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STAX_D\t(%02X)\t\t: [%s]\n", STAX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_D, 0x00, 0x00); i8080op_putDE16(state, 0xF0); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getDE(state) == 0xF1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_D\t(%02X)\t\t: [%s]\n", INX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_D, 0x00, 0x00); state->d = 255; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->d == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 0 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_D\t(%02X)\t\t: [%s]\n", INR_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_D, 0x00, 0x00); state->d = 0x0; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->d == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("d:%i z:%i s:%i p:%i ac:%i", state->d, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_D\t(%02X)\t\t: [%s]\n", DCR_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_D, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->d == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_D\t(%02X)\t\t: [%s]\n", MVI_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RAL, 0x00, 0x00); state->a = 0x40; state->f.c = 1;// Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x81 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RAL\t(%02X)\t\t: [%s]\n", RAL, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_D, 0x00, 0x00); i8080op_putDE16(state, 0x1); i8080op_putHL16(state, 0xFFFF); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_D\t(%02X)\t\t: [%s]\n", DAD_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LDAX_D, LDAX_D, 0x00); i8080op_putDE16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == LDAX_D;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDAX_D\t(%02X)\t\t: [%s]\n", LDAX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_D, 0x00, 0x00); i8080op_putDE16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getDE(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_D\t(%02X)\t\t: [%s]\n", DCX_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_E, 0x00, 0x00); state->e = 0x1; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->e == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_E\t(%02X)\t\t: [%s]\n", INR_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_E, 0x00, 0x00); state->e = 0x2; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->e == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_E\t(%02X)\t\t: [%s]\n", DCR_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_E, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->e == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_E\t(%02X)\t\t: [%s]\n", MVI_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, RAR, 0x00, 0x00); state->a = 0x81; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0xC0 && state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test RAR\t(%02X)\t\t: [%s]\n", RAR, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_H, 0x0F, 0xF0); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->l == 0x0F && state->h == 0xF0; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_B\t(%02X)\t\t: [%s]\n", LXI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SHLD, 0xFF, 0x00); i8080op_putHL16(state, 0xFFEE); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0x00FF) == 0xEE && i8080op_readMemory(state, 0x0100) == 0xFF;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test SHLD\t(%02X)\t\t: [%s]\n", SHLD, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_H, 0x00, 0x00); i8080op_putHL16(state, 0x2); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x3;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_H\t(%02X)\t\t: [%s]\n", INX_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_H, 0x00, 0x00); state->h = 255; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->h == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 0 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_H\t(%02X)\t\t: [%s]\n", INR_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_H, 0x00, 0x00); state->h = 0x0; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->h == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_H\t(%02X)\t\t: [%s]\n", DCR_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_H, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->h == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_H\t(%02X)\t\t: [%s]\n", MVI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_H, 0x00, 0x00); i8080op_putHL16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x2 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_H\t(%02X)\t\t: [%s]\n", DAD_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LHLD, 0xFF, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0xFFEE;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LHLD\t(%02X)\t\t: [%s]\n", LHLD, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_H, 0x00, 0x00); i8080op_putHL16(state, 0x1); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_H\t(%02X)\t\t: [%s]\n", DCX_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_L, 0x00, 0x00); state->l = 0x1; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->l == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_L\t(%02X)\t\t: [%s]\n", INR_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_L, 0x00, 0x00); state->l = 0x2; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->l == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_L\t(%02X)\t\t: [%s]\n", DCR_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_L, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->l == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_L\t(%02X)\t\t: [%s]\n", MVI_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMA, 0x00, 0x00); state->a = 0xFF; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x00;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test CMA\t(%02X)\t\t: [%s]\n", CMA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LXI_SP, 0x01, 0x24); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->sp == 0x2401; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LXI_SP\t(%02X)\t\t: [%s]\n", LXI_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STA, 0xFF, 0x00); state->a = 0x42; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0x00FF) == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STA\t(%02X)\t\t: [%s]\n", STA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INX_SP, 0x00, 0x00); state->sp = 0x2401; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->sp == 0x2402; // Check success conditions
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INX_SP\t(%02X)\t\t: [%s]\n", INX_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_M, 0x00, 0x00); i8080op_writeMemory(state, 0x00FF, 0xFF); i8080op_putHL16(state, 0x00FF); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0x00FF) == 0x00 && state->f.z == 1 && state->f.s == 0 && state->f.p == 0 && state->f.ac == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_M\t(%02X)\t\t: [%s]\n", INR_M, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_M, 0x00, 0x00); i8080op_writeMemory(state, 0x00FF, 0x00); i8080op_putHL16(state, 0x00FF); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0x00FF) == 0xFF && state->f.z == 0 && state->f.s == 1 && state->f.p == 1 && state->f.ac == 0;
	//log_debug("b:%i z:%i s:%i p:%i ac:%i", state->b, state->f.z, state->f.s, state->f.p, state->f.ac);
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_M\t(%02X)\t\t: [%s]\n", DCR_M, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_M, 0x42, 0x00); i8080op_putHL16(state, 0x40); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_readMemory(state, 0x40) == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_M\t(%02X)\t\t: [%s]\n", MVI_M, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DAD_SP, 0x00, 0x00); i8080op_putHL16(state, 0x1); state->sp = 0x2;// Setup the command
	i8080_cpuTick(state); // Execute command
	success = i8080op_getHL(state) == 0x3 && state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DAD_SP\t(%02X)\t\t: [%s]\n", DAD_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, LDA, 0x00, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == LDA;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test LDA\t(%02X)\t\t: [%s]\n", LDA, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCX_SP, 0x00, 0x00); state->sp = 0x2; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->sp == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCX_SP\t(%02X)\t\t: [%s]\n", DCX_SP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, INR_A, 0x00, 0x00); state->a = 0x1; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x02;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test INR_A\t(%02X)\t\t: [%s]\n", INR_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, DCR_A, 0x00, 0x00); state->a = 0x2; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x01;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test DCR_A\t(%02X)\t\t: [%s]\n", DCR_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, MVI_A, 0x42, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->a == 0x42;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test MVI_A\t(%02X)\t\t: [%s]\n", MVI_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMC, 0x00, 0x00); state->f.c = 1; // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->f.c == 0;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test CMC\t(%02X)\t\t: [%s]\n", CMC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, STC, 0x00, 0x00); // Setup the command
	i8080_cpuTick(state); // Execute command
	success = state->f.c == 1;
	if (!success) { failedTests++; }
	fprintf(testLog, "Test STC\t(%02X)\t\t: [%s]\n", STC, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test B
	utilTest_prepNext(state, MOV_BB, 0x00, 0x00); state->b = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BB\t(%02X)\t\t: [%s]\n", MOV_BB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BC, 0x00, 0x00); state->b = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BC\t(%02X)\t\t: [%s]\n", MOV_BC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BD, 0x00, 0x00); state->b = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BD\t(%02X)\t\t: [%s]\n", MOV_BD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BE, 0x00, 0x00); state->b = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BE\t(%02X)\t\t: [%s]\n", MOV_BE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BH, 0x00, 0x00); state->b = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BH\t(%02X)\t\t: [%s]\n", MOV_BH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BL, 0x00, 0x00); state->b = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BL\t(%02X)\t\t: [%s]\n", MOV_BL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BA, 0x00, 0x00); state->b = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BA\t(%02X)\t\t: [%s]\n", MOV_BA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_BM, 0x00, 0x00); state->b = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->b == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_BM\t(%02X)\t\t: [%s]\n", MOV_BM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test C
	utilTest_prepNext(state, MOV_CB, 0x00, 0x00); state->c = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CB\t(%02X)\t\t: [%s]\n", MOV_CB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CC, 0x00, 0x00); state->c = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CC\t(%02X)\t\t: [%s]\n", MOV_CC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CD, 0x00, 0x00); state->c = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CD\t(%02X)\t\t: [%s]\n", MOV_CD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CE, 0x00, 0x00); state->c = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CE\t(%02X)\t\t: [%s]\n", MOV_CE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CH, 0x00, 0x00); state->c = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CH\t(%02X)\t\t: [%s]\n", MOV_CH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CL, 0x00, 0x00); state->c = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CL\t(%02X)\t\t: [%s]\n", MOV_CL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CA, 0x00, 0x00); state->c = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CA\t(%02X)\t\t: [%s]\n", MOV_CA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_CM, 0x00, 0x00); state->c = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->c == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_CM\t(%02X)\t\t: [%s]\n", MOV_CM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test D
	utilTest_prepNext(state, MOV_DB, 0x00, 0x00); state->d = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DB\t(%02X)\t\t: [%s]\n", MOV_DB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DC, 0x00, 0x00); state->d = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DC\t(%02X)\t\t: [%s]\n", MOV_DC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DD, 0x00, 0x00); state->d = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DD\t(%02X)\t\t: [%s]\n", MOV_DD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DE, 0x00, 0x00); state->d = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DE\t(%02X)\t\t: [%s]\n", MOV_DE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DH, 0x00, 0x00); state->d = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DH\t(%02X)\t\t: [%s]\n", MOV_DH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DL, 0x00, 0x00); state->d = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DL\t(%02X)\t\t: [%s]\n", MOV_DL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DA, 0x00, 0x00); state->d = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DA\t(%02X)\t\t: [%s]\n", MOV_DA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_DM, 0x00, 0x00); state->d = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->d == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_DM\t(%02X)\t\t: [%s]\n", MOV_DM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test E
	utilTest_prepNext(state, MOV_EB, 0x00, 0x00); state->e = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EB\t(%02X)\t\t: [%s]\n", MOV_EB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EC, 0x00, 0x00); state->e = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EC\t(%02X)\t\t: [%s]\n", MOV_EC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_ED, 0x00, 0x00); state->e = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_ED\t(%02X)\t\t: [%s]\n", MOV_ED, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EE, 0x00, 0x00); state->e = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EE\t(%02X)\t\t: [%s]\n", MOV_EE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EH, 0x00, 0x00); state->e = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EH\t(%02X)\t\t: [%s]\n", MOV_EH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EL, 0x00, 0x00); state->e = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EL\t(%02X)\t\t: [%s]\n", MOV_EL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EA, 0x00, 0x00); state->e = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EA\t(%02X)\t\t: [%s]\n", MOV_EA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_EM, 0x00, 0x00); state->e = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->e == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_EM\t(%02X)\t\t: [%s]\n", MOV_EM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test H
	utilTest_prepNext(state, MOV_HB, 0x00, 0x00); state->h = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HB\t(%02X)\t\t: [%s]\n", MOV_HB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HC, 0x00, 0x00); state->h = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HC\t(%02X)\t\t: [%s]\n", MOV_HC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HD, 0x00, 0x00); state->h = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HD\t(%02X)\t\t: [%s]\n", MOV_HD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HE, 0x00, 0x00); state->h = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HE\t(%02X)\t\t: [%s]\n", MOV_HE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HH, 0x00, 0x00); state->h = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HH\t(%02X)\t\t: [%s]\n", MOV_HH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HL, 0x00, 0x00); state->h = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HL\t(%02X)\t\t: [%s]\n", MOV_HL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HA, 0x00, 0x00); state->h = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HA\t(%02X)\t\t: [%s]\n", MOV_HA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_HM, 0x00, 0x00); state->h = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->h == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_HM\t(%02X)\t\t: [%s]\n", MOV_HM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test L
	utilTest_prepNext(state, MOV_LB, 0x00, 0x00); state->l = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LB\t(%02X)\t\t: [%s]\n", MOV_LB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LC, 0x00, 0x00); state->l = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LC\t(%02X)\t\t: [%s]\n", MOV_LC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LD, 0x00, 0x00); state->l = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LD\t(%02X)\t\t: [%s]\n", MOV_LD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LE, 0x00, 0x00); state->l = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LE\t(%02X)\t\t: [%s]\n", MOV_LE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LH, 0x00, 0x00); state->l = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LH\t(%02X)\t\t: [%s]\n", MOV_LH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LL, 0x00, 0x00); state->l = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LL\t(%02X)\t\t: [%s]\n", MOV_LL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LA, 0x00, 0x00); state->l = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LA\t(%02X)\t\t: [%s]\n", MOV_LA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_LM, 0x00, 0x00); state->l = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->l == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_LM\t(%02X)\t\t: [%s]\n", MOV_LM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test A
	utilTest_prepNext(state, MOV_AB, 0x00, 0x00); state->a = 0xF; state->b = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AB\t(%02X)\t\t: [%s]\n", MOV_AB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AC, 0x00, 0x00); state->a = 0xF; state->c = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AC\t(%02X)\t\t: [%s]\n", MOV_AC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AD, 0x00, 0x00); state->a = 0xF; state->d = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AD\t(%02X)\t\t: [%s]\n", MOV_AD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AE, 0x00, 0x00); state->a = 0xF; state->e = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AE\t(%02X)\t\t: [%s]\n", MOV_AE, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AH, 0x00, 0x00); state->a = 0xF; state->h = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AH\t(%02X)\t\t: [%s]\n", MOV_AH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AL, 0x00, 0x00); state->a = 0xF; state->l = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AL\t(%02X)\t\t: [%s]\n", MOV_AL, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AA, 0x00, 0x00); state->a = 0xF; state->a = 0xA; i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AA\t(%02X)\t\t: [%s]\n", MOV_AA, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_AM, 0x00, 0x00); state->a = 0xF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xA); i8080_cpuTick(state);
	success = state->a == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_AM\t(%02X)\t\t: [%s]\n", MOV_AM, success ? "OK" : "FAIL"); // Print the result of the test

	// Mov test M
	utilTest_prepNext(state, MOV_MB, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); state->b = 0xA; i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_MB\t(%02X)\t\t: [%s]\n", MOV_MB, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_MC, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); state->c = 0xA; i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_MC\t(%02X)\t\t: [%s]\n", MOV_MC, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_MD, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); state->d = 0xA; i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_MD\t(%02X)\t\t: [%s]\n", MOV_MD, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_ME, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); state->e = 0xA; i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_ME\t(%02X)\t\t: [%s]\n", MOV_ME, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_MH, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0x00; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_MH\t(%02X)\t\t: [%s]\n", MOV_MH, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_ML, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xAA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_ML\t(%02X)\t\t: [%s]\n", MOV_ML, success ? "OK" : "FAIL"); // Print the result of the test
	utilTest_prepNext(state, MOV_MA, 0x00, 0x00); i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xF); state->a = 0xA; i8080_cpuTick(state);
	success = i8080op_readMemory(state, 0x00AA) == 0xA; if (!success) { failedTests++; } fprintf(testLog, "Test MOV_MA\t(%02X)\t\t: [%s]\n", MOV_MA, success ? "OK" : "FAIL"); // Print the result of the test

	// ADD test
	utilTest_prepNext(state, ADD_B, 0x00, 0x00); state->a = 0xFF; state->b = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; } 
	fprintf(testLog, "Test ADD_B\t(%02X)\t\t: [%s]\n", ADD_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_C, 0x00, 0x00); state->a = 0xFF; state->c = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_C\t(%02X)\t\t: [%s]\n", ADD_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_D, 0x00, 0x00); state->a = 0xFF; state->d = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_D\t(%02X)\t\t: [%s]\n", ADD_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_E, 0x00, 0x00); state->a = 0xFF; state->e = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_E\t(%02X)\t\t: [%s]\n", ADD_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_H, 0x00, 0x00); state->a = 0xFF; state->h = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_H\t(%02X)\t\t: [%s]\n", ADD_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_L, 0x00, 0x00); state->a = 0xFF; state->l = 0x1; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_L\t(%02X)\t\t: [%s]\n", ADD_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_A, 0x00, 0x00); state->a = 0x1; i8080_cpuTick(state);
	success = state->a == 0x02 && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_A\t(%02X)\t\t: [%s]\n", ADD_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADD_M, 0x00, 0x00); state->a = 0xFF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x1); i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 1 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADD_M\t(%02X)\t\t: [%s]\n", ADD_M, success ? "OK" : "FAIL"); // Print the result of the test

	// ADC test
	utilTest_prepNext(state, ADC_B, 0x00, 0x00); state->a = 0xFF; state->b = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_B\t(%02X)\t\t: [%s]\n", ADC_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_C, 0x00, 0x00); state->a = 0xFF; state->c = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_C\t(%02X)\t\t: [%s]\n", ADC_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_D, 0x00, 0x00); state->a = 0xFF; state->d = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_D\t(%02X)\t\t: [%s]\n", ADC_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_E, 0x00, 0x00); state->a = 0xFF; state->e = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_E\t(%02X)\t\t: [%s]\n", ADC_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_H, 0x00, 0x00); state->a = 0xFF; state->h = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_H\t(%02X)\t\t: [%s]\n", ADC_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_L, 0x00, 0x00); state->a = 0xFF; state->l = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_L\t(%02X)\t\t: [%s]\n", ADC_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_A, 0x00, 0x00); state->a = 0x1; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x03 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_A\t(%02X)\t\t: [%s]\n", ADC_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ADC_M, 0x00, 0x00); state->a = 0xFF; state->f.c = 1; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x1); i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADC_M\t(%02X)\t\t: [%s]\n", ADC_M, success ? "OK" : "FAIL"); // Print the result of the test

	// SUB test
	utilTest_prepNext(state, SUB_B, 0x00, 0x00); state->a = 0xFE; state->b = 0xFF; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_B\t(%02X)\t\t: [%s]\n", SUB_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_C, 0x00, 0x00); state->a = 0xFE; state->c = 0xFF; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_C\t(%02X)\t\t: [%s]\n", SUB_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_D, 0x00, 0x00); state->a = 0xFE; state->d = 0xFF; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_D\t(%02X)\t\t: [%s]\n", SUB_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_E, 0x00, 0x00); state->a = 0xFE; state->e = 0x01; i8080_cpuTick(state);
	success = state->a == 0xFD && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_E\t(%02X)\t\t: [%s]\n", SUB_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_H, 0x00, 0x00); state->a = 0xFE; state->h = 0x01; i8080_cpuTick(state);
	success = state->a == 0xFD && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_H\t(%02X)\t\t: [%s]\n", SUB_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_L, 0x00, 0x00); state->a = 0xFE; state->l = 0x01; i8080_cpuTick(state);
	success = state->a == 0xFD && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_L\t(%02X)\t\t: [%s]\n", SUB_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_A, 0x00, 0x00); state->a = 0x01; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_A\t(%02X)\t\t: [%s]\n", SUB_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SUB_M, 0x00, 0x00); state->a = 0xFF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x1); i8080_cpuTick(state);
	success = state->a == 0xFE && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUB_M\t(%02X)\t\t: [%s]\n", SUB_M, success ? "OK" : "FAIL"); // Print the result of the test

	// SBB test
	utilTest_prepNext(state, SBB_B, 0x00, 0x00); state->a = 0xFF; state->b = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_B\t(%02X)\t\t: [%s]\n", SBB_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_C, 0x00, 0x00); state->a = 0xFF; state->c = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_C\t(%02X)\t\t: [%s]\n", SBB_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_D, 0x00, 0x00); state->a = 0xFF; state->d = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_D\t(%02X)\t\t: [%s]\n", SBB_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_E, 0x00, 0x00); state->a = 0xFF; state->e = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_E\t(%02X)\t\t: [%s]\n", SBB_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_H, 0x00, 0x00); state->a = 0xFF; state->h = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_H\t(%02X)\t\t: [%s]\n", SBB_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_L, 0x00, 0x00); state->a = 0xFF; state->l = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_L\t(%02X)\t\t: [%s]\n", SBB_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_A, 0x00, 0x00); state->a = 0xFF; state->a = 0xFF; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_A\t(%02X)\t\t: [%s]\n", SBB_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, SBB_M, 0x00, 0x00); state->a = 0xFF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0xFF); state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBB_M\t(%02X)\t\t: [%s]\n", SBB_M, success ? "OK" : "FAIL"); // Print the result of the test

	// ANA testing
	utilTest_prepNext(state, ANA_B, 0x00, 0x00); state->a = 0xFF; state->b = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_B\t(%02X)\t\t: [%s]\n", ANA_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_C, 0x00, 0x00); state->a = 0xFF; state->c = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_C\t(%02X)\t\t: [%s]\n", ANA_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_D, 0x00, 0x00); state->a = 0xFF; state->d = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_D\t(%02X)\t\t: [%s]\n", ANA_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_E, 0x00, 0x00); state->a = 0xFF; state->e = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_E\t(%02X)\t\t: [%s]\n", ANA_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_H, 0x00, 0x00); state->a = 0xFF; state->h = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_H\t(%02X)\t\t: [%s]\n", ANA_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_L, 0x00, 0x00); state->a = 0xFF; state->l = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_L\t(%02X)\t\t: [%s]\n", ANA_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_A, 0x00, 0x00); state->a = 0xFF; state->a = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_A\t(%02X)\t\t: [%s]\n", ANA_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ANA_M, 0x00, 0x00); state->a = 0xFF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x0F); i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANA_M\t(%02X)\t\t: [%s]\n", ANA_M, success ? "OK" : "FAIL"); // Print the result of the test

	// XRA testing
	utilTest_prepNext(state, XRA_B, 0x00, 0x00); state->a = 0xFF; state->b = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_B\t(%02X)\t\t: [%s]\n", XRA_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_C, 0x00, 0x00); state->a = 0xFF; state->c = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_C\t(%02X)\t\t: [%s]\n", XRA_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_D, 0x00, 0x00); state->a = 0xFF; state->d = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_D\t(%02X)\t\t: [%s]\n", XRA_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_E, 0x00, 0x00); state->a = 0xFF; state->b = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_B\t(%02X)\t\t: [%s]\n", XRA_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_H, 0x00, 0x00); state->a = 0xFF; state->h = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_H\t(%02X)\t\t: [%s]\n", XRA_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_L, 0x00, 0x00); state->a = 0xFF; state->l = 0x0F; i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_L\t(%02X)\t\t: [%s]\n", XRA_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_A, 0x00, 0x00); state->a = 0xFF; i8080_cpuTick(state);
	success = state->a == 0x00 && state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_B\t(%02X)\t\t: [%s]\n", XRA_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, XRA_M, 0x00, 0x00); state->a = 0xFF; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x0F); i8080_cpuTick(state);
	success = state->a == 0xF0 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRA_M\t(%02X)\t\t: [%s]\n", XRA_M, success ? "OK" : "FAIL"); // Print the result of the test

	// ORA testing
	utilTest_prepNext(state, ORA_B, 0x00, 0x00); state->a = 0x10; state->b = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_B\t(%02X)\t\t: [%s]\n", ORA_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_C, 0x00, 0x00); state->a = 0x10; state->c = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_C\t(%02X)\t\t: [%s]\n", ORA_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_D, 0x00, 0x00); state->a = 0x10; state->d = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_D\t(%02X)\t\t: [%s]\n", ORA_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_E, 0x00, 0x00); state->a = 0x10; state->e = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_E\t(%02X)\t\t: [%s]\n", ORA_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_H, 0x00, 0x00); state->a = 0x10; state->h = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_H\t(%02X)\t\t: [%s]\n", ORA_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_L, 0x00, 0x00); state->a = 0x10; state->l = 0x0C; i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_L\t(%02X)\t\t: [%s]\n", ORA_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_A, 0x00, 0x00); state->a = 0x0F; i8080_cpuTick(state);
	success = state->a == 0x0F && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_A\t(%02X)\t\t: [%s]\n", ORA_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, ORA_M, 0x00, 0x00); state->a = 0x10; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x0C); i8080_cpuTick(state);
	success = state->a == 0x1C && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORA_M\t(%02X)\t\t: [%s]\n", ORA_M, success ? "OK" : "FAIL"); // Print the result of the test

	// CMP testing
	utilTest_prepNext(state, CMP_B, 0x00, 0x00); state->a = 0x10; state->b = 0x10; i8080_cpuTick(state);
	success = state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_B\t(%02X)\t\t: [%s]\n", CMP_B, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_C, 0x00, 0x00); state->a = 0x10; state->c = 0x10; i8080_cpuTick(state);
	success = state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_C\t(%02X)\t\t: [%s]\n", CMP_C, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_D, 0x00, 0x00); state->a = 0x10; state->d = 0x10; i8080_cpuTick(state);
	success = state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_D\t(%02X)\t\t: [%s]\n", CMP_D, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_E, 0x00, 0x00); state->a = 0x10; state->e = 0x11; i8080_cpuTick(state);
	success = state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_E\t(%02X)\t\t: [%s]\n", CMP_E, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_H, 0x00, 0x00); state->a = 0x10; state->h = 0x11; i8080_cpuTick(state);
	success = state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_H\t(%02X)\t\t: [%s]\n", CMP_H, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_L, 0x00, 0x00); state->a = 0x10; state->l = 0x11; i8080_cpuTick(state);
	success = state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_L\t(%02X)\t\t: [%s]\n", CMP_L, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_A, 0x00, 0x00); state->a = 0x10; i8080_cpuTick(state);
	success = state->f.c == 0 && state->f.z == 1 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_A\t(%02X)\t\t: [%s]\n", CMP_A, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, CMP_M, 0x00, 0x00); state->a = 0x10; i8080op_putHL16(state, 0x00AA); i8080op_writeMemory(state, 0x00AA, 0x11);  i8080_cpuTick(state);
	success = state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test CMP_M\t(%02X)\t\t: [%s]\n", CMP_M, success ? "OK" : "FAIL"); // Print the result of the test

	// ADI test
	utilTest_prepNext(state, ADI, 0x01, 0x00); state->a = 0x10; i8080_cpuTick(state);
	success = state->a == 0x11 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ADI\t(%02X)\t\t: [%s]\n", ADI, success ? "OK" : "FAIL"); // Print the result of the test

	// ACI test
	utilTest_prepNext(state, ACI, 0x01, 0x00); state->a = 0x10; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0x12 && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ACI\t(%02X)\t\t: [%s]\n", ACI, success ? "OK" : "FAIL"); // Print the result of the test

	// SUI test
	utilTest_prepNext(state, SUI, 0xFF, 0x00); state->a = 0xFE; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 1 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test SUI\t(%02X)\t\t: [%s]\n", SUI, success ? "OK" : "FAIL"); // Print the result of the test

	// SBI test
	utilTest_prepNext(state, SBI, 0xFF, 0x00); state->a = 0xFE; state->f.c = 1; i8080_cpuTick(state);
	success = state->a == 0xFE && state->f.c == 1 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test SBI\t(%02X)\t\t: [%s]\n", SBI, success ? "OK" : "FAIL"); // Print the result of the test

	// ANI test
	utilTest_prepNext(state, ANI, 0x01, 0x00); state->a = 0xFF; i8080_cpuTick(state);
	success = state->a == 0x01 && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test ANI\t(%02X)\t\t: [%s]\n", ANI, success ? "OK" : "FAIL"); // Print the result of the test

	// XRI test
	utilTest_prepNext(state, XRI, 0x18, 0x00); state->a = 0x08; i8080_cpuTick(state);
	success = state->a == 0x10 && state->f.c == 0 && state->f.z == 0 && state->f.p == 0; if (!success) { failedTests++; }
	fprintf(testLog, "Test XRI\t(%02X)\t\t: [%s]\n", XRI, success ? "OK" : "FAIL"); // Print the result of the test

	// ORI test
	utilTest_prepNext(state, ORI, 0x0F, 0x00); state->a = 0xF0; i8080_cpuTick(state);
	success = state->a == 0xFF && state->f.c == 0 && state->f.z == 0 && state->f.p == 1; if (!success) { failedTests++; }
	fprintf(testLog, "Test ORI\t(%02X)\t\t: [%s]\n", ORI, success ? "OK" : "FAIL"); // Print the result of the test

	/* Move into the stack related functions */

	// Jumping functions
	utilTest_prepNext(state, JNZ, 0xFF, 0xFF); state->f.z = 0; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JNZ\t(%02X)\t\t: [%s]\n", JNZ, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JMP, 0xFF, 0xFF); i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JMP\t(%02X)\t\t: [%s]\n", JMP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JZ, 0xFF, 0xFF); state->f.z = 1; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JZ \t(%02X)\t\t: [%s]\n", JZ, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JNC, 0xFF, 0xFF); state->f.c = 0; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JNC\t(%02X)\t\t: [%s]\n", JNC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JC, 0xFF, 0xFF); state->f.c = 1; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JC \t(%02X)\t\t: [%s]\n", JC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JNC, 0xFF, 0xFF); state->f.c = 0; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JNC\t(%02X)\t\t: [%s]\n", JNC, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JPO, 0xFF, 0xFF); state->f.p = 0; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JPO\t(%02X)\t\t: [%s]\n", JPO, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JPE, 0xFF, 0xFF); state->f.p = 1; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JPE\t(%02X)\t\t: [%s]\n", JPE, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JP, 0xFF, 0xFF); state->f.s = 0; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JP \t(%02X)\t\t: [%s]\n", JP, success ? "OK" : "FAIL"); // Print the result of the test

	utilTest_prepNext(state, JM, 0xFF, 0xFF); state->f.s = 1; i8080_cpuTick(state);
	success = state->pc == 0xFFFF; if (!success) { failedTests++; }
	fprintf(testLog, "Test JM \t(%02X)\t\t: [%s]\n", JM, success ? "OK" : "FAIL"); // Print the result of the test
	
	// Output statistics
	float elapsedTimeMs = sfTime_asMilliseconds(sfClock_getElapsedTime(timer));
	float elapsedTimeSec = elapsedTimeMs / 1000.0f;
	fprintf(testLog, "--------------------------------------------------\nTest complete!\nTime to complete test: %f seconds\nTests failed: %i\n", elapsedTimeSec, failedTests);

	// cloe test file
	fclose(testLog);
}

void utilTest_prepStack(i8080State* state, uint16_t newSp) {
	i8080op_setSP(state, newSp);
	for (uint16_t i = state->sp - 10; i < state->sp + 10; i++) {
		i8080op_writeMemory(state, i, 0);
	}
}

void utilTest_prepNext(i8080State* state, uint8_t opcode, uint8_t byte1, uint8_t byte2) {
	state->pc = 0;
	state->waitCycles = 0;
	state->memory[0] = opcode;
	state->memory[1] = byte1;
	state->memory[2] = byte2;
}
