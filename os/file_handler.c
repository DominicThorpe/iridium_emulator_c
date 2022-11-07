/*
The Iridium system assumes the use of the FAT-16 (File Allocation Table) file system to format data 
on the hard drives. This module implements this system, however, it does not make use of any initial
reserved memory sectors in this emulation, largely because the BIOS is not stored on disc and may
not be used at all, depending on how the implementation goes.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "file_handler.h"

#define FAT_SIZE 4194304 // The memory allocated to each table


/*
Creates a new FAT-16 formatted drive with 16GB of capacity, deleting/overwriting any it finds in the
same directory.

In the beginning, all cells in the map are set to 0, which means that it is unallocated. The cells
are 4098 bytes (4KiB) in size, so for 16 GiB, there must be 4MiB of memory allocated to the tables
to store it all.
*/
void init_formatted_drive(char* dirname) {
    // do nothing if drive already exists
    FILE* drive;
    if ((drive = fopen(dirname, "r"))) {
        fclose(drive);
        return;
    }
    
    drive = fopen(dirname, "w");

    // init all cells to 0
    char* bytes_to_write = malloc(FAT_SIZE);
    if (bytes_to_write == NULL) {
        printf("Insufficient resources available to initialise new drive\n");
        exit(-6);
    }

    for (long i = 0; i < FAT_SIZE; i++) {
        bytes_to_write[i] = '\0';
    }
    
    // create 2 identical tables which mirror each others
    fwrite(bytes_to_write, sizeof(char), FAT_SIZE, drive);
    fwrite(bytes_to_write, sizeof(char), FAT_SIZE, drive);

    fclose(drive);
    free(drive);
}


/*
Takes a filename and a directory to put it in. Filename is treated as a directory if it has a trailing
slash '/'. This is added to the relevant directory as a file or subdirectory with the number of sectors
specified allocated to it.

The process for creating a file is as follows:
  - Locate a run of sectors on the drive which are empty and of sufficient size to hold the file
  - Mark those sectors as in use
  - Add end of chain marker
  - Add the file header to the first sector, and the footer to each other sector
*/
int create_file(FILE* drive, char* filename, char* directory, int sectors) {
    char* file_alloc_table = malloc(sizeof(char) * FAT_SIZE);
    if (file_alloc_table == NULL) {
        printf("Insufficient resources available to read drive\n");
        exit(-6);
    }

    fseek(drive, 0L, SEEK_SET);
    fread(file_alloc_table, sizeof(char), FAT_SIZE, drive);

    // find sufficiently sized region of the hard drive for the number of sectors needed whilst copying 
    // the file into the mirror file.
    char* buffer = malloc(sizeof(char) * sectors);
    if (buffer == NULL) {
        printf("Insufficient resources available to read drive\n");
        exit(-6);
    }

    long file_pos = 0;
    int run_length = 0;
    for (long i = 0; i < FAT_SIZE; i++) {
        if (run_length >= sectors)
            break;
        
        if (file_alloc_table[i] == '\0')
            run_length++;
        else {
            run_length = 0;
            file_pos = i+1;
        }
    }

    // fail if the file could not be created due to insufficient free space
    if (run_length < sectors) {
        printf("Could not find sufficient disc space\n");
        remove("hd_mirror.drive");

        return -1;
    }

    char* table_val = malloc(2);
    for (int i = 0; i < sectors; i++) {
        if (i == sectors - 1) {
            table_val[0] = (char)0xF8;
            table_val[1] = (char)0xFF;
        } else {
            table_val[0] = (char)(i + 1) & 0xFFFF;
            table_val[1] = (char)((i + 1) >> 16) & 0xFFFF;
        }

        fseek(drive, file_pos + (i * 2), SEEK_SET);
        fwrite(table_val, sizeof(char), 2, drive);
    }
    free(table_val);

        
    // handle create directory
    if (filename[strlen(filename) - 1] == '/') {
        printf("Found dir\n");
    }

    // handle create file
    printf("Found file\n");
    free(buffer);

    return 0;
}


/*
Read the given number of subsectors (512 bytes) from the file, offset by the given number, into the
given buffer.
*/
void read_file(FILE* drive, char* directory, long offset, long subsectors) {

}


/*

*/
void write_file(FILE* drive, char* directory, long offset, char* data) {

}


/*

*/
void append_to_file(FILE* drive, char* directory, long offset, char* data) {

}


/*

*/
void delete_file(FILE* drive, char* directory) {

}
