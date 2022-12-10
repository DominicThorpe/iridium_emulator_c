#ifndef MICROKERNEL
#define MICROKERNEL

#include "../registers.h"
#include "../internal_memory.h"

typedef struct ProcessStruct {
    int id;
    int permissions;
    long memory_loc;
    long current_pc;
} Process;

void init_kernel();
int start_process(long memory_loc, int permissions);
void kill_process(int id);
void print_processes();
long execute_program(RAM* ram, Register* register_file, long start_addr, int cutoff);

#endif
