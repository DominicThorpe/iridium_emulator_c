#include <stdlib.h>
#include <stdio.h>
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
void set_flags(short val, int set_carry, short arg_a, short arg_b) {
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
        if (arg_a > 0 && arg_b > 0 && ((unsigned short)(arg_a + arg_b)) < 0)
            alu_flags.carry = 1;
        else if (arg_a < 0 && arg_b < 0 && ((unsigned short)(arg_a + arg_b)) > 0)
            alu_flags.carry = 1;
    }
}


/*
Takes 2 operands and outputs the sum of their values to a register, then sets the ALU flags
appropriately.
*/
void addition(short operand_a, short operand_b, unsigned int output_reg, Register* registers) {
    Register result_reg = get_register(output_reg, registers);
    if (output_reg < 12)
        result_reg.word_16 = operand_a + operand_b;
    else
        result_reg.word_32 = (result_reg.word_32 & 0xFFFF0000) | (operand_a + operand_b & 0x0000FFFF);

    set_flags(operand_a + operand_b, TRUE, operand_a, operand_b);
    update_register(output_reg, result_reg, registers);
}


/*
Performs subtraction by taking the compliment of operand B and adding one, thereby getting the
2s-compliment of operand B, which is -B, and addingit to A, because A +- B = A - B.
*/
void subtraction(short operand_a, short operand_b, unsigned int output_reg, Register* registers) {
    printf("%d - %d\n", operand_a, operand_b);
    operand_b = (~operand_b) + 1;
    addition(operand_a, operand_b, output_reg, registers);
}
