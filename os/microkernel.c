#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "microkernel.h"


MMUNode* MMU_head = NULL;


/**
 * @brief Initialises the memory management unit (MMU), which is represented as a linked-list, to a
 * full list of unassigned pages. 
 */
void init_MMU() {
    MMU_head = malloc(sizeof(MMUNode));
    MMU_head->allocated = 0;
    MMU_head->physical_start_addr = 0;
    MMU_head->type = FREE_PAGE;
    MMU_head->next = NULL;

    MMUNode* head = MMU_head;
    for (int i = 1; i < NUM_PAGES; i++) {
        head->next = malloc(sizeof(MMUNode));
        head->next->allocated = 0;
        head->next->physical_start_addr = i * 4096;
        head->next->type = FREE_PAGE;
        head->next->next = NULL;

        head = head->next;
    }
}


/**
 * @brief Debug to check the MMU is working correctly.
 */
void print_MMU() {
    int index = 0;
    MMUNode* current = MMU_head;
    while (current != NULL) {
        if (current->allocated == 0)
            printf("%04X: NOT ALLOCATED\n", index);
        else
            printf("0x%08lX = 0x%08lX\n", current->logical_start_addr, current->physical_start_addr);
        
        index++;
        current = current->next;
    }
}


/**
 * @brief Creates a new Process type to be run on the processor.
 * 
 * @param id The id of the process
 * @param binary_buffer The binary code the process runs
 * @return Process
 */
Process new_process(uint16_t id, uint16_t* binary_buffer) {
    Process process;
    process.id = id;
    process.pc = 0;
    return process;
}
