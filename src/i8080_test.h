#pragma once
/*

i8080_test.h

Test protocol function. Massive single opcode test for each opcode

*/


#include "i8080.h"

#include <stdio.h>
#include <stdlib.h>

#include <sfml/System/Clock.h>

/* Test fuction defs */
// Runs a batch test on the program to check each opcode
void i8080_testProtocol(i8080State* state);
// Preps the state for the next test opcode
void utilTest_prepNext(i8080State* state, uint8_t opcode, uint8_t byte1, uint8_t byte2);
// Preps the stack
void utilTest_prepStack(i8080State* state, uint16_t newSp);