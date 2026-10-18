/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#ifndef TRAP_H
#define TRAP_H

#include <span>

#include "./../include/Lc3Types.h"

class CPU;

class Trap
{
public:
    // ===== Construction =====
    Trap(std::span<Word16> memory, CPU* cpu);
    ~Trap() = default;

    // ===== Trap dispatch =====
    void Proxy(Word16 instruction);

private:
    // ===== Helpers =====
    [[nodiscard]] std::span<Word16> StringAtR0();
    void StoreCharInR0(int character);

    // ===== Trap routines =====
    void GETC();
    void OUTC();
    void PUTS();
    void INC();
    void PUTSP();
    void HALT();

    // ===== State =====
    std::span<Word16> memory;
    CPU* cpu;
};

#endif // TRAP_H