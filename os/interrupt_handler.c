#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "interrupt_handler.h"
#include "microkernel.h"
#include "filesystem/fat_functions.h"
#include "filesystem/file_dt.h"
#include "../registers.h"
#include "../internal_memory.h"


/**
 * @brief Opens a file with the address in the registers $g8 and $g9, and returns
 * a FATPtr* to the start of that file, or NULL if the file could not be opened
 * for any reason.
 * 
 * @param process The process opening the file
 * @param registers The system registers
 * @param ram The system RAM
 * @return A FATPtr* to the start of the file, or NULL if fails to open
 */
FATPtr* open_file_routine(Process* process, Register* registers, RAM* ram) {
    char* ascii_buffer = malloc(128 * sizeof(wchar_t));
    uint32_t addr_buffer = get_physical_from_logical_addr(process->id,
                            (get_register(10, registers).word_16 << 16) | 
                             get_register(9, registers).word_16);

    int index = 0;
    while (1) {
        ascii_buffer[index] = get_from_ram(ram, addr_buffer + index) & 0x0000FFFF;
        if (ascii_buffer[index] == '\0' || index >= 127)
            break;

        index++;
    }
    ascii_buffer[index] = '\0';
    
    // open the file
    FILE* image = fopen("os/filesystem/harddrive.img", "rb");
    if (image == NULL) {
        printf("Could not open hard drive - image could not be found!\n");
        return NULL;
    }
    
    Metadata* metadata = malloc(sizeof(Metadata));
    read_sys_metadata(image, metadata);    
    scan_FAT_into_RAM(image, metadata);
    free(metadata);

    fseek(image, 0x8800, SEEK_SET);
    FATPtr* fileptr = f_open(image, ascii_buffer);
    free(ascii_buffer);

    return fileptr;
}


/**
 * @brief Generates a 32-bit file descriptor with bits 0:11 being for the currently pointed
 * to cluster, 12:23 for the byte, 24 is 1 if read mode is on, 25 is for if write mode is
 * on, and 26:31 is for the file ID.
 * 
 * @param cluster The cluster being pointed to
 * @param offset The byte in the cluster being pointed to
 * @param read 1 if in 'r' or 'r+' mode, otherwise 0
 * @param write 1 if in 'w' or 'w+' mode, otherwise 0
 * @param id ID of the file
 * @return The 32 bit file descriptor
 */
uint32_t generate_file_descriptor(int cluster, int offset, int read, int write, int id) {
    uint32_t descriptor = 0;
    descriptor |= (id & 0x003F) << 26;
    descriptor |= (read & 0x0001) << 25;
    descriptor |= (write & 0x0001) << 24;
    descriptor |= (offset & 0x0FFF) << 12;
    descriptor |= cluster & 0x0FFF;

    return descriptor;
}


/*
Takes a code relating to an interrupt code to handle and acts appropriately. Currently, only 
program interrupts (a.k.a "syscalls") are handled, although system and external interrupts
will come eventually.

When reading or printing, the `printable` union should be used for integers and floats. This 
ensures they can be represented and printed properly.
*/
void handle_interrupt_code(unsigned short code, Register* registers, RAM* ram, Process* process) {
    // represent values that can be read or printed
    union {
        int i;
        float f;
    } printable;

    // set the upper 16 bits of the value to print to $g8, and the lower 16 bits to $g9
    printable.i = (get_register(9, registers).word_16 << 16) | get_register(10, registers).word_16;

    Register upper_bits, lower_bits;
    int32_t sbrk_pages_offset, int_buffer;
    uint32_t addr_buffer, offset, buffer_len, file_descriptor;
    wchar_t char_buffer, *str_buffer;
    Filedir* root_dirs;
    switch (code) {
        case 1:  // print signed int in $g8, $g9
            printf("%d\n", printable.i);
            break;

        case 2:  // print float in $g8, $g9
            printf("%f\n", printable.f);
            break;

        case 3:  // print str starting at addr in $ua, $g9, ending at next 0x0000 in RAM
            addr_buffer = (get_register(11, registers).word_16 << 16) | get_register(10, registers).word_16;
            offset = 0;
            while (get_from_ram(ram, addr_buffer + offset) != 0) {
                char_buffer = get_from_ram(ram, addr_buffer + offset);
                printf("%c", char_buffer);
                offset++;
            }

            printf("\n");
            
            break;

        case 4:  // read int
            scanf("%d", &printable.i);

            upper_bits.word_16 = (printable.i & 0xFFFF0000) >> 16;
            lower_bits.word_16 = printable.i & 0x0000FFFF;

            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 5:  // read float
            scanf("%f", &printable.f);

            upper_bits.word_16 = (printable.i & 0xFFFF0000) >> 16;
            lower_bits.word_16 = printable.i & 0x0000FFFF;

            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 6:  // read str into addr in $ua, $g9 of length in $g8
            // allocate size of buffer of characters to read
            buffer_len = get_register(9, registers).word_16;
            str_buffer = malloc(buffer_len * sizeof(wchar_t)); 

            // flush standard input and then get the string from the user
            fflush(stdin);
            fgetws(str_buffer, buffer_len, stdin);

            // remove final line feed
            for (unsigned int  i = buffer_len - 1; i > 0; i--) {
                if (str_buffer[i] == 0xA) {
                    str_buffer[i] = 0;
                    break;
                }
            }

            // add to ram
            addr_buffer = (get_register(11, registers).word_16 << 16) | get_register(10, registers).word_16;
            for (unsigned int i = 0; i < buffer_len; i++) {
                add_to_ram(ram, addr_buffer + i, str_buffer[i]);
            }

            free(str_buffer);
            break;

        case 7: // allocate heap memory, length in bytes of len at $g8, $g9
            addr_buffer = (get_register(10, registers).word_16 << 16) | get_register(9, registers).word_16;
            addr_buffer = allocate_memory(process->heap_root, addr_buffer);
            upper_bits.word_16 = (addr_buffer & 0xFFFF0000) >> 16;
            lower_bits.word_16 = addr_buffer & 0x0000FFFF;
            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 8:  // open file with name in str starting at addr in $g8, $g9, file descriptor put into $g8, $g9
            addr_buffer = open_file_routine(process, registers, ram)->start_sector;
            file_descriptor = generate_file_descriptor(addr_buffer, 0, 0, 0, 0);

            lower_bits.word_16 = file_descriptor & 0x0000FFFF;
            upper_bits.word_16 = (file_descriptor & 0xFFFF0000) >> 16;
            update_register(9, lower_bits, registers);
            update_register(10, upper_bits, registers);
            break;

        case 9: // close file
        case 10: // read $g7 bytes of data from HD pointer in $g8, $g9
        case 11: // write byte in $g7 to file pointer in $g8, $g9 (writes to buffer, OUT 0 writes to disk)
        case 12: // MIDI out, MIDI code in $g9
        case 13: // get system time into $g8 and $g9
        case 14: // sleep for milliseconds in $g8, $g9
            printf("Valid unimplemented syscall detected!\n");
            break;

        case 15: // set seed for random num generator to $g8, $g9
            printable.i = get_register(9, registers).word_16 << 16;
            printable.i |= get_register(10, registers).word_16;
            srand(printable.i);
            break;

        case 16: // get random integer into $g9
            printable.i = rand();
            upper_bits.word_16 = (printable.i >> 16) & 0xFFFF;
            lower_bits.word_16 = printable.i & 0xFFFF;
            
            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 17: // get random float into $g8, $g9
            printable.f = (float)rand() / (float)(RAND_MAX);
            upper_bits.word_16 = (printable.i >> 16) & 0xFFFF;
            lower_bits.word_16 = printable.i & 0xFFFF;

            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 18: // print integer in $g9 as hex
            printf("%X\n", printable.i);
            break;

        case 19: // print integer in $g8, $g9 as unsigned int
            printf("%u\n", printable.i);
            break;
        
        case 20: // "sbrk" syscall, increases heap into stack by $g8, $g9 pages (signed)
            sbrk_pages_offset = (get_register(10, registers).word_16 << 16) | get_register(9, registers).word_16;
            change_heap_size(sbrk_pages_offset, process);
            break;
        
        default:
            printf("Invalid syscall detected!");
            exit(-5);
    }
}
