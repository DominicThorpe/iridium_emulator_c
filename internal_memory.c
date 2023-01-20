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
#include <stdio.h>
#include <stdint.h>
#include "internal_memory.h"

#define FALSE 0
#define TRUE  1


static long ram_hash_capacity = -1;
static short periodic_interrupt_enabled = TRUE;
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


/**
 * @brief Hashes the key (RAM address) passed and adds it to RAM, represented as a linked-list hashmap
 * 
 * @param ram Pointer to the system RAM
 * @param key Address to add the data to
 * @param value The data to add to RAM
 */
void add_to_ram(RAM* ram, unsigned int key, uint16_t value) {
    printf("Writing 0x%04X to 0x%08X\n", value, key);

    // get hash and create new element in the linked list
    long hash = hash_function(key);
    RAMKeyValuePair* pair = malloc(sizeof(RAMKeyValuePair));
    pair->key = key;
    pair->value = value;
    pair->next = NULL;

    // if the start of the new linked list, add immediately
    if (ram->buckets[hash] == NULL)
        ram->buckets[hash] = pair;
    
    // otherwise go through the linked list at that hash until you get to the end and add it there
    else {
        RAMKeyValuePair* current_kvp = ram->buckets[hash];
        while (current_kvp->next != NULL) {
            if (current_kvp->key == key) {
                current_kvp->value = value;
                return;
            }

            current_kvp = current_kvp->next;
        }

        // for if the end of the linked list has the same key as the new data
        if (current_kvp->key == key) {
            current_kvp->value = value;
            return;
        }

        current_kvp->next = pair;
    }
}


/*
Takes a pointer to the RAM and finds the correct bucket for that key. Goes through each item in
the corresponding linked list and returns the value of the first RAMKeyValuePair it finds with
the correct key. 
*/
short get_from_ram(RAM* ram, unsigned int key) {
    long hash = hash_function(key);
    if (ram->buckets[hash] != NULL && ram->buckets[hash]->key == key)
        return ram->buckets[hash]->value; 

    else if (ram->buckets[hash] != NULL) {
        RAMKeyValuePair* current_kvp = ram->buckets[hash];
        if (current_kvp->key == key) // problem here or here
            return current_kvp->value; // problem here or here

        while (current_kvp->next != NULL) {
            current_kvp = current_kvp->next;
            if (current_kvp->key == key)
                return current_kvp->value;
        }
    }
    
    printf("Could not get key %u from RAM\n", key);
    return 0;
}


/*
Iterates through each bucket in RAM and prints the contents from top to bottom.
*/
void print_RAM(RAM* ram) {
    RAMKeyValuePair* current_bucket;
    for (long i = 0; i < ram_hash_capacity; i++) {
        if (ram->buckets[i] == NULL)
            continue;
        
        current_bucket = ram->buckets[i];
        
        printf("Bucket %03X:\n", i);
        do {
            printf("    0x%08X:\t0x%04hX\n", current_bucket->key, current_bucket->value);
            current_bucket = current_bucket->next;
        } while (current_bucket != NULL);

        printf("\n");
    }
}


void toggle_periodic_interrupts() {
    periodic_interrupt_enabled = !periodic_interrupt_enabled;
}


short get_periodic_interrupts_enabled() {
    return periodic_interrupt_enabled;
}


/*
Resets static variable `ram_is_initialised` to 0 (aka. FALSE).

WARNING: FOR TESTING PURPOSES ONLY!
*/
void reset_RAM() {
    ram_is_initialised = FALSE;
}
