#include <stdlib.h>
#include <stdio.h>
#include "registers.h"



int main() {
    Register* register_file = init_registers();
    for (int i = 0; i < 16; i++) {
        printf("Register %X = %hu\n", i, register_file[i]);
    }

    free(register_file);
    
    return 0;
}
