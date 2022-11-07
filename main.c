#include <stdlib.h>
#include <stdio.h>
#include "registers.h"
#include "internal_memory.h"
#include "control_unit.h"
#include "os/file_handler.h"

#define DATA_SECTION_OFFSET 0x00C00000
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
    char* commands = read_commands(argv[1], &prog_len);
    int data_section = FALSE;
    int index = 0;
    for (long i = 0; i < prog_len; i += 2) {
        short command = ((short)(commands[i] & 0x00FF) << 8) | (short)(commands[i+1] & 0x00FF);

        // found the start of the data section
        if (commands[i] == 'd' && commands[i+1] == 'a' && commands[i+2] == 't' && commands[i+3] == 'a') {
            data_section = TRUE;
            index = 0;

            // skip the "data:" label bytes 
            i += 3;
            continue;
        }
        
        if (data_section == FALSE)
            add_to_ram(ram, index, command);
        else
            add_to_ram(ram, index + DATA_SECTION_OFFSET, command);
        
        index++;
    }

    execute_program(ram, register_file);
    print_registers(register_file);
    // print_RAM(ram);
    init_formatted_drive("my_drive.drive");
    FILE* drive;

    drive = fopen("my_drive.drive", "r+");
    create_file(drive, "new_file.txt", ".", 10);
    create_file(drive, "new123f-ile.c", ".", 10);
    create_file(drive, "file.", ".", 10);
    create_file(drive, "file&licious.exe", ".", 10);
    
    free(register_file);
    free(commands);
    
    return 0;
}
