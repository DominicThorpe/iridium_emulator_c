#ifndef REGISTERS
#define REGISTERS

#include <stdint.h>

typedef union Register {
    uint16_t word_16;
    uint32_t word_32;
} Register;


Register* init_registers();
void update_register(unsigned int index, Register new_value, Register* registers);
Register get_register(unsigned int index, Register* registers);
void print_registers(Register* registers);

#endif
