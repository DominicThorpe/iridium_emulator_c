/*
RAM is represented as a 4KiB, long-addressed array of 8-bit numbers. This is malloc-ed all at
once and is accessed through special functions to ensure it is accessed in a valid way as this
is not trivial.
*/


#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "internal_memory.h"

#define TRUE  1
#define FALSE 0


static short ram_is_initialised = FALSE;


/**
 * @brief Initialises the RAM to an array of 65,535 bytes and returns a pointer to it
 * 
 * @return RAM* pointer to the RAM
 */
uint8_t* init_RAM() {
    // Don't let RAM be initialised if hash capacity is less than 1 or ram is already initialised
    if (ram_is_initialised == TRUE)
        exit(-2);
    
    ram_is_initialised = TRUE;
        
    // pointer to the start of an array of 8 bit integers, initialised to random values
    uint8_t* RAM = malloc(sizeof(uint8_t) * 0xFFFF); 
    return RAM;
}


/**
 * @brief Inserts the given value into the RAM at the given address
 * 
 * @param ram Pointer to the system RAM
 * @param key Address to add the data to
 * @param value The data to add to RAM
 */
void add_to_ram(uint8_t* ram, uint16_t key, uint8_t value) {
    ram[key] = value;
}


/**
 * @brief Get the value at the given address from the RAM
 * 
 * @param ram An array representing the system RAM
 * @param key The address to get the data in RAM from
 * @return The value at that address in RAM
 */
uint8_t get_from_ram(uint8_t* ram, uint16_t key) {
    return ram[key];
}


/**
 * @brief Iterates through each bucket in RAM and prints the contents from top to bottom.
 * 
 * @param ram Pointer to the system RAM
 */
void print_RAM(uint8_t* ram) {
    for (long i = 0; i < 0xFFFF; i++) {
        if (ram[i] == 0)
            continue;
        
        printf("%04X: %d\n", i, ram[i]);
    }
}


/*
Resets static variable `ram_is_initialised` to 0 (aka. FALSE).

WARNING: FOR TESTING PURPOSES ONLY!
*/
void reset_RAM() {
    ram_is_initialised = FALSE;
}
