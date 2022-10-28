#include <stdlib.h>
#include <stdio.h>
#include "registers.h"
#include "ram.h"
#include "control_unit.h"


// Reads bytes from the specified file and outputs those bytes as a pointer to an array
char* read_commands(char* filename) {
    FILE *fileptr;
    char *buffer;
    long filelen;

    fileptr = fopen(filename, "rb");

    // determine length of file
    fseek(fileptr, 0, SEEK_END);
    filelen = ftell(fileptr);
    rewind(fileptr);

    buffer = ( char* ) malloc(filelen * sizeof(char));
    fread(buffer, filelen, 1, fileptr);
    fclose(fileptr);
    free(fileptr);

    return buffer;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename>\n");
        exit(-1);
    }

    Register* register_file = init_registers();
    RAM* ram = init_RAM(1024);

    char* commands = read_commands(argv[1]);
    execute_command(0xD110, ram, register_file);
    execute_command(0xD230, ram, register_file);
    execute_command(0x1312, ram, register_file);
    print_registers(register_file);
    
    free(register_file);
    free(commands);
    
    return 0;
}
