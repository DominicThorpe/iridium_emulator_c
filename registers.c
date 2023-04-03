#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "registers.h"


/*
Allocates memory for 16 registers and returns a pointer to that chunk of memory with all the registers
initialised to 0.
*/
uint16_t* init_registers() {
    uint16_t* register_file = malloc(sizeof(uint16_t));
    for (uint8_t i = 0; i < 16; i++) {
        register_file[i] = 0;
    }
    
    return register_file;
}


/*
Takes the index of a register, the new value, and a pointer to the array of registers, and changes 
the value in the register to the new value.

The register with index 0 will always be 0 and cannot be changed.
*/
void update_register(unsigned int index, uint16_t new_value, uint16_t* registers) {
    if (index == 0)
        return;

    registers[index] = new_value;
}


/*
Takes the index of a register and the array of register, and returns the value of the register with 
that index. 

The register at index 0 will always return 0.
*/
uint16_t get_register(unsigned int index, uint16_t* registers) {
    if (index == 0)
        return 0;
    else if (index > 15) {
        printf("Invalid register index %d\n", index);
        exit(-4);
    }

    return registers[index];
}


/*
Takes the array of registers (which should have length 16), and prints their values.
*/
void print_registers(uint16_t* registers) {
    printf("$zero: 0x0000\n");

    for (int i = 1; i < 12; i++) {
        printf("$g%d: 0x%04hX\n", i-1, registers[i]);
    }

    printf("$ar: 0x%04hX\n", registers[12]);
    printf("$sp: 0x%08hX\n", registers[13]);
    printf("$fp: 0x%08hX\n", registers[14]);
    printf("$ra: 0x%08hX\n", registers[15]);
}

