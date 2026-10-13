/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#ifndef TRAP_H
#define TRAP_H


#include <cstdint>


class CPU;


enum TrapCodes : uint16_t
{
    TRAP_GETC = 0x0020,  // get character from keyboard, not echoed onto the terminal
    TRAP_OUT = 0x0021,   // output a character
    TRAP_PUTS = 0x0022,  // output a word string
    TRAP_IN = 0x0023,    // get character from keyboard, echoed onto the terminal
    TRAP_PUTSP = 0x0024, // output a byte string
    TRAP_HALT = 0x0025   // halt the program
};


class Trap
{
private:
    uint16_t* memoryPtr;
    uint16_t* registersPtr;
    CPU* cpuPtr;

public:
    Trap(uint16_t* memory, uint16_t* registers, CPU* cpu);

    void Proxy(uint16_t instruction);

    void GETC();
    void OUTC();
    void PUTS();
    void INC();
    void PUTSP();
    void HALT();
};
#endif