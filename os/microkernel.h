#ifndef MICROKERNEL
#define MICROKERNEL

#define PAGE_SIZE 4096
#define NUM_PAGES 0x1000 // 256Mb total data in 65536 pages
#define HEAP_SIZE 0x100000 // 1Mb heap per process
#define BURST_LEN 1024
#define CODE_PAGE 'c' 
#define TEXT_PAGE 't'
#define DATA_PAGE 'd'
#define HEAP_PAGE 'h' 
#define FREE_PAGE 'f' 
#define STACK_PAGE 's'

#include <stdint.h>
#include <stdio.h>
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
    uint8_t started; // 0 if process not ever run, otherwise 1
    uint32_t max_addr; // the highest valid address
    HeapBlock* heap_root;
    struct ALU_flags flags;
} Process;


void init_processes();
void init_MMU();

Process* new_process(uint8_t id, uint16_t* binary_buffer, long prog_len, RAM* ram);
void execute_scheduled_processes(RAM* ram, Register* registers, FILE* hd_img);

MMUEntry* request_new_page(Process* process, char type);
uint32_t allocate_memory(HeapBlock* root, uint32_t size);
void free_memory(HeapBlock* root, long address);
void change_heap_size(int32_t offset, Process* process);

void print_malloc_tree(HeapBlock root, int depth);
void print_MMU(int num_pages);
void print_processes();


#endif
