#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "microkernel.h"
#include "../internal_memory.h"


MMUEntry* MMU = NULL;


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
    for (int i = 0; i < pages_to_print; i++) {
        if (MMU[i].allocated == 0)
            printf("0x%04X: NOT ALLOCATED\n", i);
        else
            printf(
                "0x%04lX: 0x%08lX\t(%c)\n", MMU[i].logical_start_addr, 
                MMU[i].physical_start_addr, MMU[i].type
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
        ) return MMU->physical_start_addr + (logical_addr & 0x0FFF);
    }
    
    return -1;
}


/**
 * @brief Creates a new Process type to be run on the processor.
 * 
 * @param id The id of the new process
 * @param binary_buffer The binary code the process runs
 * @return New Process struct
 */
Process new_process(uint16_t id, uint16_t* binary_buffer, long prog_len, RAM* ram) {
    Process process;
    process.id = id;
    process.pc = 0;
    process.max_addr = 0;

    char section = CODE_PAGE;
    uint32_t address = 0;
    MMUEntry* page = request_new_page(&process, section);
    for (long i = 0; i < prog_len; i++) {
        // check if we have moved into the data section
        if (binary_buffer[i] == 0x6164 && binary_buffer[i+1] == 0x6174 && binary_buffer[i+2] == 0x003A) {
            section = DATA_PAGE;
            page = request_new_page(&process, section);
            address += address % PAGE_SIZE;

            // skip the data label bytes
            i += 1;
            continue;
        }

        if (binary_buffer[i] == 0x6574 && binary_buffer[i+1] == 0x7478 && binary_buffer[i+2] == 0x003A) {
            section = TEXT_PAGE;
            page = request_new_page(&process, section);
            address += address % PAGE_SIZE;

            // skip the text label bytes
            i += 1;
            continue;
        }

        add_to_ram(ram, get_physical_from_logical_addr(id, address), binary_buffer[i]);
        if (address >= process.max_addr)
            page = request_new_page(&process, section);
        
        address++;
    }

    // every process defaults to 2 pages for heap and 2 pages for stack
    request_new_page(&process, HEAP_PAGE);
    request_new_page(&process, HEAP_PAGE);
    request_new_page(&process, STACK_PAGE);
    request_new_page(&process, STACK_PAGE);

    return process;
}
