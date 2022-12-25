#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "registers.h"
#include "internal_memory.h"
#include "control_unit.h"
#include "os/microkernel.h"

#define DATA_SECTION_OFFSET 0x00C00000
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


void execute_program(RAM* ram, Register* register_file) {
    int pc_count;
    Register new_count;
    short command;
    while (TRUE) {
        pc_count = get_register(15, register_file).word_32;
        command = get_from_ram(ram, pc_count);
        
        if (command == 0x0000 || command == 0xFFFF)
            break;
        
        execute_command(command, ram, register_file);
        new_count.word_32 = get_register(15, register_file).word_32 + 1;
        update_register(15, new_count, register_file);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename>\n");
        exit(-1);
    }

    Register* register_file = init_registers();
    RAM* ram = init_RAM(1024);

    // read program data into RAM
    long prog_len;
    uint16_t* commands = read_commands(argv[1], &prog_len);
    
    // execute_program(ram, register_file);
    // print_registers(register_file);
    init_MMU();
    Process process = new_process(0, commands, prog_len, ram);
    print_RAM(ram);
    
    free(register_file);
    free(commands);
    
    return 0;
}
