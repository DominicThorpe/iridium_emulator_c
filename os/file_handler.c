/*
The Iridium system assumes the use of the FAT-16 (File Allocation Table) file system to format data 
on the hard drives. This module implements this system, however, it does not make use of any initial
reserved memory sectors in this emulation, largely because the BIOS is not stored on disc and may
not be used at all, depending on how the implementation goes.

NOTE: This implementation uses big-endian values, despite FAT usually being little-endian. This is so
that the hard drive has the same format as the main system, as it is assumed that the drives are
internal to the system.
*/

#include <stdlib.h>
#include <stdio.h>

#define FAT_SIZE 4194304 // The memory allocated to each table


/*
Creates a new FAT-16 formatted drive with 16GB of capacity, deleting/overwriting any it finds in the
same directory.

In the beginning, all cells in the map are set to 0, which means that it is unallocated. The cells
are 4098 bytes (4KiB) in size, so for 16 GiB, there must be 4MiB of memory allocated to the tables
to store it all.
*/
void init_formatted_drive(char* dirname) {
    FILE* drive;
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
*/
int create_file(FILE* drive, char* filename, char* directory, int sectors) {

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
