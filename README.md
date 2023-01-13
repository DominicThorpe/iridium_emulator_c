# The Iridium Computer Emulator

Part of a series on my blog: [The System Fire](https://www.thesystemfire.com/).

The Iridium Computer Emulator is a program written in pure C which is designed to emulate the Iridium Computer (of course). This is a custom 16-bit processor the author of this repository has designed and plans to implement using a microcontroller, probably the Raspberry Pi Zero W.


## The Architecture

The architecture uses a custom instruction set, the specification for which can be found [here](https://github.com/DominicThorpe/iridium_assembler). There are 16 registers available, 5 of which are special-purpose, and 10 are general-purpose. 4 of the special-purpose registers are 32 bits instead of 16 so they can store complete memory addresses.

The hardware architectire is 16-bits, but has a 32-bit wide address bus, so can address up to 4GB of memory, with a minimum of 8KB being ROM, 64KB being for video memory, and 32KB being for the kernel, leaving most of the ram available for user programs.

There is also support for a number of I/O devices. We plan to support an SD Card for the hard drive, video and sound output, and a keyboard. There may also be mouse support, but this is not yet decided.


### Memory Map

Because we have an address space of 4GB, we know we can have *up to* 4GB of RAM/ROM, but we don't know how much we will actually have. When we implement a microprocessor version of this system, this amount will be much less than 1GB.

Each process has access to 1Mb of stack, and another 1Mb of heap memory, with the heap starting at the bottom and the stack at the top. The high water mark of the heap can be adjusted via syscall 19 (aka *sbrk*). The code for a process goes at the start of process memory, followed by non-text data, and text data has its own section, followed by the heap and stack.

Memory is allocated on the stack using the *friend system*, wherein the heap is organised into a binary tree with each level being a certain block size. When allocating memory, the tree is traversed to find a block of the right size. If one cannot be found, a large, free block is split into 2 recursively until a best-fit is found. Find out more about this technique [here](https://www.geeksforgeeks.org/buddy-system-memory-allocation-technique/).


## Current Progress

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
    - Microkernel
      - [x] Paging
      - [x] Memory Management
      - [x] Process Scheduling

    - [ ] Filesystwm
    - [ ] Interrupt Handler
    - [ ] System UI
    - [ ] Inter-Process Communication
  
  - I/O Support
    - [ ] Keyboard
    - [ ] Screen
    - [ ] Sound
