#ifndef MICROKERNEL
#define MICROKERNEL

#define PAGE_SIZE 4096
#define NUM_PAGES 0x10000 // 256Mb total data in 65536 pages
#define BURST_LEN 1024
#define CODE_PAGE 'c' 
#define TEXT_PAGE 't'
#define DATA_PAGE 'd'
#define HEAP_PAGE 'h' 
#define FREE_PAGE 'f' 
#define STACK_PAGE 's'

#include <stdint.h>
#include "../internal_memory.h"
#include "../registers.h"
#include "../ALU.h"


/**
 * @brief Represents a block of memory on the heap allocated via the buddy system.
 */
typedef struct HeapBlock {
    uint32_t start_addr;
    uint32_t size;
    uint8_t status;
    struct HeapBlock* right_child;
    struct HeapBlock* left_child;
} HeapBlock;


/**
 * @brief Used to keep track of all the pages currently created and how they map from logical to 
 * physical memory, and to track available IDs and locations in physical memory.
 */
typedef struct MMUEntry {
    uint8_t process_id;
    char type;
    char allocated;
    uint32_t logical_start_addr;
    uint32_t physical_start_addr;
} MMUEntry;


/**
 * @brief Represents a single process being run on the processor
 */
typedef struct Process {
    uint8_t id;
    uint32_t pc;
    uint32_t max_addr; // the highest valid address
    struct ALU_flags flags;
} Process;


void init_processes();
void init_MMU();
void print_MMU(int num_pages);
Process* new_process(uint8_t id, uint16_t* binary_buffer, long prog_len, RAM* ram);
MMUEntry* request_new_page(Process* process, char type);
void execute_scheduled_processes(RAM* ram, Register* registers);


#endif