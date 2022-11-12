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

#define DRIVE_SIZE      0x40000000 // total size of the hard drive
#define TABLE_BOUNDARY  0x80000    // The memory allocated to each table
#define FAT_SIZE        0x100000   // Boundary between main & mirror table

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

    // create 2 identical tables which mirror each others
    fseek(drive, FAT_SIZE, SEEK_SET);
    fprintf(drive, "\0");

    // initialise the rest of the drive to 0
    printf("Initialising hard drive...\n");
    char null_byte[1];
    null_byte[0] = '\0';
    for (int i = 0; i < 2048; i++) {
        fseek(drive, FAT_SIZE, SEEK_CUR);
        fwrite(null_byte, 1, 1, drive);
    }
    printf("Hard drive initialised!\n");

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


/*
Takes a reference to the file which represents the hard drive and a number of sectors to mark as
taken, plus a parameter to say if this is a directory or not. Directories always take up 1 sector
regardless of the sectors parameter.

The sectors taken are marked as 0x0000, 0x0100, 0x0200 ... 0xNNNN, and the final sector is 0xF8FF,
all of which are little-endian. Directory sectors are marked as 0xFFFF.

The return value is the pointer to the start of the first sector in the newly allocated file.
*/
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

    return FAT_SIZE + ((file_pos / 2) * 4096);
}


/*
Generates a sector header, which is formatted as follows and takes up a total of 32 bytes:
  - 24 bytes for filename encoded in ASCII
  - 4 bytes for the pointer to the start of the sector at the start of the file
  - 4 bytes for the pointer to the start of the next file (0xFF if this is the last sector)

The header is passed as a reference and is modified in place. 
*/
void generate_header(char* header, char* filename, long start_of_file_ptr, long next_sector_ptr) {
    if (strlen(filename) > 24) {
        printf("Filename %s is too long!", filename);
        exit(-10);
    }

    for (int i = 0; i < 24; i++) {
        header[i] = '\0';
    }
    strcpy(header, filename);
    
    for (int i = 24; i < 28; i++) {
        /* code */
        header[i] = start_of_file_ptr & 0xFF;
        start_of_file_ptr = start_of_file_ptr >> 8;
    }
    

    for (int i = 28; i < 32; i++) {
        header[i] = next_sector_ptr & 0xFF;
        next_sector_ptr = next_sector_ptr >> 8;
    }
}


/*
Moves the file pointer to the start of the first sector of the specified directory. Requires an absolute
path to work.

Works by splitting up the directory and then recursively iterating through the directories until it gets
to the final desired directory.
*/
void navigate_to_dir(FILE* drive, char* directory) {
    fseek(drive, 32, SEEK_CUR); // skip the 32 byte directory header
    if (directory[0] != '/') {
        printf("Invalid directory %s\n", directory);
        exit(-11);
    }

    // get next dir name, no not recur if no more directories to navigate
    char* next_dir_name = malloc(24);
    next_dir_name = strtok(directory, "/");
    if (next_dir_name == NULL)
        return;
    
    long parent_addr = ftell(drive);
    long next_addr = -1;
    char dir_name[24];
    int dir_index = 2; // current file being checked (skips the parent and own dir pointers) 
    while (next_addr != 0) {
        fseek(drive, parent_addr + (dir_index * 4), SEEK_SET);
        fread(&next_addr, sizeof(int), 1, drive);
        fseek(drive, next_addr, SEEK_SET);
        fread(dir_name, 1, 24, drive);

        if (strcmp(dir_name, next_dir_name) == 0) {
            navigate_to_dir(drive, directory);
            break;
        }

        dir_index++;
    }

    free(next_dir_name);
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

    long new_dir_ptr = mark_dir_in_FAT(drive, sectors, is_directory);
    if (new_dir_ptr < 0) {
        printf("Could not add file to FAT\n");
        return -1;
    }

    fseek(drive, new_dir_ptr, SEEK_SET);

    // 24 bytes for filename, 4 for chain start ptr, 4 for next sector ptr
    char header[32];
    long next_ptr;
    for (int i = 0; i < sectors; i++) {
        next_ptr = i == sectors - 1 ? -1 : new_dir_ptr + ((i + 1) * 4096);

        generate_header(header, filename, new_dir_ptr, next_ptr);
        fwrite(header, 1, 32, drive);
        fseek(drive, 4096 - 32, SEEK_CUR);
    }

    if (strcmp(directory, "") != 0) {
        fseek(drive, FAT_SIZE, SEEK_SET); // skip header
        navigate_to_dir(drive, directory);

        int pointer;
        fread(&pointer, 4, 1, drive);
        while (pointer != 0) {
            fread(&pointer, 4, 1, drive);
        }
        fseek(drive, -4, SEEK_CUR); // move back to start of section just read
        
        fwrite(&new_dir_ptr, 4, 1, drive);
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
