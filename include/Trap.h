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
    void Dispatch(Word16 instruction);   // or Dispatch(TrapVector)

private:
    // ===== Trap handlers =====
    void GETC();
    void OUT();
    void PUTS();
    void IN();
    void PUTSP();
    void HALT();

    // ===== State =====
    std::span<Word16> memory;
    CPU* cpu;
};

#endif