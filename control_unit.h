#ifndef CONTROL_UNIT
#define CONTROL_UNIT

#include "internal_memory.h"
#include "registers.h"

void execute_command(short command, RAM* ram, Register* registers);

#endif

