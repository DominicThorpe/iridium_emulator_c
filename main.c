#include <stdlib.h>
#include <stdio.h>
#include "registers.h"
#include "internal_memory.h"
#include "control_unit.h"
#include "os/microkernel.h"

#define DATA_SECTION_OFFSET 0x8000
#define TRUE 1
#define FALSE 0


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


void read_program_into_RAM(char* prog_name, RAM* ram, long base_addr) {
    long prog_len;
    char* commands = read_commands(prog_name, &prog_len);
    int data_section = FALSE;
    int index = 0;
    for (long i = 0; i < prog_len; i += 2) {
        short command = ((short)(commands[i] & 0x00FF) << 8) | (short)(commands[i+1] & 0x00FF);

        // found the start of the data section
        if (commands[i] == 'd' && commands[i+1] == 'a' && 
                        commands[i+2] == 't' && commands[i+3] == 'a') {
            data_section = TRUE;
            index = 0;

            // skip the "data:" label bytes 
            i += 3;
            continue;
        }
        
        if (data_section == FALSE)
            add_to_ram(ram, index + base_addr, command);
        else
            add_to_ram(ram, index + base_addr + DATA_SECTION_OFFSET, command);
        
        index++;
    }

    free(commands);
}


int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename1> <filename2> ...\n");
        exit(-1);
    }

    Register* register_file = init_registers();
    RAM* ram = init_RAM(1024);

    read_program_into_RAM(argv[1], ram, 0x10000);
    read_program_into_RAM(argv[2], ram, 0x20000);

    init_kernel(); // start the kernel process
    start_process(0x20000, 2);
    start_process(0x10000, 2);
    run_active_processes(ram, register_file);
    print_registers(register_file);
    // print_RAM(ram);
    
    free(register_file);    
    return 0;
}
