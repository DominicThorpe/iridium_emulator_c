#ifndef RAMHASH
#define RAMHASH

#include <stdint.h>


/*
Represents a node in a linked list in the RAM hashmap data structure.
*/
typedef struct RAMKeyValuePair {
    int key;
    short value;
    struct RAMKeyValuePair* next;
} RAMKeyValuePair;


/*
A type representing the hashmap which represents RAM.

Contains proverbial "buckets" which may contain 0, 1, or more items corresponding to values at
addresses in memory represented as a linked list.
*/
typedef struct RAM {
    RAMKeyValuePair** buckets;
} RAM;


RAM* init_RAM(long hash_capacity);
void add_to_ram(RAM* ram, unsigned int key, uint16_t value);
short get_from_ram(RAM* ram, unsigned int key);
void reset_RAM();
void print_RAM(RAM* ram);
void toggle_periodic_interrupts();
short get_periodic_interrupts_enabled();

#endif
