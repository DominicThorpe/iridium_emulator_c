#ifndef RAMHASH
#define RAMHASH

#include <stdint.h>

uint8_t* init_RAM();
void add_to_ram(uint8_t* ram, uint16_t key, uint8_t value);
uint8_t get_from_ram(uint8_t* ram, uint16_t key);
void reset_RAM();
void print_RAM(uint8_t* ram);

#endif
