#ifndef REGISTERS
#define REGISTERS

typedef union Register {
    unsigned short word_16;
    unsigned int word_32;
} Register;


Register* init_registers();
void update_register(unsigned int index, Register new_value, Register* registers);
Register get_register(unsigned int index, Register* registers);
void print_registers(Register* registers);

#endif
