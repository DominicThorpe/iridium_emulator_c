/*
This operating system uses the microkernel architecture. This part of the OS, the kernel, shall be
able to schedule processes using round-robin scheduling, and manage memory appropriately using the
friends-system. 

It shall also be able to invoke the systems the OS and Application Layer have access to, which fill
in many functionalities.
*/
#include <stdlib.h>
#include <stdio.h>
#include "microkernel.h"
#include "../control_unit.h"
#include "../registers.h"
#include "../internal_memory.h"

#define TRUE 1
#define FALSE 0
#define PROCESS_BURST_LENGTH 1000
#define PROCESS_ARRAY_VOLUME 8


static Process** processes;
static int process_array_size = 0;
static int active_processes = 0;
static long long internal_count = 0; // total number of clock ticks so far


// Initialises the kernel and starts the process scheduler to present the UI to the user and start
// the start-up process, and load relevant memory.
void init_kernel() {
    // system("cls");

    process_array_size += PROCESS_ARRAY_VOLUME;
    processes = malloc(sizeof( Process* ) * PROCESS_ARRAY_VOLUME);
    for (int  i = 0; i < PROCESS_ARRAY_VOLUME; i++) {
        processes[i] = NULL;
    }
    
    
    start_process(0, 0);
}


// Starts a new process by allocating it an ID and adding it to the array of active processes. Expands
// active processes array if necessary.
int start_process(long memory_loc, int permissions) {
    // expand size of processes array if it is not big enough
    if (active_processes >= process_array_size) {
        processes = realloc(processes, sizeof(Process) * (active_processes  + PROCESS_ARRAY_VOLUME));
        if (processes == NULL) {
            printf("Failed to realloc processes memory length!\n");
            exit(-100);
        }
    }

    for (int i = 0; i < process_array_size; i++) {
        if (processes[i] == NULL) {
            Process new_process = {
                id: i,
                permissions: permissions,
                memory_loc: memory_loc,
                current_pc: 0,
            };

            processes[i] = malloc(sizeof(Process));
            *processes[i] = new_process;

            active_processes++;
            return i;
        }
    }
}


// Prints out all the active processes
void print_processes() {
    printf("\nID\tPermit\tAddr\t\tPC\n");
    for (int i = 0; i < active_processes; i++) {
        printf(
            "%d\t%d\t0x%08lX\n", 
            processes[i]->id, 
            processes[i]->permissions, 
            processes[i]->memory_loc,
            processes[i]->current_pc
        );
    }
    printf("\n");
}


// Kills the process with the given id and makes the id and place in the array available
void kill_process(int id) {
    for (int i = 0; i < active_processes; i++) {
        if (processes[i] != NULL && processes[i]->id == id) {
            free(processes[i]);
            processes[i] = NULL;
            active_processes--;
        }
    }
}


/*
Executes a program until the internal clock tick count reaches cutoff, at which point it yields
control back to the process scheduler. If a HALT instruction (0xFFFF) is encountered, control is
yielded.

The return value is the current value in the program counter, or -1 if the process has finished and
can be killed.
*/
long execute_program(RAM* ram, Register* register_file, long start_addr, int cutoff) {
    // set the PC to the starting point, or where the process last left off
    long pc_count = start_addr;
    Register start_addr_reg, page_num;
    start_addr_reg.word_32 = start_addr;
    page_num.word_16 = start_addr / 0x10000; // divide start addr by page size to get page number
    update_register(15, start_addr_reg, register_file);
    update_register(11, page_num, register_file);

    // go through the instructions one by one
    // TODO: break when the cutoff has been reached to move on to the next process
    Register new_count;
    short command;
    while (TRUE) {
        pc_count = get_register(15, register_file).word_32;
        command = get_from_ram(ram, pc_count);
        
        if (command == 0x0000 || command == 0xFFFF)
            break;
        
        execute_command(command, ram, register_file, page_num.word_16);
        new_count.word_32 = get_register(15, register_file).word_32 + 1;
        update_register(15, new_count, register_file);

        internal_count++;
    }

    return pc_count;
}
