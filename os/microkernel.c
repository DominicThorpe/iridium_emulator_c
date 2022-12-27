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
    printf("Logical\tPhysical\tType\tProcess\n");
    for (int i = 0; i < pages_to_print; i++) {
        if (MMU[i].allocated == 0)
            printf("0x%04X: NOT ALLOCATED\n", i);
        else
            printf(
                "0x%04lX:\t0x%08lX\t(%c)\t%d\n", MMU[i].logical_start_addr, 
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

    // every process defaults to 2 pages for heap and 2 pages for stack
    request_new_page(process, HEAP_PAGE);
    request_new_page(process, HEAP_PAGE);
    request_new_page(process, STACK_PAGE);
    request_new_page(process, STACK_PAGE);

    num_active_processes++;
    processes[id] = process;

    return processes[id];
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

        execute_command(command, ram, registers);
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
