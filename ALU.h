#ifndef ALU
#define ALU

#include <stdint.h>
#include "registers.h"


struct ALU_flags {
    uint8_t zero : 1;
    uint8_t negative : 1;
    uint8_t carry : 1;
} alu_flags;


void set_flags(uint16_t val, int set_carry, uint16_t arg_a, uint16_t arg_b);
void addition(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void subtraction(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void left_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void arithmetic_right_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void logical_right_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void logical_nand(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);
void logical_or(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers);


#endif
