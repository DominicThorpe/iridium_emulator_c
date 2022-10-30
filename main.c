#include <stdlib.h>
#include <stdio.h>
#include "registers.h"
#include "ram.h"
#include "control_unit.h"


// Reads bytes from the specified file and outputs those bytes as a pointer to an array
char* read_commands(char* filename, long* prog_len) {
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

    *prog_len = filelen;
    return buffer;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename>\n");
        exit(-1);
    }

    Register* register_file = init_registers();
    RAM* ram = init_RAM(1024);

    // read program instr data into RAM
    long prog_len;
    char* commands = read_commands(argv[1], &prog_len);
    for (long i = 0; i < prog_len; i += 2) {
        short command = (commands[i] << 8) | commands[i+1];

        // found the start of the data section
        if (commands[i] == 'd' && commands[i+1] == 'a' && commands[i+2] == 't' && commands[i+3] == 'a')
            break;
        
        add_to_ram(ram, i / 2, command);
    }

    int pc_count;
    Register new_count;
    short command;
    while (1) {
        pc_count = get_register(15, register_file).word_32;
        command = get_from_ram(ram, pc_count);
        
        if (command == 0x0000 || command == 0xFFFF)
            break;
        
        printf("Command: %04hX\n", command);
        execute_command(command, ram, register_file);
        new_count.word_32 = get_register(15, register_file).word_32 + 1;
        update_register(15, new_count, register_file);
    }

    print_registers(register_file);
    
    free(register_file);
    free(commands);

    printf("Done\n");
    
    return 0;
}
