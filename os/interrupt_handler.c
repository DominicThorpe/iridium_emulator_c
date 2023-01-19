#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "interrupt_handler.h"
#include "microkernel.h"
#include "filesystem/fat_functions.h"
#include "../registers.h"
#include "../internal_memory.h"



/**
 * @brief Takes a code relating to an interrupt code to handle and acts appropriately. Currently, only 
 * program interrupts (a.k.a "syscalls") are handled, although system and external interrupts
 * will come eventually.
 * 
 * @note When reading or printing, the `printable` union should be used for integers and floats. This 
 * ensures they can be represented and printed properly.
 * 
 * @param code The interrupt code
 * @param registers The system registers
 * @param ram The system RAM
 * @param process The process calling the interrupt
 * @param hd_img File pointer to the image of the harddrive
 */
void handle_interrupt_code(unsigned short code, Register* registers, RAM* ram, Process* process, FILE* hd_img) {
    // represent values that can be read or printed
    union {
        int i;
        float f;
    } printable;

    // set the upper 16 bits of the value to print to $g8, and the lower 16 bits to $g9
    printable.i = (get_register(9, registers).word_16 << 16) | get_register(10, registers).word_16;

    Register upper_bits, lower_bits;
    int32_t sbrk_pages_offset;
    uint32_t addr_to_get, offset, buffer_len;
    wchar_t char_to_print, *str_input_buffer;
    switch (code) {
        case 1:  // print signed int in $g8, $g9
            printf("%d\n", printable.i);
            break;

        case 2:  // print float in $g8, $g9
            printf("%f\n", printable.f);
            break;

        case 3:  // print str starting at addr in $ua, $g9, ending at next 0x0000 in RAM
            addr_to_get = (get_register(11, registers).word_16 << 16) | get_register(10, registers).word_16;
            offset = 0;
            while (get_from_ram(ram, addr_to_get + offset) != 0) {
                char_to_print = get_from_ram(ram, addr_to_get + offset);
                printf("%c", char_to_print);
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
            str_input_buffer = malloc(buffer_len * sizeof(wchar_t)); 

            // flush standard input and then get the string from the user
            fflush(stdin);
            fgetws(str_input_buffer, buffer_len, stdin);

            // remove final line feed
            for (unsigned int  i = buffer_len - 1; i > 0; i--) {
                if (str_input_buffer[i] == 0xA) {
                    str_input_buffer[i] = 0;
                    break;
                }
            }

            // add to ram
            addr_to_get = (get_register(11, registers).word_16 << 16) | get_register(10, registers).word_16;
            for (unsigned int i = 0; i < buffer_len; i++) {
                add_to_ram(ram, addr_to_get + i, str_input_buffer[i]);
            }

            free(str_input_buffer);
            break;

        case 7: // allocate heap memory, length in bytes of len at $g8, $g9
            addr_to_get = (get_register(10, registers).word_16 << 16) | get_register(9, registers).word_16;
            addr_to_get = allocate_memory(process->heap_root, addr_to_get);
            upper_bits.word_16 = (addr_to_get & 0xFFFF0000) >> 16;
            lower_bits.word_16 = addr_to_get & 0x0000FFFF;
            update_register(9, upper_bits, registers);
            update_register(10, lower_bits, registers);
            break;

        case 8: { // open file with name in str starting at addr in $g8, $g9, puts id of open file in $g9
            char buffer[100];
            uint32_t address = (get_register(10, registers).word_16 << 16) | get_register(9, registers).word_16;
            for (int i = 0; i < sizeof(buffer); i++) {
                buffer[i] = get_from_ram(ram, address + i) & 0x0000FFFF;
                if (buffer[i] == '\0')
                    break;
            }

            FATPtr* file_ptr = f_open(hd_img, buffer);
            Register id;
            id.word_16 = file_ptr->id;
            update_register(10, id, registers);
            break;
        } 

        case 9:  // read no. bytes in $g8 from file id in $g9 into buffer at $ua, $g7
        case 10: // write no. bytes in $g8 into file id in $g9 into buffer at $ua, $g7
        case 11: // close file
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
