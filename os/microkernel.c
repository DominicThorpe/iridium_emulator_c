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
#define PROCESS_BURST_LENGTH 10
#define PROCESS_ARRAY_VOLUME 8
#define PAGE_SIZE 0x10000


static Process** processes;
static int process_array_size = 0;
static int active_processes = 0;
static long internal_count = 0; // total number of clock ticks so far


/*
Creates a new HeapBlock with the given parameters, a status of 1, and NULL left and right 
children.

Status: 0 = memory allocated, 1 = memory free, 2 = memory subdivided
*/
HeapBlock* new_heap_block(long start_addr, long size) {
    HeapBlock* block = malloc(sizeof(HeapBlock));
    block->start_addr = start_addr;
    block->size = size;
    block->status = 1;
    block->right_child = NULL;
    block->left_child = NULL;

    return block;
}


/* 
Creates a new HeapBlock of at the most appropriate position in memory it can find by
finding the first fit it can in RAM.
TODO: Allow for the allocation of multiple pages in memory, maybe use a lookup table?
*/
HeapBlock* new_heap_root(long size) {
    long start_addr = 0x00C00000;
    for (int i = 0; i < process_array_size; i++) {
        if (processes[i] == NULL)
            continue;
        else if (processes[i]->heap_root->start_addr == start_addr)
            start_addr += PAGE_SIZE;
    }    

    HeapBlock* new_block = new_heap_block(start_addr, PAGE_SIZE);
    return new_block;
}


/*
Recursive function to start at the root of a process's heap memory and go through it, subdividing
as necessary by adding new blocks to the binary tree, until it finds an appropriately sized block
to allocate - uses the friend's system.

Returns the address of the allocated block if one is found, and -1 if one could not be found.
*/
long allocate_memory(HeapBlock* root, long size) {
    // if memory is taken, do nothing
    if (root == NULL || size > root->size || root->status == 0) 
        return -1;
    
    // if root is subdivided, can continue to go down the tree but cannot allocate
    else if (root->status == 2) { 
        long alloc_status = allocate_memory(root->left_child, size);
        if (alloc_status >= 0)
            return alloc_status;
        
        alloc_status = allocate_memory(root->right_child, size);
        if (alloc_status >= 0)
            return alloc_status;
    }
    
    // if memory is free, but too large, create new node and go further down the tree
    else if (root->size / 2 >= size) {
        root->status = 2;
        root->left_child = new_heap_block(root->start_addr, root->size / 2);
        root->right_child = new_heap_block(root->start_addr + root->size / 2, root->size / 2);

        long child_alloc = allocate_memory(root->left_child, size);
        if (child_alloc >= 0)
            return child_alloc;

        child_alloc = allocate_memory(root->right_child, size);
        if (child_alloc >= 0)
            return child_alloc;
        
        return -1;
    }

    // found appropriate block to allocate
    else if (root->size >= size && root->status == 1) { // allocate this
        root->status = 0;
        return root->start_addr;
    } 
    
    return -1;
}


// Frees the memory in the heap at the given memory and coalesces it if its friend is also free
// TODO: coalesce memory
void free_memory(HeapBlock* root, long address) {
    // Check if either child is a match and free and NULL it if it is
    if (root->left_child != NULL && root->left_child->start_addr == address && root->left_child->status == 0) {
        free(root->left_child);
        root->left_child = NULL;
    } else if (root->right_child != NULL && root->right_child->start_addr == address && root->right_child->status == 0) {
        free(root->right_child);
        root->right_child = NULL;
    }

    // If not, find the most appropriate child and recur into it
    if (root->left_child != NULL && address >= root->left_child->start_addr && address < root->left_child->start_addr + root->left_child->size)
        free_memory(root->left_child, address);
    else if (root->right_child != NULL && address >= root->right_child->start_addr && address < root->right_child->start_addr + root->right_child->size)
        free_memory(root->right_child, address);
    
    // Coalesce friends if they are both free or NULL
    if ((root->left_child == NULL || root->left_child->status == 1) && (root->right_child == NULL || root->right_child->status == 1)) {
        free(root->left_child);
        free(root->right_child);
        root->left_child = NULL;
        root->right_child = NULL;
        root->status = 1;
    }
}


// Pretty-prints the current memory allocation tree
void print_malloc_tree(HeapBlock root, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    printf("-%08lX: %04X - %d, (%p, %p)\n", root.start_addr, root.size, root.status, root.left_child, root.right_child);
    if (root.left_child != NULL)
        print_malloc_tree( *root.left_child, depth + 1);
    if (root.right_child != NULL)
        print_malloc_tree( *root.right_child, depth + 1);
}


// Initialises the kernel and starts the process scheduler to present the UI to the user and start
// the start-up process, and load relevant memory.
void init_kernel() {
    system("cls");

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
                current_pc: memory_loc,
                heap_root: new_heap_root(PAGE_SIZE)
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
    for (int i = 0; i < process_array_size; i++) {
        if (processes[i] == NULL)
            continue;

        printf(
            "%d\t%d\t0x%08lX\t%04lX\t%08lX\n", 
            processes[i]->id, 
            processes[i]->permissions, 
            processes[i]->memory_loc,
            processes[i]->current_pc,
            processes[i]->heap_root->start_addr
        );
    }
    printf("\n");
}


// Kills the process with the given id and makes the id and place in the array available
void kill_process(int id) {
    for (int i = 0; i < process_array_size; i++) {
        if (processes[i] != NULL && processes[i]->id == id) {
            free(processes[i]);
            active_processes--;
            break;
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
long execute_process(RAM* ram, Register* register_file, long start_addr, int cutoff) {
    // set the PC to the starting point, or where the process last left off
    long pc_count = start_addr;
    Register start_addr_reg, page_num;
    start_addr_reg.word_32 = start_addr;
    page_num.word_16 = start_addr / PAGE_SIZE; // divide start addr by page size to get page number
    update_register(15, start_addr_reg, register_file);
    update_register(11, page_num, register_file);

    // go through the instructions one by one until either program end or cutoff is reached
    Register new_count;
    short command;
    while (internal_count < cutoff) {
        pc_count = get_register(15, register_file).word_32;
        command = get_from_ram(ram, pc_count);
        
        if (command == 0x0000 || command == 0xFFFF)
            return -1;
        
        execute_command(command, ram, register_file, page_num.word_16);
        new_count.word_32 = get_register(15, register_file).word_32 + 1;
        update_register(15, new_count, register_file);

        internal_count++;
    }

    return pc_count;
}


/*
Takes the current state of the registers and pushes it to the stack. Location subject to change when
memory management becomes focus of development - currently end of the process's page with $zero going
first and $pc going last.
*/
void push_registers(RAM* ram, Register* registers, long page_addr) {
    unsigned long end_addr = page_addr + PAGE_SIZE - 1;
    add_to_ram(ram, end_addr, 0);
    
    for (int i = 1; i < 12; i++) {
        add_to_ram(ram, end_addr - i, get_register(i, registers).word_16);
    } for (int i = 12; i < 16; i++) {
        add_to_ram(ram, end_addr - (i * 2), (get_register(i, registers).word_32 >> 16) & 0xFFFF);
        add_to_ram(ram, end_addr - (i * 2) + 1, get_register(i, registers).word_32 & 0xFFFF);
    }
}


/*
Goes to the end of the specified page and loads the values there into the registers as specified by
the push_registers function.
*/
void pop_registers(RAM* ram, Register* registers, long page_addr) {
    unsigned long end_addr = page_addr + PAGE_SIZE - 1;
    Register reg;
    reg.word_32 = 0;

    registers[0] = reg;
    for (int i = 1; i < 12; i++) {
        reg.word_16 = get_from_ram(ram, end_addr - i);
        update_register(i, reg, registers);
    } for (int i = 12; i < 16; i++) {
        reg.word_32 = (get_from_ram(ram, end_addr - (i * 2)) << 16) | (get_from_ram(ram, end_addr - (i * 2) + 1));
        update_register(i, reg, registers);
    }
}


/*
Uses round-robin scheduling to run all the active processes for a certain number of clock ticks
before yielding to the next porcess. This is repeated for all processes until all the processes
with permissions > 0 die (i.e. active processes > 1).
*/
void run_active_processes(RAM* ram, Register* registers) {
    while (active_processes > 1) {
        for (int i = 0; i < process_array_size; i++) {
            if (processes[i] == NULL)
                continue;

            pop_registers(ram, registers, processes[i]->memory_loc);
            int result = execute_process(
                ram, registers, 
                processes[i]->current_pc,
                internal_count + PROCESS_BURST_LENGTH
            );

            if (result < 0 && processes[i]->permissions != 0) {
                kill_process(processes[i]->id);
                processes[i] = NULL;
                continue;
            }

            processes[i]->current_pc = result;
            push_registers(ram, registers, processes[i]->memory_loc);
        }
    }
}
