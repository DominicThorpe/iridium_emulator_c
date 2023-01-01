#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "microkernel.h"
#include "../internal_memory.h"
#include "../registers.h"
#include "../control_unit.h"
#include "../ALU.h"


MMUEntry* MMU = NULL;
Process** processes = NULL;
uint8_t num_active_processes = 0;
const uint8_t max_processes = 255;


/**
 * @brief Initialise all the processes in the array to NULL
 */
void init_processes() {
    processes = malloc(sizeof(Process*) * 255);
    for (int i = 0; i < max_processes; i++) {
        processes[i] = NULL;
    }
}


/**
 * @brief Initialises the memory management unit (MMU), which is an inverted page table.
 */
void init_MMU() {
    MMU = malloc(sizeof(MMUEntry) * NUM_PAGES);
    for (int i = 0; i < NUM_PAGES; i++) {
        MMUEntry new_node;
        new_node.allocated = 0;
        new_node.physical_start_addr = i * 4096;
        new_node.type = FREE_PAGE;
        MMU[i] = new_node;
    }
}


/**
 * @brief Debug to check the MMU is working correctly.
 */
void print_MMU(int num_pages) {
    int pages_to_print = num_pages > 0 ? num_pages : NUM_PAGES;
    printf("Logical\t\tPhysical\tType\tProcess\n");
    for (int i = 0; i < pages_to_print; i++) {
        if (MMU[i].allocated == 0)
            printf("0x%04X: NOT ALLOCATED\n", i);
        else
            printf(
                "0x%08lX:\t0x%08lX\t(%c)\t%d\n", MMU[i].logical_start_addr, 
                MMU[i].physical_start_addr, MMU[i].type, MMU[i].process_id
            );
    }
}


/**
 * @brief Takes the ID of a process and assigns a new page to it, if there is one, and returns a
 * pointer to the page, or NULL if one cannot be found. 
 * 
 * @param process_id The ID of the process requesting a page
 * @param type The type of the new page
 * @param logical_addr The logical address of the first byte of the page
 * @return MMUEntry* if a page is found, NULL if not
 */
MMUEntry* request_new_page(Process* process, char type) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (MMU[i].allocated == 0) {
            MMU[i].allocated = 1;
            MMU[i].process_id = process->id;
            MMU[i].type = type;

            MMU[i].logical_start_addr = process->max_addr;
            process->max_addr += PAGE_SIZE;

            return &MMU[i];
        }
    }

    return NULL;
}


/**
 * @brief Get the physical address of a byte from its logical address and process id
 * 
 * @param process_id The id of the process the page belongs to
 * @param logical_addr The logical address of the byte
 * @return The physical address of the byte
 */
uint32_t get_physical_from_logical_addr(uint16_t process_id, uint32_t logical_addr) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (
            MMU[i].process_id == process_id && 
            MMU[i].logical_start_addr <= logical_addr && 
            MMU[i].logical_start_addr + PAGE_SIZE > logical_addr
        ) return MMU[i].physical_start_addr + (logical_addr & 0x0FFF);
    }
    
    return -1;
}


/**
 * @brief Creates a new HeapBlock with the given parameters, a status of 1, and NULL left and right 
 * children.
 * 
 * @param start_addr The address of the first byte of the block
 * @param size The size in bytes of the block
 * @return A pointer to the new block
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


/**
 * @brief Creates a new Process type to be run on the processor.
 * 
 * @param id The id of the new process
 * @param binary_buffer The binary code the process runs
 * @return New Process struct
 */
Process* new_process(uint8_t id, uint16_t* binary_buffer, long prog_len, RAM* ram) {
    if (num_active_processes >= 255)
        return NULL;

    Process* process = malloc(sizeof(Process));
    process->id = id;
    process->pc = 0;
    process->max_addr = 0;
    process->flags.carry = 0;
    process->flags.negative = 0;
    process->flags.zero = 0;

    char section = CODE_PAGE;
    uint32_t address = 0;
    MMUEntry* page = request_new_page(process, section);
    for (long i = 0; i < prog_len; i++) {
        // check if we have moved into the data section
        if (binary_buffer[i] == 0x6164 && binary_buffer[i+1] == 0x6174 && binary_buffer[i+2] == 0x003A) {
            section = DATA_PAGE;
            page = request_new_page(process, section);
            address += PAGE_SIZE - (address % PAGE_SIZE);

            // skip the data label bytes
            i += 2;
            continue;
        }

        if (binary_buffer[i] == 0x6574 && binary_buffer[i+1] == 0x7478 && binary_buffer[i+2] == 0x003A) {
            section = TEXT_PAGE;
            page = request_new_page(process, section);
            address += PAGE_SIZE - (address % PAGE_SIZE);

            // skip the text label bytes
            i += 2;
            continue;
        }

        add_to_ram(ram, get_physical_from_logical_addr(id, address), binary_buffer[i]);
        if (address >= process->max_addr)
            page = request_new_page(process, section);
        
        address++;
    }

    // Assign the default number of pages for heap and stack
    process->heap_root = NULL;
    for (int i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
        MMUEntry* this = request_new_page(process, HEAP_PAGE);
        if (process->heap_root == NULL)
            process->heap_root = new_heap_block(this->logical_start_addr, HEAP_SIZE);
    }

    for (int i = 0; i < HEAP_SIZE / PAGE_SIZE; i++) {
        request_new_page(process, STACK_PAGE);
    }

    num_active_processes++;
    processes[id] = process;

    return processes[id];
}


void print_processes() {
    printf("ID\tPC\t\tMax Addr\tHeap Start\n");
    for (int i = 0; i < max_processes; i++) {
        if (processes[i] == NULL)
            continue;

        printf("%d\t0x%08X\t0x%08X\t0x%08X\n", 
            processes[i]->id, processes[i]->pc, processes[i]->max_addr, 
            processes[i]->heap_root->start_addr
        );
    }
}


/**
 * @brief Takes a process, RAM, and registers, then executes the process for a number of instructions
 * equal to the burst length, round-robin style.
 * 
 * @param ram The system RAM
 * @param register The CPU registers
 * @param process The process being executed
 * @param burst_len The number of instructions to execute
 * @return The value in the program counter at the end of the burst, or -1 if the process completes
 */
uint32_t execute_process_burst(RAM* ram, Register* registers, Process* process, uint32_t burst_len) {
    Register new_pc;
    new_pc.word_32 = process->pc;
    update_register(15, new_pc, registers);
    alu_flags.carry = process->flags.carry;
    alu_flags.negative = process->flags.negative;
    alu_flags.zero = process->flags.zero;

    uint16_t command;
    Register new_count;
    uint32_t address;
    uint32_t instrs_executed = 0;
    while (instrs_executed < burst_len) {
        address = get_physical_from_logical_addr(process->id, get_register(15, registers).word_32);
        command = get_from_ram(ram, address);
        if (command == 0x0000 || command == 0xFFFF)
            return -1;

        execute_command(command, ram, registers, process);
        new_count.word_32 = get_register(15, registers).word_32 + 1;
        update_register(15, new_count, registers);

        instrs_executed++;
    }

    process->pc = get_register(15, registers).word_32;
    process->flags.carry = alu_flags.carry;
    process->flags.negative = alu_flags.negative;
    process->flags.zero = alu_flags.zero;

    return get_register(15, registers).word_32;
}


/**
 * @brief Runs all the currently active processes using round-robin scheduling.
 */
void execute_scheduled_processes(RAM* ram, Register* registers) {
    while (num_active_processes > 0) {
        for (int i = 0; i < max_processes; i++) {
            if (processes[i] == NULL)
                continue;

            uint32_t result = execute_process_burst(ram, registers, processes[i], BURST_LEN);
            if (result == -1) {
                free(processes[i]);
                processes[i] = NULL;
                num_active_processes--;
            }
        }
    }
}


/**
 * @brief Recursive function to start at the root of a process's heap memory and go through it, 
 * subdividing as necessary by adding new blocks to the binary tree, until it finds an appropriately 
 * sized block to allocate - uses the friend's system.
 * 
 * @param root Pointer to the root block in the heap allocation tree
 * @param size The size of the memory to allocate
 * @return Returns the address of the allocated block if one is found, and -1 if one could not be found. 
 */
uint32_t allocate_memory(HeapBlock* root, uint32_t size) {
    if (size == 0)
        return -1;
        
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
    else if (root->size / 2 >= size && root->size != 0) {
        root->status = 2;
        root->left_child = new_heap_block(root->start_addr, root->size / 2);
        root->right_child = new_heap_block(root->start_addr + root->size / 2, root->size / 2);
        if (root->left_child == NULL || root->right_child == NULL)
            printf("Could not find memory for heap tree child node!\n");

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


/**
 * @brief Frees the memory in the heap at the given memory and coalesces it if its friend is also free
 * 
 * @param root Pointer to the root block in the heap allocation tree
 * @param address The address of the first byte of the block to be freed
 */
void free_memory(HeapBlock* root, long address) {
    // Check if either child is a match and free and NULL it if it is
    if (
        root->left_child != NULL && 
        root->left_child->start_addr == address && 
        root->left_child->status == 0
    ) {
        free(root->left_child);
        root->left_child = NULL;
    } else if (
        root->right_child != NULL && 
        root->right_child->start_addr == address && 
        root->right_child->status == 0
    ) {
        free(root->right_child);
        root->right_child = NULL;
    }

    // If not, find the most appropriate child and recur into it
    if (
        root->left_child != NULL && 
        address >= root->left_child->start_addr && 
        address < root->left_child->start_addr + root->left_child->size
    ) free_memory(root->left_child, address);
    else if (
        root->right_child != NULL && 
        address >= root->right_child->start_addr && 
        address < root->right_child->start_addr + root->right_child->size
    ) free_memory(root->right_child, address);
    
    // Coalesce friends if they are both free or NULL
    if (
        (root->left_child == NULL || root->left_child->status == 1) && 
        (root->right_child == NULL || root->right_child->status == 1)
    ) {
        free(root->left_child);
        free(root->right_child);
        root->left_child = NULL;
        root->right_child = NULL;
        root->status = 1;
    }
}


/**
 * @brief Recursively pretty-prints the current memory allocation tree
 * 
 * @param root Pointer to the root block in the heap allocation tree
 * @param depth The current level of the node the tree is currently on
 */
void print_malloc_tree(HeapBlock root, int depth) {
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }
    
    printf("-%08lX: %04X - %d, (%p, %p)\n", 
            root.start_addr, root.size, root.status, root.left_child, root.right_child);
    if (root.left_child != NULL)
        print_malloc_tree( *root.left_child, depth + 1);
    if (root.right_child != NULL)
        print_malloc_tree( *root.right_child, depth + 1);
}


/**
 * @brief Adjusts the breakpoint between the process stack and heap by the offset. Note that increasing 
 * the heap shrinks the stack, and vice versa.
 * 
 * @param offset The number of pages to add to the heap; negative value shrinks the heap.
 * @param process The process to adjust
 */
void change_heap_size(int32_t offset, Process* process) {
    if (offset == 0)
        return;
    
    MMUEntry* lowest_stack = NULL;
    MMUEntry* highest_heap = NULL;
    for (int i = 0; i < NUM_PAGES; i++) {
        if (MMU[i].process_id == process->id) {
            if (lowest_stack == NULL && MMU[i].type == STACK_PAGE)
                lowest_stack = &MMU[i];
            else if (highest_heap == NULL && MMU[i].type == HEAP_PAGE)
                highest_heap = &MMU[i];
            else if (MMU[i].type == STACK_PAGE && MMU[i].logical_start_addr < lowest_stack->logical_start_addr)
                lowest_stack = &MMU[i];
            else if (MMU[i].type == HEAP_PAGE && MMU[i].logical_start_addr > highest_heap->logical_start_addr)
                highest_heap = &MMU[i];
        }
    }

    if (lowest_stack == NULL && highest_heap == NULL) {
        printf("Failed to adjust stack size\n");
        exit(50);
    }

    if (offset > 0) {
        lowest_stack->type = HEAP_PAGE;
        offset--;
    } else {
        highest_heap->type = STACK_PAGE;
        offset++;
    }
    
    change_heap_size(offset, process);
}
