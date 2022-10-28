#include <stdio.h>
#include "test_registers.h"
#include "test_RAM.h"
#include "test_ALU.h"


int main() {
    // testing the registers
    test_registers_init();
    test_registers_update();
    test_registers_get();
    printf("Registers OK!\n");

    // testing the RAM
    test_ram_init();
    test_ram_insert();
    test_ram_update();
    printf("RAM OK!\n");

    test_ALU();
    printf("ALU OK!\n");

    printf("\nALL TESTS PASSED!\n");
    
    return 0;
}
