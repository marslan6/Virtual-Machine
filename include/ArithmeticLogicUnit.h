/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#ifndef ALU_H
#define ALU_H

#include <cstdint>

#include "./../include/Lc3Types.h"

class MemoryIO;
class CPU;

class ArithmeticLogicUnit
{
public:
    // ===== Construction =====
    ArithmeticLogicUnit(MemoryIO* memoryIO, CPU* cpu);

    // ===== Instruction handlers =====
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

private:
    // ===== Instruction field decoders =====
    [[nodiscard]] static std::uint16_t SignExtend(std::uint16_t immNumber, std::uint32_t immNumberLength) noexcept;
    [[nodiscard]] static std::uint16_t GetDestinationRegister(Word16 instruction) noexcept;
    [[nodiscard]] static std::uint16_t GetSource1Register(Word16 instruction) noexcept;
    [[nodiscard]] static std::uint16_t GetSource2Register(Word16 instruction) noexcept;
    [[nodiscard]] static bool          GetImmediateFlag(Word16 instruction) noexcept;
    [[nodiscard]] static std::uint16_t GetImmediate5(Word16 instruction) noexcept;
    [[nodiscard]] static std::uint16_t GetPcOffset9(Word16 instruction) noexcept;
    [[nodiscard]] static std::uint16_t GetOffset6(Word16 instruction) noexcept;

    // ===== State =====
    CPU* cpu;
    MemoryIO* memoryIO;
};

#endif // ALU_H