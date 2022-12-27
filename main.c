#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "registers.h"
#include "internal_memory.h"
#include "control_unit.h"
#include "os/microkernel.h"

#define TRUE 1
#define FALSE 0


// Reads bytes from the specified file and outputs those bytes as a pointer to an array
uint16_t* read_commands(char* filename, long* prog_len) {
    FILE *fileptr;
    uint16_t *buffer;
    long filelen;

    fileptr = fopen(filename, "rb");

    // determine length of file
    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr) / 2;
    rewind(fileptr);

    buffer = malloc(filelen * sizeof(uint16_t));
    fread(buffer, filelen, sizeof(uint16_t), fileptr);
    fclose(fileptr);
    free(fileptr);

    *prog_len = filelen;
    return buffer;
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename>\n");
        exit(-1);
    }

    Register* register_file = init_registers();
    RAM* ram = init_RAM(1024);

    // read program data into RAM
    long prog_len_a, prog_len_b;
    uint16_t* commands_a = read_commands(argv[1], &prog_len_a);
    uint16_t* commands_b = read_commands(argv[2], &prog_len_b);
    
    init_MMU();
    init_processes();
    Process* process_a = new_process(0, commands_a, prog_len_a, ram);
    Process* process_b = new_process(1, commands_b, prog_len_b, ram);
    execute_scheduled_processes(ram, register_file);
    print_registers(register_file);
    
    return 0;
}
