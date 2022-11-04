#ifndef ALU
#define ALU
#include "registers.h"


struct {
    unsigned int zero : 1;
    unsigned int negative : 1;
    unsigned int carry : 1;
} alu_flags;


void set_flags(short val, int set_carry, short arg_a, short arg_b);
void addition(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void subtraction(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void left_shift(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void arithmetic_right_shift(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void logical_right_shift(unsigned short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void logical_nand(short operand_a, short operand_b, unsigned int output_reg, Register* registers);
void logical_or(short operand_a, short operand_b, unsigned int output_reg, Register* registers);


#endif
