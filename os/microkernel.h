#ifndef MICROKERNEL
#define MICROKERNEL

#include "../registers.h"
#include "../internal_memory.h"


typedef struct HeapBlock {
    long start_addr;
    int size;
    int status; // 0 = full, 1 = available, 2 = split
    struct HeapBlock* right_child;
    struct HeapBlock* left_child;
} HeapBlock;

typedef struct ProcessStruct {
    int id;
    int permissions;
    long memory_loc;
    long current_pc;
    HeapBlock* heap_root; // maybe convert this to linked-list?
} Process;

void init_kernel();
int start_process(long memory_loc, int permissions);
void kill_process(int id);
void print_processes();
long execute_process(RAM* ram, Register* register_file, long start_addr, int cutoff);
void run_active_processes(RAM* ram, Register* registers);
long allocate_memory(HeapBlock* root, long size);

#endif
