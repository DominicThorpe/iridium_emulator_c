#ifndef CONTROL_UNIT
#define CONTROL_UNIT

#include <stdio.h>
#include "internal_memory.h"
#include "registers.h"
#include "os/microkernel.h"

void execute_command(short command, RAM* ram, Register* registers, Process* process, FILE* hd_img);

#endif

