#ifndef REGISTERS
#define REGISTERS

#include <stdint.h>

uint16_t* init_registers();
void update_register(unsigned int index, uint16_t new_value, uint16_t* registers);
uint16_t get_register(unsigned int index, uint16_t* registers);
void print_registers(uint16_t* registers);

#endif
