#ifndef INTERRUPT_HANDLER
#define INTERRUPT_HANDLER

#include <stdint.h>
#include "../registers.h"
#include "../internal_memory.h"
#include "microkernel.h"


void handle_interrupt_code(unsigned short code, Register* registers, RAM* ram, Process* process, FILE* hd_img);
void print_open_files();

#endif
