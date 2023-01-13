#ifndef SYSMETA
#define SYSMETA

#include <stdint.h>
#include <stdio.h>


extern uint16_t* FAT;


/**
 * @brief Designed to store the metadata for a FAT16 image.
 */
typedef struct metadata {
    uint16_t    BPB_BytesPerSec;    // Bytes per sector
    uint8_t     BPB_SecPerClus;     // Sectors per cluster
    uint16_t    BPB_RsvdSecCnt;     // Reserved sector count
    uint8_t     BPB_NumFATs;        // Num FAT tables
    uint16_t    BPB_RootEntCnt;     // FAT12/16 size of root dir 
    uint16_t    BPB_TotSec16;       // Sectors, may be 0
    uint16_t    BPB_FATz16;         // Sectors in FAT
    uint32_t    BPB_TotSec32;       // Sectors if BPB_TotSec16 == 0
    uint8_t     BS_VolLab[ 11 ];    // Non 0-terminated string
} Metadata;

void read_sys_metadata(FILE* image, Metadata* metadata);
void print_sys_metadata(Metadata* metadata);

#endif