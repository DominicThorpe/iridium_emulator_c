#include <stdio.h>
#include <stdlib.h>
#include "internal_memory.h"
#include "registers.h"
#include "ALU.h"


/*
Takes a 16-bit binary command and decomposes it into 4-bit sections. Extracts the opcode and operands
from the instruction, and then executes ir appropriately, making the correct modifications to the RAM
and registers, pointers to which are passed as parameters.
*/
void execute_command(uint16_t command, uint8_t* ram, uint16_t* registers, uint16_t* pc) {
    // convert instruction to a bit field containing each nibble of data
    struct {
        uint16_t nibble_1 : 4;
        uint16_t nibble_2 : 4;
        uint16_t nibble_3 : 4;
        uint16_t nibble_4 : 4;
    } instr_components;

    instr_components.nibble_1 = (command & 0xF000) >> 12;
    instr_components.nibble_2 = (command & 0x0F00) >> 8;
    instr_components.nibble_3 = (command & 0x00F0) >> 4;
    instr_components.nibble_4 = command & 0x000F;

    uint16_t operand_1, operand_2, operand_3;
    int immediate, mask, upper_addr;
    short code;
    uint16_t result_reg;

    if (instr_components.nibble_1 == 0xF) { // 8-bit opcode
        switch (instr_components.nibble_2) {
            case 0x0: // ADDC
                operand_1 = registers[instr_components.nibble_3];
                addition(operand_1, alu_flags.carry, instr_components.nibble_4, registers);
                break;
            
            case 0x1: // SUBC
                operand_1 = registers[instr_components.nibble_3];
                subtraction(operand_1, alu_flags.carry, instr_components.nibble_4, registers);
                break;
            
            case 0x2: // JUMP
                result_reg = (registers[instr_components.nibble_3] << 16) + registers[instr_components.nibble_4] - 1;
                *pc = result_reg;
                break;
            
            case 0x3: // JAL
                result_reg = (registers[instr_components.nibble_3]);
                update_register(14, *pc, registers);
                *pc = result_reg;
                break;
            
            case 0x4: // CMP
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                subtraction(operand_2, operand_1, 0, registers); // output to $zero
                break;
            
            case 0x5: // BEQ
                result_reg = registers[instr_components.nibble_3];
                if (alu_flags.zero == 1)
                    *pc = result_reg;

                break;
            
            case 0x6: // BNE
                result_reg = registers[instr_components.nibble_3];
                if (alu_flags.zero == 0)
                    *pc = result_reg;

                break;
            
            case 0x7: // BLT
                result_reg = registers[instr_components.nibble_3];
                if (alu_flags.negative == 0)
                    *pc = result_reg;

                break;
            
            case 0x8: // BGT
                result_reg = registers[instr_components.nibble_3];
                if (alu_flags.negative == 1)
                    *pc = result_reg;

                break;
            
            case 0x9: // IN            
            case 0xA: // OUT            
            case 0xC: // syscall            
            case 0xF: // HALT            
            default: 
                break;
        }
    } else { // 4-bit opcode
        switch (instr_components.nibble_1) {
            case 0x0: // NOP
                1 + 1; // waste a clock cycle
                break;
            
            case 0x1: // ADD
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                addition(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x2: // SUB
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                subtraction(operand_1, operand_2, instr_components.nibble_2, registers);
                break;

            case 0x3: // ADDI
                operand_1 = registers[instr_components.nibble_3];
                addition(operand_1, instr_components.nibble_4, instr_components.nibble_2, registers);
                break;
            
            case 0x4: // SUBI
                operand_1 = registers[instr_components.nibble_3];
                subtraction(operand_1, instr_components.nibble_4, instr_components.nibble_2, registers);
                break;
            
            case 0x5: // SLL
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                left_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x6: // SRL
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                logical_right_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x7: // SRA
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                arithmetic_right_shift(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x8: // NAND
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                logical_nand(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0x9: // OR
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                logical_or(operand_1, operand_2, instr_components.nibble_2, registers);
                break;
            
            case 0xA: // LOAD
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                upper_addr = get_register(11, registers);

                immediate = get_from_ram(ram, (upper_addr << 16) + (operand_1 + operand_2));
                result_reg = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xB: // STORE
                operand_1 = registers[instr_components.nibble_3];
                operand_2 = registers[instr_components.nibble_4];
                upper_addr = get_register(11, registers);
                immediate = get_register(instr_components.nibble_2, registers);
                
                add_to_ram(ram, (upper_addr << 16) + (operand_1 + operand_2), immediate);

                break;
            
            case 0xC: // MOVUI
                immediate = registers[instr_components.nibble_2];

                mask = 0xFFFF00FF;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 12;
                immediate |= instr_components.nibble_4 << 8;

                result_reg = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            case 0xD: // MOVLI
                immediate = registers[instr_components.nibble_2];

                mask = 0xFFFFFF00;
                immediate &= mask;
                immediate |= instr_components.nibble_3 << 4;
                immediate |= instr_components.nibble_4;

                result_reg = immediate;

                update_register(instr_components.nibble_2, result_reg, registers);
                break;
            
            default:
                exit(-3);
                break;
        }
    }
}
