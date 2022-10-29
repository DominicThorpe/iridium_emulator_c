#include <stdio.h>
#include <stdlib.h>
#include "ram.h"
#include "registers.h"
#include "ALU.h"

#define GET_REG_VAL(index) index < 12 ? get_register(index, registers).word_16 : get_register(index, registers).word_32


/*
Takes a 16-bit binary command and decomposes it into 4-bit sections. Extracts the opcode and operands
from the instruction, and then executes ir appropriately, making the correct modifications to the RAM
and registers, pointers to which are passed as parameters.
*/
void execute_command(short command, RAM* ram, Register* registers) {
    // convert instruction to a bit field containing each nibble of data
    struct {
        unsigned int nibble_1 : 4;
        unsigned int nibble_2 : 4;
        unsigned int nibble_3 : 4;
        unsigned int nibble_4 : 4;
    } instr_components;

    instr_components.nibble_1 = (command & 0xF000) >> 12;
    instr_components.nibble_2 = (command & 0x0F00) >> 8;
    instr_components.nibble_3 = (command & 0x00F0) >> 4;
    instr_components.nibble_4 = command & 0x000F;

    int operand_1, operand_2, operand_3;
    int immediate, mask, upper_addr;
    Register result_reg;

    if (instr_components.nibble_1 == 0xF) { // 8-bit opcode
        
    } else { // 4-bit opcode
        switch (instr_components.nibble_1) {
            case 0x0: // NOP
                1 + 1; // waste a clock cycle
                break;
            
            case 0x1: // ADD
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                addition(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x2: // SUB
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                subtraction(operand_1, operand_2, instr_components.nibble_2, registers);
                break;

            case 0x3: // ADDI
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                addition(operand_1, instr_components.nibble_4, instr_components.nibble_2, registers);
                break;
            
            case 0x4: // SUBI
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                subtraction(operand_1, instr_components.nibble_4, instr_components.nibble_2, registers);
                break;
            
            case 0x5: // SLL
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                left_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x6: // SRL
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                logical_right_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x7: // SRA
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                arithmetic_right_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x8: // NAND
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                
                if (instr_components.nibble_2 < 12)
                    result_reg.word_16 = ~(operand_1 & operand_2);
                else
                    result_reg.word_32 = ~(operand_1 & operand_2);

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x9: // OR
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                
                if (instr_components.nibble_2 < 12)
                    result_reg.word_16 = operand_1 | operand_2;
                else
                    result_reg.word_32 = operand_1 | operand_2;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xA: // LOAD
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                upper_addr = get_register(11, registers).word_16;

                immediate = get_from_ram(ram, upper_addr | (operand_1 + operand_2));
                if (instr_components.nibble_2 < 12)
                    result_reg.word_16 = immediate;
                else
                    result_reg.word_32 = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xB: // STORE
                operand_1 = GET_REG_VAL(instr_components.nibble_3);
                operand_2 = GET_REG_VAL(instr_components.nibble_4);
                upper_addr = get_register(11, registers).word_16 << 16;

                if (instr_components.nibble_2 < 12)
                    immediate = get_register(instr_components.nibble_2, registers).word_16;
                else
                    immediate = get_register(instr_components.nibble_2, registers).word_32 & 0x0000FFFF;
                
                add_to_ram(ram, upper_addr | (operand_1 + operand_2), immediate);

                break;
            
            case 0xC: // MOVUI
                immediate = GET_REG_VAL(instr_components.nibble_2);

                mask = 0xFFFF00FF;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 12;
                immediate |= instr_components.nibble_4 << 8;

                if (instr_components.nibble_2 < 12)
                    result_reg.word_16 = immediate;
                else
                    result_reg.word_32 = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xD: // MOVLI
                immediate = GET_REG_VAL(instr_components.nibble_2);

                mask = 0xFFFFFF00;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 4;
                immediate |= instr_components.nibble_4;

                if (instr_components.nibble_2 < 12)
                    result_reg.word_16 = immediate;
                else
                    result_reg.word_32 = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            default:
                exit(-3);
                break;
        }
    }
}
