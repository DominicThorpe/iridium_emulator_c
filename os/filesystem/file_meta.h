#ifndef FILEMETA
#define FILEMETA

#include <stdint.h>
#include <stdio.h>


// Designed to store the header of a directory
typedef struct filedir {
    char*       DIR_Name;        // String with null terminator added by program
    uint8_t     DIR_Attr;        // File attributes
    uint16_t    DIR_LstAccDate;  // Date of last read or write
    uint16_t    DIR_FstClusHI;   // Top 16 bits file's 1st cluster
    uint16_t    DIR_WrtTime;     // Time of last write
    uint16_t    DIR_WrtDate;     // Date of last write
    uint16_t    DIR_FstClusLO;   // Lower 16 bits file's 1st cluster
    uint32_t    DIR_FileSize;    // File size in bytes
} Filedir;


void read_dir_metadata(FILE* image, Filedir* filedir);
int list_sectors_by_start_num(FILE* image, uint16_t start_sector, uint16_t* sectors);
void read_long_filename(FILE* image, char* name);


#endif