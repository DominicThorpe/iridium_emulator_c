#ifndef FILEDT
#define FILEDT

#include "stdint.h"
#include "file_meta.h"


// Stores a date and a time a directory was created or last written or read
// TODO: integrate into the initialisation of the file metadata
typedef struct datetime {
    uint16_t year   : 7;
    uint16_t month  : 4;
    uint16_t day    : 5;

    uint32_t hour   : 5;
    uint32_t minute : 6;
    uint32_t second : 5;
    uint32_t subsec : 7; // Not used except for creation time
} DateTime;

void create_date_time(DateTime* dt, Filedir* filedir);


#endif