#include <stdlib.h>
#include <stdio.h>
#include "registers.h"


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
        printf("Incorrect number of arguments!\nUSAGE: emulator <filename>");
        exit(2);
    }
    
    char* commands = read_commands(argv[1]);
    for (long i = 0; i < 0x50; i++) {
        printf("%04X: %02X\n", i, commands[i] & 0xFF);
    }
    

    Register* register_file = init_registers();
    
    free(register_file);
    free(commands);
    
    return 0;
}
