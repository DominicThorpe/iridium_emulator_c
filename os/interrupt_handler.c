#include "interrupt_handler.h"
#include "../registers.h"
#include "../internal_memory.h"
#include <stdio.h>
#include <stdlib.h>


/*
Takes a code relating to an interrupt code to handle and acts appropriately. Currently, only 
program interrupts (a.k.a "syscalls") are handled, although system and external interrupts
will come eventually.

When printing, the `printable` union should be used for integers and floats. This ensures they
can be represented and printed properly.
*/
void handle_interrupt_code(unsigned short code, Register* registers, RAM* ram) {
    // represent values that can be printed
    union {
        int i;
        float f;
    } printable;

    // set the upper 16 bits of the value to print to $g8, and the lower 16 bits to $g9
    printable.i = (get_register(9, registers).word_16 << 16) | get_register(10, registers).word_16;

    switch (code) {
        case 1:  // print signed int in $g8, $g9
            printf("%d\n", printable.i);
            break;

        case 2:  // print float in $g8, $g9
            printf("%f\n", printable.f);
            break;

        case 3:  // print str starting at addr in $ua, $g9, ending at next 0x0000 in RAM
        case 4:  // read int
        case 5:  // read float
        case 6:  // read str into addr in $ua, $g9
        case 7:  // allocate heap memory, length in bytes of len at $g8, $g9
        case 8:  // open file with name in str starting at addr in $g9, in mode in $g8
        case 9:  // read block of data from opened file into addr starting at $g9, offset block by $g8
        case 10: // write byte in $g8 to file to block in $ua, byte index in $g9
        case 11: // close file
        case 12: // MIDI out, MIDI code in $g9
        case 13: // get system time into $g8 and $g9
        case 14: // sleep for milliseconds in $g8, $g9
        case 15: // set seed for random num generator to $g8, $g9
        case 16: // get random integer into $g9
        case 17: // get random float into $g8, $g9
            printf("Valid syscall detected!");
            break;

        case 18: // print integer in $g9 as hex
            printf("%X\n", printable.i);
            break;

        case 19: // print integer in $g8, $g9 as unsigned int
            printf("%u\n", printable.i);
            break;
        
        default:
            printf("Invalid syscall detected!");
            exit(-5);
    }
}