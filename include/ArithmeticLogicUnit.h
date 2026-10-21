/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#ifndef ALU_H
#define ALU_H


#include <cstdint>
#include <span>

#include "./../include/Lc3Types.h"

class MemoryIO;
class CPU;


class ArithmeticLogicUnit
{
private:
    CPU* cpu;
    MemoryIO* memoryIO;

public:
    ArithmeticLogicUnit(MemoryIO* memoryIO, CPU* cpu);

    [[nodiscard]] std::uint16_t SignExtend(std::uint16_t immNumber, int immNumberLength) const;

    void ADD(Word16 instruction);
    void AND(Word16 instruction);
    void NOT(Word16 instruction);

    void BR(Word16 instruction);
    void JMP(Word16 instruction);
    void JSR(Word16 instruction);

    void LD(Word16 instruction);
    void LDR(Word16 instruction);
    void LEA(Word16 instruction);

    void ST(Word16 instruction);
    void STI(Word16 instruction);
    void STR(Word16 instruction);

    void LDI(Word16 instruction);
};
#endif