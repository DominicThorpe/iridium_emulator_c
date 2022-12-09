#ifndef MICROKERNEL
#define MICROKERNEL

typedef struct ProcessStruct {
    int id;
    int permissions;
    long memory_loc;
} Process;

void init_kernel();
int start_process(long memory_loc, int permissions);
void print_processes();

#endif
