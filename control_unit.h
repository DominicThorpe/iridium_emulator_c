#ifndef CONTROL_UNIT
#define CONTROL_UNIT

#include <stdio.h>
#include "internal_memory.h"
#include "registers.h"

void execute_command(uint16_t command, uint8_t* ram, uint16_t* registers, uint16_t* pc);

#endif

