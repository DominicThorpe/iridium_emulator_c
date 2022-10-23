#ifndef REGISTERS
#define REGISTERS

typedef union Register {
    unsigned short word_16;
    unsigned int word_32;
} Register;


Register* init_registers();

#endif
