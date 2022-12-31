#ifndef INTERRUPT_HANDLER
#define INTERRUPT_HANDLER
#include "../registers.h"
#include "../internal_memory.h"
#include "microkernel.h"

void handle_interrupt_code(unsigned short code, Register* registers, RAM* ram, Process* process);

#endif
