#ifndef FATFUNCS
#define FATFUNCS

#include <stdio.h>
#include <stdint.h>
#include "sys_meta.h"
#include "file_meta.h"


extern uint16_t* FAT;


// Represents a pointer to a location in a FAT
typedef struct fatptr {
    FILE* fileptr;
    Metadata* sys_context;
    Filedir* file_context;
    long sector_num;
    long start_sector;
    long next_sector;
    int current_pos;
    uint8_t id;
} FATPtr;

FILE* init_harddrive(Metadata* metadata);
FATPtr* f_open(FILE* image, char* dir);
void f_seek(FATPtr* fileptr, long offset, short whence);
void f_read(FATPtr* fileptr, long bytes, char* buffer);
void f_close(FATPtr* fileptr, int id);
Filedir* iterate_directory(FILE* image, int addr, int* num_dirs);
long get_addr_from_cluster(long cluster_num, Metadata* metadata);
void print_open_files();
FATPtr* get_open_file_id(int id);

#endif