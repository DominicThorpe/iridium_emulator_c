#include <stdio.h>
#include <stdlib.h>
#include "ram.h"
#include "registers.h"


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
    int immediate, mask;
    Register result_reg;

    if (instr_components.nibble_1 == 0xF) { // 8-bit opcode
        
    } else { // 4-bit opcode
        switch (instr_components.nibble_1) {
            case 0x0: // NOP
                1 + 1;
                break;
            
            case 0x1: // ADD
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 + operand_2;
                } else {
                    result_reg.word_32 = operand_1 + operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x2: // SUB
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 - operand_2;
                } else {
                    result_reg.word_32 = operand_1 - operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;

            case 0x3: // ADDI
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;

                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 + instr_components.nibble_4;
                } else {
                    result_reg.word_32 = operand_1 + instr_components.nibble_4;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x4: // SUBI
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;

                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 - instr_components.nibble_4;
                } else {
                    result_reg.word_32 = operand_1 - instr_components.nibble_4;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x5: // SLL
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 << operand_2;
                } else {
                    result_reg.word_32 = operand_1 << operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x6: // SRL
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                unsigned short unsigned_1 = operand_1;
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = unsigned_1 >> operand_2;
                } else {
                    result_reg.word_32 = unsigned_1 >> operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x7: // SRA
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = (short)operand_1 >> (short)operand_2;
                } else {
                    result_reg.word_32 = operand_1 >> operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x8: // NAND
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = ~(operand_1 & operand_2);
                } else {
                    result_reg.word_32 = ~(operand_1 & operand_2);
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0x9: // OR
                operand_1 = instr_components.nibble_3 < 12 ? 
                                get_register(instr_components.nibble_3, registers).word_16 : 
                                get_register(instr_components.nibble_3, registers).word_32;
                operand_2 = instr_components.nibble_4 < 12 ? 
                                get_register(instr_components.nibble_4, registers).word_16 : 
                                get_register(instr_components.nibble_4, registers).word_32;
                
                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = operand_1 | operand_2;
                } else {
                    result_reg.word_32 = operand_1 | operand_2;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xA: // LOAD
                break;
            
            case 0xB: // STORE
                break;
            
            case 0xC: // MOVUI
                if (instr_components.nibble_2 < 12)
                    immediate = get_register(instr_components.nibble_2, registers).word_16;
                else
                    immediate = get_register(instr_components.nibble_2, registers).word_32;

                mask = 0xFFFF00FF;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 12;
                immediate |= instr_components.nibble_4 << 8;

                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = immediate;
                } else {
                    result_reg.word_32 = immediate;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xD: // MOVLI
                if (instr_components.nibble_2 < 12)
                    immediate = get_register(instr_components.nibble_2, registers).word_16;
                else
                    immediate = get_register(instr_components.nibble_2, registers).word_32;

                mask = 0xFFFFFF00;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 4;
                immediate |= instr_components.nibble_4;

                if (instr_components.nibble_2 < 12) {
                    result_reg.word_16 = immediate;
                } else {
                    result_reg.word_32 = immediate;
                }

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            default:
                exit(-3);
                break;
        }
    }
}
