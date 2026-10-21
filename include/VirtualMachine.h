/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#ifndef VIRTUAL_MACHINE_H
#define VIRTUAL_MACHINE_H


#include <cstdint>
#include "./../include/Lc3Types.h"


class CPU;
class OS;
class Trap;
class MemoryIO;
class ArithmeticLogicUnit;


class VirtualMachine
{
private:
	CPU* cpuPtr;
	OS* osPtr;
	Trap* trap;
	MemoryIO* memoryIOPtr;
	ArithmeticLogicUnit* alu;

public:
	VirtualMachine(CPU* cpu, OS* os, Trap* trap, MemoryIO* memoryIO, ArithmeticLogicUnit* alu);
	void RunVirtualMachine(int argc, const char* argv[]);
};
#endif