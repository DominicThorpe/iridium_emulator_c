#include <stdlib.h>
#include "registers.h"


// Allocates memory for 16 registers and returns a pointer to that chunk of memory with all the registers
// initialised to 0.
Register* init_registers() {
    Register* register_file = malloc(sizeof(Register) * 16);
    for (int i = 0; i < 16; i++) {
        Register new_register = { 0 };
        register_file[i] = new_register;
    }

    return register_file;
}

