#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "sys_meta.h"
#include "file_meta.h"
#include "fat_functions.h"
#include "file_dt.h"


uint16_t* FAT = NULL;


// /**
//  * @brief Seeks the start of the FAT, and then loads the contents into the passed pointer of 
//  * 16-bit integers.
//  * 
//  * @param image Image of the hard drive
//  * @param metadata Metadata about the hard drive
//  */
// void scan_FAT_into_RAM(FILE* image, Metadata* metadata) {
//     const int FAT_start_addr = metadata->BPB_BytesPerSec * metadata->BPB_RsvdSecCnt;
//     const int FAT_size = metadata->BPB_BytesPerSec * metadata->BPB_FATz16;
//     FAT = malloc(FAT_size);
//     if (FAT == NULL) {
//         printf("ERROR: COULD NOT ALLOCATE SUFFICIENT MEMORY FOR FAT!\n");
//         exit(-1);
//     }

//     fseek(image, FAT_start_addr, SEEK_SET);
//     fread(FAT, sizeof(uint16_t), FAT_size / 2, image);
// }


// int main() {
//    // Get filesystem metadata
//     FILE* image = fopen("fat16.img", "rb");
//     Metadata* metadata = malloc(sizeof(Metadata));
//     read_sys_metadata(image, metadata);
//     print_sys_metadata(metadata);

//     // Load the FAT into RAM
//     scan_FAT_into_RAM(image, metadata);  

//     // get the root dir
//     int num_root_dirs;
//     const long root_dir_sector = metadata->BPB_RsvdSecCnt + (metadata->BPB_NumFATs * metadata->BPB_FATz16);
//     Filedir* root_dirs = iterate_directory(image, root_dir_sector * metadata->BPB_BytesPerSec, &num_root_dirs);

//     // print out the root dir
//     printf("Start Cluster\tLast Modified\t\tFlags\t\tLength\t\tName\n");
//     for (int i = 0; i < num_root_dirs; i++) {
//         print_directory(&root_dirs[i]);
//     }
    
//     // char filename[50] = "/man/man2/./chmod.2\0";
//     // FATPtr* file = f_open(image, filename);
//     // if (file == NULL) {
//     //     printf("FAIL\n");
//     //     exit(-1);
//     // } 

//     // char *buffer = malloc(1501);
//     // if (buffer == NULL)
//     //     exit(-1);

//     // buffer[1500] = '\0';

//     // f_read(file, 1500, buffer);
//     // printf("%s\n", buffer);
//     // f_close(file);

//     // printf("END\n");
//     fseek(image, 0x8800, SEEK_SET);
//     command_line(image);
// }
