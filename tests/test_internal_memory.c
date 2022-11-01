#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "../internal_memory.h"


/*
Upon initialisation the RAM should:
  - have the correct number of "buckets"
  - all buckets should be empty (pointers == NULL)
*/
void test_ram_init() {
    RAM* ram = init_RAM(1024);
    for (long i = 0; i < 1024; i++) {
        assert(ram->buckets[i] == NULL);
    }

    free (ram);
}


/*
When inserting into the RAM the value should:
  - be inserted at the top correctly if the first value in that bucket
  - be inserted at the end if not the first value in that bucket
  - properly store all correct 16-bit values
  - not properly store values that overflow 16-bits
*/
void test_ram_insert() {
    reset_RAM(); // allow for a new RAM to be initialised

    RAM* ram = init_RAM(1024);
    add_to_ram(ram, 0, 0x0000);
    add_to_ram(ram, 1, 0x000F);
    add_to_ram(ram, 1024, 0x0005);
    add_to_ram(ram, 2048, 0x0006);
    add_to_ram(ram, 100, 0xABCDEF);

    assert(get_from_ram(ram, 0) == 0);
    assert(get_from_ram(ram, 1) == 15);
    assert(get_from_ram(ram, 1024) == 5);
    assert(get_from_ram(ram, 2048) == 6);
    assert(get_from_ram(ram, 1) != 0xABCDEF);
    free(ram);
}


/* 
When updating an existant value in RAM, the value should:
  - update the correct value at the start of the bucket
  - update the correct value in the middle of the bucket
  - update the correct value at the end of the bucket
*/
void test_ram_update() {
    reset_RAM(); // allow for a new RAM to be initialised

    // initialise values
    RAM* ram = init_RAM(1024);
    add_to_ram(ram, 0, 0x0000);
    add_to_ram(ram, 1024, 0x0005);
    add_to_ram(ram, 2048, 0x0006);

    // update values
    add_to_ram(ram, 0, 0x0001);
    add_to_ram(ram, 1024, 0x0002);
    add_to_ram(ram, 2048, 0x0003);

    assert(get_from_ram(ram, 0) == 1);
    assert(get_from_ram(ram, 1024) == 2);
    assert(get_from_ram(ram, 2048) == 3);
}
