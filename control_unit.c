#include <stdio.h>
#include <stdlib.h>
#include "ram.h"
#include "registers.h"


void execute_command(short command, RAM* ram, Register* registers) {
    // if the command has a 4-bit opcode, get only first nibble, else get whole byte
    // TODO: maybe convert to bit-field?
    char nibble_1 = (command & 0xF000) >> 12;
    char nibble_2 = (command & 0x0F00) >> 8;
    char nibble_3 = (command & 0x00F0) >> 4;
    char nibble_4 = command & 0x000F;

    int operand_1, operand_2, operand_3;
    Register result_reg;

    if (nibble_1 == 0xF) {
        
    } else {
        switch (nibble_1) {
            case 0x0: // NOP
                break;
            
            case 0x1: // ADD
                operand_1 = nibble_3 < 12 ? 
                                get_register(nibble_3, registers).word_16 : 
                                get_register(nibble_3, registers).word_32;
                operand_2 = nibble_4 < 12 ? 
                                get_register(nibble_4, registers).word_16 : 
                                get_register(nibble_4, registers).word_32;
                
                if (nibble_2 < 12) {
                    result_reg.word_16 = operand_1 + operand_2;
                } else {
                    result_reg.word_32 = operand_1 + operand_2;
                }

                update_register(nibble_2, result_reg, registers);
                break;
            
            case 0x2: // SUB
                break;

            case 0x3: // ADDI
                operand_1 = nibble_3 < 12 ? 
                                get_register(nibble_3, registers).word_16 : 
                                get_register(nibble_3, registers).word_32;

                if (nibble_2 < 12) {
                    result_reg.word_16 = operand_1 + nibble_4;
                } else {
                    result_reg.word_32 = operand_1 + nibble_4;
                }

                update_register(nibble_2, result_reg, registers);
                break;
            
            case 0x4:
                break;
            
            case 0x5:
                break;
            
            case 0x6:
                break;
            
            case 0x7:
                break;
            
            case 0x8:
                break;
            
            case 0x9:
                break;
            
            case 0xA:
                break;
            
            case 0xB:
                break;
            
            case 0xC:
                break;
            
            case 0xD:
                break;
            
            case 0xE:
                break;
            
            case 0xF:
                break;
            
            default:
                exit(-3);
                break;
        }
    }
}
