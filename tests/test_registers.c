#include <stdlib.h>
#include <assert.h>
#include "../registers.h"


/*
Test the following situations:
  - All registers should be 0 at initialisation
*/
void test_registers_init() {
    Register* registers = init_registers();
    for (int i = 0; i < 16; i++) {
        if (i < 12)
            assert(registers[i].word_16 == 0);
        else
            assert(registers[i].word_32 == 0);
    }

    free(registers);
}


/*
Test the following situations:
  - $zero shall always return constant 0, even after an attempt to update it
  - 16-bit registers should behave properly when updated to a valid positive number
  - 16-bit registers should behave properly when updated to a valid negative number
  - 16-bit registers should behave properly when updated to a valid positive number at the edge of the valid range
  - 16-bit registers should behave properly when updated to a valid negative number at the edge of the valid range
  - 16-bit registers should not behave when updated to a positive number outside the valid range
  - 16-bit registers should not behave when updated to a negative number outside the valid range
  - 32-bit registers should do all of the above
*/
void test_registers_update() {
    Register* registers = init_registers();

    // Check constant 0 cannot be updated
    Register new_val;
    new_val.word_16 = 0; 
    update_register(0, new_val, registers);
    assert(get_register(0, registers).word_16 == 0 && get_register(0, registers).word_32 == 0);

    // Check 16-bit registers behave properly when updated to a valid positive number
    new_val.word_16 = 0x7F0F; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0x7F0F);

    // Check 32-bit registers behave properly when updated to a valid positive number
    new_val.word_32 = 0x700FFFFF; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0x700FFFFF);

    // Check 16-bit registers behave properly when updated to a valid positive number at the edge of the valid range
    new_val.word_16 = 0x7FFF; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0x7FFF);

    // Check 32-bit registers behave properly when updated to a valid positive number at the edge of the valid range
    new_val.word_32 = 0x7FFFFFFF; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0x7FFFFFFF);

    // Check 16-bit registers behave properly when updated to a valid negative number
    new_val.word_16 = -1; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0xFFFF);

    // Check 32-bit registers behave properly when updated to a valid negative number
    new_val.word_32 = -1; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0xFFFFFFFF);

    // Check 16-bit registers behave properly when updated to a valid negative number at the edge of the valid range
    new_val.word_16 = -32768; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0x8000);

    // Check 32-bit registers behave properly when updated to a valid negative number at the edge of the valid range
    new_val.word_32 = -2147483648; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0x80000000);

    // Check 16-bit register does not behave when updated to a positive number outside the valid range
    new_val.word_16 = 400000; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0x1A80);

    // Check 32-bit register does not behave when updated to a positive number outside the valid range
    new_val.word_32 = 0xFEDCBA987654321; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0x87654321);

    // Check 16-bit register does not behave when updated to a positive number outside the valid range
    new_val.word_16 = -400000; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 0xE580);

    // Check 32-bit register does not behave when updated to a positive number outside the valid range
    new_val.word_32 = -40000000000; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 0xAFD07000);

    free(registers);
}


/*
Test the following situations:
  - $zero shall always return constant 0
  - 16-bit registers shall return the set value
  - 32-bit registers shall return the set value
*/
void test_registers_get() {
    Register* registers = init_registers();
    Register new_val;

    // Check $zero always returns constant 0
    assert(get_register(0, registers).word_16 == 0);
    assert(get_register(0, registers).word_32 == 0);

    // Check 16-bit registers return the correct value
    new_val.word_16 = 10; 
    update_register(1, new_val, registers);
    assert(get_register(1, registers).word_16 == 10);

    // Check 32-bit registers return the correct value
    new_val.word_32 = 80000; 
    update_register(14, new_val, registers);
    assert(get_register(14, registers).word_32 == 80000);

    free(registers);
}
