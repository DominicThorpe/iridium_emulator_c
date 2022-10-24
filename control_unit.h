#ifndef CONTROL_UNIT
#define CONTROL_UNIT

#include "ram.h"
#include "registers.h"

void execute_command(short command, RAM* ram, Register* registers);

#endif

