#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "../ALU.h"
#include "../registers.h"


/*
Test that ALU addition does the following:
  - will not change the $zero register
  - will add 16 registers properly
  - will properly add a 16-bit to a 32-bit register
  - sets the flags correctly
*/
void test_add() {
    Register* registers = init_registers();
    Register new_reg;

    // Check will not change the $zero register
    addition(5, 5, 0, registers);
    assert(get_register(0, registers).word_16 == 0);

    // Check will add 16-bit registers
    addition(0x4444, 0x4321, 1, registers);
    assert(get_register(1, registers).word_16 == 0x8765);

    // Check will add 32-bit registers (will only add the 1st 16 bits and AND them 
    // into the 1st 16-bits of the 32 bit-reg)    
    new_reg.word_32 = 0x55555555;
    update_register(13, new_reg, registers);
    addition(0x5555, 0xF123, 13, registers);
    assert(get_register(13, registers).word_32 == 0x55554678);

    // Check will properly add a 16-bit to a 32-bit register
    new_reg.word_32 = 0xAAAAAAAA;
    update_register(13, new_reg, registers);
    addition(0x1234, 0xAAAA, 13, registers);
    assert(get_register(13, registers).word_32 == 0xAAAABCDE);

    // Check sets flags correctly
    addition(0, 0, 1, registers);
    assert(alu_flags.zero == 1);
    assert(alu_flags.carry == 0);
    assert(alu_flags.negative == 0);

    addition(5, 5, 1, registers);
    assert(alu_flags.zero == 0);
    assert(alu_flags.carry == 0);
    assert(alu_flags.negative == 0);

    new_reg.word_32 = 0;
    update_register(0, new_reg, registers);
    addition(0xF000, 0xA000, 1, registers);
    assert(alu_flags.zero == 0);
    assert(alu_flags.carry == 1);
    assert(alu_flags.negative == 1);
}


void test_ALU() {
    test_add();
}
