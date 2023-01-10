#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "file_meta.h"


/**
 * @brief Convert a single status flag (chosen by the mask) of a directory to its string representation
 * 
 * @example convert_flag_to_str(flags, 0b00100000, buffer, &ptr, 'A');
 * 
 * @param flags The flags of the directory
 * @param mask The mask for the bit
 * @param buffer Buffer to put the flag character into
 * @param ptr Pointer to the index in the string to insert the flag character into
 * @param label The character to represent the flag
 */
void convert_flag_to_str(uint8_t flags, uint8_t mask, char buffer[13], int* ptr, char label) {
    if ((flags & mask) == 0) {
        buffer[*ptr] = '~';
        ( *ptr )++;
    }
    buffer[*ptr] = label;
    ( *ptr )++;
}


/**
 * @brief Puts the string-representation of the passed status flags into the given string pointer
 * 
 * @param buffer Destination of the flags string
 * @param flags The flags of the directory
 */
void get_flags_str(char buffer[13], uint8_t flags) {
    int ptr = 0;
    
    convert_flag_to_str(flags, 0b00100000, buffer, &ptr, 'A');
    convert_flag_to_str(flags, 0b00010000, buffer, &ptr, 'D');
    convert_flag_to_str(flags, 0b00001000, buffer, &ptr, 'V');
    convert_flag_to_str(flags, 0b00000100, buffer, &ptr, 'S');
    convert_flag_to_str(flags, 0b00000010, buffer, &ptr, 'H');
    convert_flag_to_str(flags, 0b00000001, buffer, &ptr, 'R');

    buffer[ptr] = '\0';
}


/**
 * @brief Takes a file's metadata and prints it in the format shown in the spec
 * 
 * @param fdir The directory to print
 */
void print_directory(Filedir* fdir) {
    DateTime* dt = malloc(sizeof(DateTime));
    char status_str[13];

    get_flags_str(status_str, fdir->DIR_Attr);
    create_date_time(dt, fdir);
    printf(
        "0x%04X%04X\t%02d:%02d:%02d %04d/%02d/%02d\t%02s\t%d\t\t%s\n",
        fdir->DIR_FstClusHI, fdir->DIR_FstClusLO,
        dt->hour, dt->minute, dt->second, dt->year + 1980, dt->month, dt->day,
        status_str,
        fdir->DIR_FileSize,
        fdir->DIR_Name
    );

    free(dt);
}


/**
 * @brief Open the directory at the given cluster and read its metadata into a Filedir struct passed as a pointer.
 * 
 * @param image The hard drive image to read the metadata of
 * @param filedir The directory metadata object to put the metadata into
 */
void read_dir_metadata(FILE* image, Filedir* filedir) {
    char dirname[ 12 ];
    fread(dirname, 1, 11, image);
    for (short i = 0; i < 12; i++) {
        if (dirname[i] == 0x20) {
            dirname[i] = '\0';
            break;
        }
    }
    
    dirname[11] = '\0';

    filedir->DIR_Name = malloc(12);
    strcpy(filedir->DIR_Name, dirname);
    fread(&filedir->DIR_Attr, 1, 1, image);
    fseek(image, 6, SEEK_CUR);
    fread(&filedir->DIR_LstAccDate, 2, 1, image);
    fread(&filedir->DIR_FstClusHI, 2, 1, image);
    fread(&filedir->DIR_WrtTime, 2, 1, image);
    fread(&filedir->DIR_WrtDate, 2, 1, image);
    fread(&filedir->DIR_FstClusLO, 2, 1, image);
    fread(&filedir->DIR_FileSize, 4, 1, image);
}


/**
 * @brief Takes a sector in the FAT and inserts every sector in that entry's chain of sectors, in order, 
 * into the sectors list, then returns the total number of sectors.
 * 
 * @param image The hard drive image
 * @param start_sector The sector to start reading from
 * @param sectors The array of sector nums to put the results into
 * @return The total number of sectors
 */
int list_sectors_by_start_num(FILE* image, uint16_t start_sector, uint16_t* sectors) {
    start_sector += 2;

    int sector_as_addr = (start_sector * 2) + 0x800;
    fseek(image, sector_as_addr, SEEK_SET);
    uint16_t result;

    int index = 0;
    do {
        fread(&result, 2, 1, image);
        if (result < 0xFFF8)
            fseek(image, (result * 2) + 0x800, SEEK_SET);

        sectors[index] = result;
        index++;
    } while (result < 0xFFF8);
    
    sectors[index-1] = (ftell(image) - 0x800) / 2;
    return index;
}


/**
 * @brief Assumes that the file has already been read by the read_dir_metadata() function and has been rewound 
 * to the start of the entry in the directory. Then reads the long name_data for the entry into the string
 * provided.
 * 
 * @param image The image of the hard drive
 * @param long_name The string to read the filename into
 */
void read_long_filename(FILE* image, char* long_name) {
    uint16_t name_data[13];
    char name[14];
    name[13] = '\0';

    fseek(image, 1, SEEK_CUR); // skip LDIR_Ord
    fread(name_data, sizeof(uint16_t), 5, image);
    fseek(image, 3, SEEK_CUR); // skip LDIR_Attr, LDIR_Type, LDIR_Chksum
    fread(name_data + 5, sizeof(uint16_t), 6, image);
    fseek(image, 2, SEEK_CUR); // skip LDIR_FstClusLO
    fread(name_data + 11, sizeof(uint16_t), 2, image);

    for (int i = 0; i < 13; i++) {
        name[i] = (char)name_data[i];
    }    

    memmove(long_name + strlen(name), long_name, strlen(long_name) + 1);
    memcpy(long_name, name, strlen(name));
}
