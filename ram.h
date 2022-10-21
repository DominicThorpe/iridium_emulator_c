#ifndef RAMHASH
#define RAMHASH


/*
Represents a node in a linked list in the RAM hashmap data structure.
*/
typedef struct RAMKeyValuePair {
    char* key;
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

#endif
