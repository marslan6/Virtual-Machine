/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#include <iostream>
#include "CPU.h"
#include "ArithmeticLogicUnit.h"
#include "MemoryIO.h"
#include "OS.h"
#include "Trap.h"
#include "VirtualMachine.h"

int main(int argc, const char* argv[])
{
    CPU cpu;
    OS os;
    Trap trap(cpu.memory, cpu.registers, &cpu);
    MemoryIO memoryIO(cpu.memory, &os);
    ArithmeticLogicUnit alu(cpu.memory, cpu.registers, &memoryIO, &cpu);

    VirtualMachine virtualMachine(&cpu, &os, &trap, &memoryIO, &alu);
    virtualMachine.RunVirtualMachine(argc, argv);
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
