#include "registers.h"


struct {
    unsigned int zero : 1;
    unsigned int negative : 1;
    unsigned int carry : 1;
} alu_flags;


void set_flags(short val, int set_carry, short arg_a, short arg_b);
void addition(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
