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

#define DRIVE_SIZE      1073741824  // total size of the hard drive
#define TABLE_BOUNDARY  524288      // The memory allocated to each table
#define FAT_SIZE        1048576     // Boundary between main & mirror table

#define TRUE 1
#define FALSE 0


/*
Creates a new FAT-16 formatted drive with 2GB of capacity, deleting/overwriting any it finds in the
same directory.

In the beginning, all cells in the map are set to 0, which means that it is unallocated. The cells
are 4098 bytes (4KiB) in size, so for 512KiB, there must be 4MiB of memory allocated to the tables
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

    // initialise the rest of the drive to 0
    long space_left_to_alloc = DRIVE_SIZE - FAT_SIZE;
    for (int i = 0; i < 1023; i++) {
        if (fwrite(bytes_to_write, sizeof(char), FAT_SIZE, drive) != 0)
            printf("FAIL\n");
    }

    create_file(drive, "/", "", 1);

    fclose(drive);
}


/*
Takes a filename and checks it is valid based on the following requirements:
  - contains only ASCII characters a-z, A-Z, 0-9, space, -, and _
  - ends with a dot and some characters a-z
  - is not more than 24 characters
*/
int validate_filename(char* filename) {
    if (strlen(filename) > 24 || strlen(filename) == 0)
        return -1;

    // root directory "/" is valid
    if (strcmp(filename, "/") == 0)
        return 0;

    char* name_to_check = filename;
    if (filename[strlen(filename) - 1] == '/')
        name_to_check[strlen(name_to_check) - 1] = '\0';

    int found_dot = FALSE;
    int found_type = FALSE;
    for (int i = 0; i < strlen(name_to_check); i++) {
        if (name_to_check[i] == '.') {
            found_dot = TRUE;
            continue;
        } else if (found_dot == TRUE)
            found_type = TRUE;
        
        if (
            name_to_check[i] != 0x20 && // allow space
            name_to_check[i] != 0x2D && // allow -
            name_to_check[i] != 0x5F && // allow _
            !(name_to_check[i] > 0x40 && name_to_check[i] < 0x5B) && // allow A-Z
            !(name_to_check[i] > 0x60 && name_to_check[i] < 0x7B) && // allow a-z
            !(name_to_check[i] > 0x2F && name_to_check[i] < 0x3A) // allow 0-9
        ) return -1;
    }

    // must have valid file extension or be a directory
    if (
        (found_type == FALSE && !(filename[strlen(filename) - 1] == '/')) || 
        (found_dot == TRUE && filename[strlen(filename) - 1] == '/')
    ) return -1;

    return 0;
}


int mark_dir_in_FAT(FILE* drive, int sectors, int is_directory) {
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
        if (is_directory == TRUE) {
            table_val[0] = (char)0xFF;
            table_val[1] = (char)0xFF;
        } else if (i == sectors - 1) {
            table_val[0] = (char)0xF8;
            table_val[1] = (char)0xFF;
        } else {
            table_val[0] = (char)(i + 1) & 0xFFFF;
            table_val[1] = (char)((i + 1) >> 16) & 0xFFFF;
        }

        // write to main table
        fseek(drive, file_pos + (i * 2), SEEK_SET);
        fwrite(table_val, sizeof(char), 2, drive);

        // write to mirror table
        fseek(drive, TABLE_BOUNDARY + file_pos + (i * 2), SEEK_SET);
        fwrite(table_val, sizeof(char), 2, drive);
    }
    
    free(table_val);
    free(buffer);

    return 0;
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

Note that when creating a directory, the directory will always take up 1 sector, regardless of how many
were passed into the sectors parameter.
*/
int create_file(FILE* drive, char* filename, char* directory, int sectors) {
    if (validate_filename(filename) != 0) {
        printf("'%s' is an invalid filename\n", filename);
        return -1;
    }
    
    // a directory is always 1 sector
    int is_directory = FALSE;
    if (filename[strlen(filename) - 1] == '/') {
        sectors = 1;
        is_directory = TRUE;
    }

    if (mark_dir_in_FAT(drive, sectors, is_directory) != 0) {
        printf("Could not add file to FAT\n");
        return -1;
    }
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
