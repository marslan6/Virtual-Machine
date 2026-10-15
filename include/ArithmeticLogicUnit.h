/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#ifndef ALU_H
#define ALU_H


#include <cstdint>


class MemoryIO;
class CPU;


enum Opcodes : std::uint16_t
{
    OP_BR = 0, // branch
    OP_ADD,    // add 
    OP_LD,     // load
    OP_ST,     // store
    OP_JSR,    // jump register
    OP_AND,    // bitwise and
    OP_LDR,    // load register
    OP_STR,    // store register
    OP_RTI,    // unused
    OP_NOT,    // bitwise not
    OP_LDI,    // load indirect
    OP_STI,    // store indirect
    OP_JMP,    // jump
    OP_RES,    // reserved (unused)
    OP_LEA,    // load effective address
    OP_TRAP    // execute trap
};


class ArithmeticLogicUnit
{
private:
    std::uint16_t* memoryPtr;
    std::uint16_t* registersPtr;
    MemoryIO* memoryIOPtr;
    CPU* cpuPtr;

public:
    ArithmeticLogicUnit(std::uint16_t* memory, std::uint16_t* registers, MemoryIO* memoryIO, CPU* cpu);

    std::uint16_t SignExtend(std::uint16_t immNumber, int immNumberLength) const;
    std::uint16_t Swap16(std::uint16_t number);

    void ADD(std::uint16_t instruction);
    void AND(std::uint16_t instruction);
    void NOT(std::uint16_t instruction);

    void BR(std::uint16_t instruction);
    void JMP(std::uint16_t instruction);
    void JSR(std::uint16_t instruction);

    void LD(std::uint16_t instruction);
    void LDR(std::uint16_t instruction);
    void LEA(std::uint16_t instruction);

    void ST(std::uint16_t instruction);
    void STI(std::uint16_t instruction);
    void STR(std::uint16_t instruction);

    void LDI(std::uint16_t instruction);
};
#endif