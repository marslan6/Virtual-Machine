/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H

#include <span>

class CPU;
class OS;
class Trap;
class MemoryIO;
class ArithmeticLogicUnit;

class VirtualMachine
{
public:
    // ===== Construction =====
    VirtualMachine(CPU* cpu, OS* os, Trap* trap, MemoryIO* memoryIO, ArithmeticLogicUnit* alu);

    // ===== Execution =====
    void RunVirtualMachine(std::span<const char*> args);

private:
    // ===== State =====
    CPU* cpu;
    OS* os;
    Trap* trap;
    MemoryIO* memoryIO;
    ArithmeticLogicUnit* alu;
};

#endif // VIRTUAL_MACHINE_H