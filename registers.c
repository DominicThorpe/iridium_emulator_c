#include <stdlib.h>
#include <stdio.h>
#include "registers.h"


// Allocates memory for 16 registers and returns a pointer to that chunk of memory with all the registers
// initialised to 0.
Register* init_registers() {
    Register* register_file = malloc(sizeof(Register) * 16);
    for (int i = 0; i < 16; i++) {
        Register new_register;
        if (i < 0xC)
            new_register.word_16 = 0;
        else
            new_register.word_32 = 0;

        register_file[i] = new_register;
    }

    return register_file;
}


/*
Takes the index of a register, the new value, and a pointer to the array of registers, and changes 
the value in the register to the new value. Will change the register, represented by a Union, to the
field appropriate to that register, which is `short` if index < 12, and `int` otherwise.

The register with index 0 will always be 0 and cannot be changed.
*/
void update_register(unsigned int index, Register new_value, Register* registers) {
    if (index == 0)
        return;

    if (index < 12)
        registers[index].word_16 = new_value.word_16;
    else if (index < 16)
        registers[index].word_32 = new_value.word_32;
    else
        exit(-4);
}


/*
Takes the index of a register and the array of register, and returns a `Register` containing the value of
the register with that index. The field containing the value will be `short` if the index < 12, and `int`
otherwise.

The register at index 0 will always return 0.
*/
Register get_register(unsigned int index, Register* registers) {
    if (index == 0) {
        Register zero;
        zero.word_32 = 0;
        return zero;
    } else if (index > 15) {
        printf("Invalid register index %d\n", index);
        exit(-4);
    }

    return registers[index];
}


/*
Takes the array of registers (which should have length 16), and prints their values.
*/
void print_registers(Register* registers) {
    printf("$zero: 0x0000\n");

    for (int i = 1; i < 11; i++) {
        printf("$g%d: 0x%04hX\n", i-1, registers[i].word_16);
    }

    printf("$ua: 0x%04hX\n", registers[11].word_16);
    printf("$sp: 0x%08hX\n", registers[12].word_32);
    printf("$fp: 0x%08hX\n", registers[13].word_32);
    printf("$ra: 0x%08hX\n", registers[14].word_32);
    printf("$pc: 0x%08hX\n", registers[15].word_32);
}

