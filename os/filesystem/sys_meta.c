#include <stdio.h>
#include <string.h>
#include "sys_meta.h"


/*
Open the FAT16 image at the given pointer and read its into a metadata into the Metadata struct, and
the return a pointer to that struct.
*/
void read_sys_metadata(FILE* image, Metadata* metadata) {
    rewind(image); // ensure at start of file
    fseek(image, 11, SEEK_SET);

    fread(&metadata->BPB_BytesPerSec, 2, 1, image);
    fread(&metadata->BPB_SecPerClus, 1, 1, image);
    fread(&metadata->BPB_RsvdSecCnt, 2, 1, image);
    fread(&metadata->BPB_NumFATs, 1, 1, image);
    fread(&metadata->BPB_RootEntCnt, 2, 1, image);
    fread(&metadata->BPB_TotSec16, 2, 1, image);
    fseek(image, 1, SEEK_CUR);
    fread(&metadata->BPB_FATz16, 2, 1, image);
    fseek(image, 8, SEEK_CUR);
    fread(&metadata->BPB_TotSec32, 4, 1, image);
    fseek(image, 7, SEEK_CUR);
    fread(metadata->BS_VolLab, 1, 11, image);
}


//Prints out the metadata stored in the given Metadata struct
void print_sys_metadata(Metadata* metadata) {
    char BS_VolLab[11];
    strcpy(BS_VolLab, metadata->BS_VolLab);
    BS_VolLab[10] = '\0';

    printf(
        "Bytes per sector: %u\nSectors per cluster: %u\nReserved sector count: %u\nFAT count %u\n",
        (unsigned int)metadata->BPB_BytesPerSec, 
        (unsigned int)metadata->BPB_SecPerClus, 
        (unsigned int)metadata->BPB_RsvdSecCnt,
        (unsigned int)metadata->BPB_NumFATs
    );
    printf(
        "Size of root: %u\nTotal 16 sectors: %u\nSectors in FAT: %u\nTotal 32 sectors: %u\nVol lab: %s\n",
        (unsigned int)metadata->BPB_RootEntCnt, 
        (unsigned int)metadata->BPB_TotSec16, 
        (unsigned int)metadata->BPB_FATz16, 
        (unsigned int)metadata->BPB_TotSec32, 
        BS_VolLab
    );
}
