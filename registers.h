#ifndef REGISTERS
#define REGISTERS

#include <stdint.h>

#define GET_REG_VAL(index) index < 12 ? get_register(index, registers).word_16 : get_register(index, registers).word_32

typedef union Register {
    uint16_t word_16;
    uint32_t word_32;
} Register;


Register* init_registers();
void update_register(unsigned int index, Register new_value, Register* registers);
Register get_register(unsigned int index, Register* registers);
void print_registers(Register* registers);

#endif
