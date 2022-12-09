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

#define TRUE 1
#define FALSE 0
#define PROCESS_BURST_LENGTH 1000
#define PROCESS_ARRAY_VOLUME 8


static Process** processes;
static int process_array_size = 0;
static int active_processes = 0;


// Initialises the kernel and starts the process scheduler to present the UI to the user and start
// the start-up process, and load relevant memory.
void init_kernel() {
    system("cls");

    process_array_size += PROCESS_ARRAY_VOLUME;
    processes = malloc(sizeof( Process* ) * PROCESS_ARRAY_VOLUME);
    for (int  i = 0; i < PROCESS_ARRAY_VOLUME; i++) {
        processes[i] = NULL;
    }
    
    
    start_process(0, 1);
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
                memory_loc: memory_loc
            };

            processes[i] = malloc(sizeof(Process));
            *processes[i] = new_process;

            break;
        }
    }

    active_processes++;
}


// Prints out all the active processes
void print_processes() {
    printf("\nID\tPermit\tAddr\n");
    for (int i = 0; i < active_processes; i++) {
        printf(
            "%d\t%d\t0x%08lX\n", 
            processes[i]->id, 
            processes[i]->permissions, 
            processes[i]->memory_loc
        );
    }
    printf("\n");
}
