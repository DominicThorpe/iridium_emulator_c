/*
Implementing RAM as a hashmap.

It would not be at all memory efficient to create an array 4GB in size to hold the entirety of the 
RAM for the system. Therefore, each address in RAM which has data will be a key in a hash table with 
a simple hashing algorithm. 

The hashmap contains 1024 proverbial "buckets" which each are a linked list which may be empty. 

When adding a value to RAM, the item is appended to the linked list corresponding to the result of the 
hash function when applied to its address. When removing a value, the item is removed from the 
corresponding linked list. If the address is not in the hashmap, it will return 0.
*/


#include <stdlib.h>
#include "ram.h"

#define FALSE 0
#define TRUE  1


static long ram_hash_capacity = -1;
static short ram_is_initialised = FALSE;


RAM* init_RAM(long hash_capacity) {
    // Don't let RAM be initialised if hash capacity is less than 1 or ram is already initialised
    if (hash_capacity <= 0 || ram_is_initialised == TRUE)
        exit(-2);
    
    ram_hash_capacity = hash_capacity;
    ram_is_initialised = TRUE;
        
    // pointer to the start of an array of pointers to linked lists (i.e. the buckets)
    RAMKeyValuePair** RAM_initial_state = malloc(sizeof(RAMKeyValuePair) * hash_capacity);
    for (unsigned long i = 0; i < hash_capacity; i++) { // initialise all buckets to null
        RAM_initial_state[i] = NULL; 
    }
    

    RAM* ram = malloc(sizeof(RAM));
    ram->buckets = RAM_initial_state;

    return ram;
}



long hash_function(int input) {
    return abs(input) % ram_hash_capacity;
}
