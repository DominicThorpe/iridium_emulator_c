#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "registers.h"
#include "internal_memory.h"
#include "control_unit.h"

#define TRUE 1
#define FALSE 0


/**
 * @brief Reads bytes from the specified file and outputs those bytes as a pointer to an array
 * 
 * @param filename Name of the file to read the program from
 * @param prog_len Pointer in which the length of the file is placed
 * @return uint16_t* Array of the commands the file contains
 */
uint16_t* read_commands(char* filename, long* prog_len) {
    FILE *fileptr;
    uint16_t *buffer;
    long filelen;

    fileptr = fopen(filename, "r+b");

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

    uint16_t* register_file = init_registers();
    uint8_t* RAM = init_RAM();

    // read program data into RAM
    long prog_len_a;
    uint16_t* commands = read_commands(argv[1], &prog_len_a);

    uint16_t pc = 0;
    while (commands[pc] != 0x0000) {
        /* code */
    }
    
    
    print_registers(register_file);

    return 0;
}
