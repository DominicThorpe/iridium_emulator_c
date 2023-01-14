#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "file_meta.h"
#include "fat_functions.h"
#include "sys_meta.h"
#include "file_dt.h"

#define FALSE 0
#define TRUE  1


uint16_t* FAT = NULL;


long get_addr_from_cluster(long cluster_num, Metadata* metadata) {
    const int sector_offset_for_root = 4;
    const long root_dir_addr = ((metadata->BPB_RsvdSecCnt + (metadata->BPB_NumFATs * metadata->BPB_FATz16)) 
                        * metadata->BPB_BytesPerSec);
    long new_addr = (((cluster_num + 2 + sector_offset_for_root) * metadata->BPB_SecPerClus) * 
                        metadata->BPB_BytesPerSec + root_dir_addr);
}


/**
 * @brief Seeks the start of the FAT, and then loads the contents into the passed pointer of 16-bit integers
 * 
 * @param image Image of the harddrive
 * @param metadata Filesystem metadata
 */
void scan_FAT_into_RAM(FILE* image, Metadata* metadata) {
    const int FAT_start_addr = metadata->BPB_BytesPerSec * metadata->BPB_RsvdSecCnt;
    const int FAT_size = metadata->BPB_BytesPerSec * metadata->BPB_FATz16;
    FAT = malloc(FAT_size);
    if (FAT == NULL) {
        printf("ERROR: COULD NOT ALLOCATE SUFFICIENT MEMORY FOR FAT!\n");
        exit(-1);
    }

    fseek(image, FAT_start_addr, SEEK_SET);
    fread(FAT, sizeof(uint16_t), FAT_size / 2, image);
}


/**
 * @brief Initialises the harddrive by scanning the FAT into RAM and getting the metadata
 * before returning a pointer to the harddrive image.
 * 
 * @param metadata Pointer to put the harddrive metadata into
 * @return Pointer to the harddrive image file 
 */
FILE* init_harddrive(Metadata* metadata) {
    FILE* image = fopen("os/filesystem/harddrive.img", "rb");
    metadata = malloc(sizeof(Metadata));
    read_sys_metadata(image, metadata);

    scan_FAT_into_RAM(image, metadata);
    fseek(image, 0x8800, SEEK_SET);

    return image;
}


/*
Goes into the given directory, gets all the files in it 1-by-1, assigns long filenames to them if
neccessary, and prints them using print_directory
TODO: decouple the print dir and iterate dir functions
*/
Filedir* iterate_directory(FILE* image, int addr, int* num_dirs) {
    fseek(image, addr, SEEK_SET);
    Filedir fdir;
    Filedir* directories = malloc(sizeof(Filedir) * 16);
    int index = 0;

    char filename[128];
    short read_filename = FALSE;
    while (1) {
        read_dir_metadata(image, &fdir);

        // get the long file name of the file if applicable
        if ((fdir.DIR_Attr & 0b00111111) == 0b00001111) {
            fseek(image, -32, SEEK_CUR);
            read_long_filename(image, filename);
            read_filename = TRUE;
            continue;
        }

        else if ((fdir.DIR_FileSize == 0 && fdir.DIR_WrtDate == 0 && fdir.DIR_WrtTime == 0) || ftell(image) > addr + 0x800)
            break;
        
        if (read_filename == TRUE) {
            free(fdir.DIR_Name);
            fdir.DIR_Name = malloc(strlen(filename) + 1); // memory fails to malloc
            strcpy(fdir.DIR_Name, filename);
        }

        read_filename = FALSE;
        strcpy(filename, "\0");

        // if running out of space in the directories array, add more
        if (index % 16 == 0) {
            directories = ( Filedir* ) realloc(directories, sizeof(Filedir) * (index + 16));
            if (directories == NULL)
                printf("Realloc not ok\n");
        }
        directories[index] = fdir;
        index++;
    }

    *num_dirs = index;
    return directories;
}


// Creates a new FATPtr struct from the Filedir struct and system metadata provided.
FATPtr* create_new_FAT_ptr(Filedir* filedir, Metadata* sys_metadata, int root) {
    FATPtr* fatptr = malloc(sizeof(FATPtr));
    FILE* fileptr = fopen("fat16.img", "r");
    
    // go to the correct addr according to cluster if not root, else go to 0x8800
    int cluster_num = (long)filedir->DIR_FstClusHI << 16 | filedir->DIR_FstClusLO;
    long new_addr = root == 0 ? get_addr_from_cluster(cluster_num, sys_metadata) : 0x8800;
    fseek(fileptr, new_addr, SEEK_SET);

    // create the fat pointer to be returned
    fatptr->fileptr = fileptr;
    fatptr->sys_context = sys_metadata;
    fatptr->file_context = filedir;
    fatptr->sector_num = cluster_num;
    fatptr->start_sector = cluster_num;
    fatptr->next_sector = FAT[cluster_num];
    fatptr->current_pos = 0;

    return fatptr;
}


/*
Goes to the root directory and uses the strtok function to split the directory passed up into its
different sections, then goes through each of them until it gets to the correct directory, then
returns the file pointer. 
*/
FATPtr* f_open(FILE* image, char* dir) {
    long current_pos = ftell(image);
    Metadata* sys_metadata = malloc(sizeof(Metadata));
    read_sys_metadata(image, sys_metadata);
    fseek(image, current_pos, SEEK_SET);

    Filedir* filedir = malloc(sizeof(Filedir));
    if ((dir[0] == '/' && strlen(dir) == 1) || strlen(dir) == 0) {
        const long root_dir_addr = ((sys_metadata->BPB_RsvdSecCnt + (sys_metadata->BPB_NumFATs * sys_metadata->BPB_FATz16)) 
                        * sys_metadata->BPB_BytesPerSec);

        filedir->DIR_FstClusHI = 0xFFFF;
        filedir->DIR_FstClusLO = 0xFFFA;
        FATPtr* root_ptr = create_new_FAT_ptr(filedir, sys_metadata, 1);
        fseek(root_ptr->fileptr, root_dir_addr, SEEK_SET);

        free(filedir);
        return root_ptr;
    }

    char* dir_copy = malloc(strlen(dir));
    strcpy(dir_copy, dir);
    char* subdir;
    int num_subdirs;
    subdir = strtok(dir_copy, "/");

    Filedir* root_dirs = iterate_directory(image, ftell(image), &num_subdirs);
    for (int i = 0; i < num_subdirs; i++) {
        filedir = &root_dirs[i];

        // skip if the directory does not match the goal and validate the directory is not a long file name
        if (strcmp(filedir->DIR_Name, subdir) != 0 || (filedir->DIR_Attr & 0b00111111) == 0b00001111)
            continue;
        else
            break;
    }

    if (strcmp(filedir->DIR_Name, subdir) != 0) {
        printf("WARNING: DID NOT FIND FILE: %s!\n", dir);
        return NULL;
    }

    // if this is not the final directory, use recursion to go to the next level down
    if (dir[0] == '/') 
        dir++;

    char* remaining_dir = strchr(dir, '/');
    if (strlen(dir) > 0 && dir[strlen(dir) - 1] != '/' && remaining_dir != NULL) {
        free(filedir);
        free(dir_copy);

        long next_cluster = ((long)filedir->DIR_FstClusHI << 16) | filedir->DIR_FstClusLO;
        fseek(image, get_addr_from_cluster(next_cluster, sys_metadata), SEEK_SET);
        return f_open(image, remaining_dir + 1);
    }

    // create a FATPtr struct to hold the details of the file and then return it
    FATPtr* fatptr = create_new_FAT_ptr(filedir, sys_metadata, 0);
    free(filedir);
    free(dir_copy);

    return fatptr;
}


/*
Goes from a given position in the file "whence" where:
    0 - the start of the file
    1 - the current position in the file

And moves the file pointer to that position.
TODO: maybe add SEEK_END?
*/
void f_seek(FATPtr* fileptr, long offset, short whence) {
    if (offset == 0)
        return;

    // Start of file, go to start of file and seek thence using recursion
    if (whence == 0) { 
        fileptr->sector_num = fileptr->start_sector;
        fileptr->current_pos = 0;
        f_seek(fileptr, offset, 1);
    } 
    
    // Current pos in file, iterate through sectors until no sectors left, then go through remainder
    // and end recursion.
    else if (whence == 1) {
        if (fileptr->current_pos + offset > 0x800) {
            fileptr->sector_num = fileptr->next_sector;
            fileptr->next_sector = FAT[fileptr->sector_num];
            f_seek(fileptr, offset - 0x800, whence);
        } else {
            fileptr->current_pos = fileptr->current_pos + offset;
            long new_addr = get_addr_from_cluster(fileptr->sector_num, fileptr->sys_context);
            fseek(fileptr->fileptr, new_addr + fileptr->current_pos, SEEK_SET);
        }
    }
    
    else {
        printf("%d is not a valid whence!\n");
        exit(-1);
    }
}


/*
Reads "vol" bytes from the current position onwards in the file into the buffer and moves the file 
pointer that many bytes onwards.
*/
void f_read(FATPtr* fileptr, long bytes, char* buffer) {
    // if the file is a directory or a volume, do not allow it to be opened
    if ((fileptr->file_context->DIR_Attr & 0b011000) != 0) {
        printf("CANNOT READ DIRECTORY OR VOLUME: %s, %X!\n", fileptr->file_context->DIR_Name, fileptr->file_context->DIR_Attr);
        return;
    }

    // long new_addr = get_addr_from_cluster(fileptr->sector_num, fileptr->sys_context);
    // fseek(fileptr->fileptr, new_addr + fileptr->current_pos, SEEK_SET);

    long index = 0;
    while (bytes > 0) {
        if (fileptr->current_pos + bytes >= 0x800 && fileptr->sector_num < 0xFFF8) {
            fread(buffer + index, 1, 0x800 - fileptr->current_pos, fileptr->fileptr);
            bytes -= 0x800 - fileptr->current_pos;
            index += 0x800 - fileptr->current_pos;

            fileptr->sector_num = fileptr->next_sector;
            fileptr->next_sector = fileptr->sector_num < 0xFFF8 ? FAT[fileptr->sector_num] : -1;
            fileptr->current_pos = 0;
        } else {
            fread(buffer + index, 1, bytes, fileptr->fileptr);
            
            fileptr->current_pos += bytes;
            index += bytes;
            bytes = 0;
        }
    }

    printf("SectorB: %X.%X, %X\n", fileptr->sector_num, fileptr->current_pos, ftell(fileptr->fileptr));
}


// Takes a file pointer and frees it, including the internal C FILE* pointer
void f_close(FATPtr* fileptr) {
    fclose(fileptr->fileptr);
    free(fileptr);
}
