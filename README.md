# The Iridium Computer Emulator

Part of a series on my blog: [The System Fire](https://www.thesystemfire.com/).

The Iridium Computer Emulator is a program written in pure C which is designed to emulate the Iridium Computer (of course). This is a custom 16-bit processor the author of this repository has designed and plans to implement using a microcontroller, probably the Raspberry Pi Zero W. The general capabilities of the computer are aimed to be roughly equiavlent to that of the Commodore 64.


## The Architecture

The architecture uses a custom instruction set, the specification for which can be found [here](https://github.com/DominicThorpe/iridium_assembler). There are 16 16-bit registers available, 5 of which are special-purpose, and 11 are general-purpose. The hardware architecture is 16-bits, and can therefore address 16KiB of RAM, which is the maximum addressable by 16 bits. 

There will also be support for a number of I/O devices. We plan to support an SD Card for the hard drive, video and sound output, and a keyboard. There may also be mouse support, but this is not yet decided.


### Memory Map

The memory map has yet to be fully determined whilst the emulator is adapted to a fully 16-bit architecture.

Memory is allocated on the stack using the *friend system*, wherein the heap is organised into a binary tree with each level being a certain block size. When allocating memory, the tree is traversed to find a block of the right size. If one cannot be found, a large, free block is split into 2 recursively until a best-fit is found. Find out more about this technique [here](https://www.geeksforgeeks.org/buddy-system-memory-allocation-technique/).


## Current Progress

Currently migrating from a mixed 32/16-bit architecture to a full 16-bit architecture.

This project is still a WIP with a lot left to go. The roadmap is as follows, and always check the blog to see current progress:
  - Memory
    - [x] Array-based implementation of the registers, including 16- and 32-bit registers  
    - [x] Chained hashmap implementation of RAM
    - [ ] File-based implementation of SD hard drive
    - [ ] File-based implementation of ROM

  - Processing
    - [x] Processing RRR-type ALU instructions (ADD, SUB, SLL, SRL, SRA, NAND, OR)
    - [x] Processing RRR-type memory read/write instructions (LOAD, STORE)
    - [x] Processing RRI-type ALU instructions (ADDI, SUBI)
    - [x] Processing RII-type general instructions (MOVLI, MOVUI)
    - [x] Processing ORR-type ALU instructions (ADDC, SUBC)
    - [x] Processing ORI-type system instructions 
    - [x] Program halting (HALT)

  - Flow-Control
    - [x] Unconditional branching (JUMP, JAL)
    - [x] Comparing values of 2 registers (CMP)
    - [x] Conditional branching (BEQ, BNE, BLT, BGT)

  - Program Loading
    - Can load instructions from a specified file
      - [x] Into RAM from external file
      - [ ] Into RAM from hard drive image

  - Operating System
    - [x] Memory Management
    - [ ] Filesystwm
    - [ ] Interrupt Handler
    - [ ] System UI
    - [ ] Inter-Process Communication
  
  - I/O Support
    - [ ] Keyboard
    - [ ] Screen
    - [ ] Sound
