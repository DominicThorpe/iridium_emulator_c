#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "registers.h"
#include "ALU.h"

#define FALSE 0
#define TRUE  1


/* 
Checks the result of ALU operations and sets the flags accordingly, which are:
  - zero (Z): set if the result of the operation was 0,
  - negative (N): set if the result of the operation was negative
  - carry (C): set if the result of the *unsigned* operation was incorrect (only for addition and subtraction)
*/
void set_flags(uint16_t val, int set_carry, uint16_t arg_a, uint16_t arg_b) {
    alu_flags.zero = 0;
    alu_flags.carry = 0;
    alu_flags.negative = 0;

    if (val == 0)
        alu_flags.zero = 1;
    else if (val < 0)
        alu_flags.negative = 1;
    
    // If the sign of the result does not match the sign of the operands, and the operands have the same
    // sign, then there has been an overflow
    if (set_carry == TRUE) {
        if (arg_a > 0 && arg_b > 0 && ((uint16_t)(arg_a + arg_b)) < 0)
            alu_flags.carry = 1;
        else if (arg_a < 0 && arg_b < 0 && ((uint16_t)(arg_a + arg_b)) > 0)
            alu_flags.carry = 1;
    }
}


/*
Takes 2 operands and outputs the sum of their values to a register, then sets the ALU flags
appropriately.
*/
void addition(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = get_register(output_reg, registers);
    result_reg = operand_a + operand_b;

    set_flags(operand_a + operand_b, TRUE, operand_a, operand_b);
    update_register(output_reg, result_reg, registers);
}


/*
Performs subtraction by taking the compliment of operand B and adding one, thereby getting the
2s-compliment of operand B, which is -B, and adding it to A, because A +- B = A - B.
*/
void subtraction(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    operand_b = (~operand_b) + 1;
    addition(operand_a, operand_b, output_reg, registers);
}


/*
Performs a left shift on operand A by the number of bits specified by operand B and stores the 
result in the output register. 

Does not set ALU flags.
*/
void left_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = operand_a << operand_b;
    update_register(output_reg, result_reg, registers);
}


/*
Performs a *logical* right shift on operand A by the number of bits specified by operand B and stores the 
result in the output register. 

Does not set ALU flags.
*/
void arithmetic_right_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = operand_a >> operand_b;
    update_register(output_reg, result_reg, registers);
}


/*
Performs an *arithmetic* right shift on operand A by the number of bits specified by operand B and stores the 
result in the output register. 

Does not set ALU flags.
*/
void logical_right_shift(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = operand_a >> operand_b;
    update_register(output_reg, result_reg, registers);
}


/* 
Performs the logical NAND operation on the 2 inputs and places the result in the output register.

Does not set ALU flags
*/
void logical_nand(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = ~(operand_a & operand_b);
    update_register(output_reg, result_reg, registers);
}


/* 
Performs the logical OR operation on the 2 inputs and places the result in the output register.

Does not set ALU flags
*/
void logical_or(uint16_t operand_a, uint16_t operand_b, unsigned int output_reg, uint16_t* registers) {
    uint16_t result_reg = operand_a | operand_b;
    update_register(output_reg, result_reg, registers);
}
