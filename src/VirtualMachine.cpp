/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#include "VirtualMachine.h"
#include "CPU.h"
#include "ArithmeticLogicUnit.h"
#include "MemoryIO.h"
#include "OS.h"
#include "Trap.h"


VirtualMachine::VirtualMachine(CPU* cpu, OS* os, Trap* trap, MemoryIO* memoryIO, ArithmeticLogicUnit* alu)
{
    cpuPtr = cpu;
    osPtr = os;
    trapPtr = trap;
    memoryIOPtr = memoryIO;
    aluPtr = alu;
}


void VirtualMachine::RunVirtualMachine(int argc, const char* argv[])
{
    // Check if at least one image file is provided as a command-line argument
    if (argc < 2)
    {
        // Display usage information and exit if no image files are provided
        printf("lc3 [image-file1] ...\n");
        exit(2);
    }

    // Iterate over command-line arguments (excluding the program name)
    for (int j = 1; j < argc; ++j)
    {
        // Attempt to read the image file specified by the current command-line argument
        if (!cpuPtr->ReadImage(argv[j], aluPtr))
        {
            // Print error message if image file cannot be loaded and exit with error code 1
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    // Set up a signal handler for interrupt signal (Ctrl+C)
    signal(SIGINT, OS::HandleInterruptWrapper);

    // Disable input buffering to allow direct console input
    osPtr->DisableInputBuffering();

    while (cpuPtr->running)
    {

        // Fetch Instruction. Read the memory location pointed by program counter.
        uint16_t instruction = memoryIOPtr->Read(cpuPtr->registers[Registers::R_PC]++);

        // Extract the opcode from the instruction by considering bits [15:12]
        uint16_t operation = (instruction >> 12);

        switch (operation)
        {
        case OP_ADD:
            aluPtr->ADD(instruction);
            break;
        case OP_AND:
            aluPtr->AND(instruction);
            break;
        case OP_NOT:
            aluPtr->NOT(instruction);
            break;
        case OP_BR:
            aluPtr->BR(instruction);
            break;
        case OP_JMP:
            aluPtr->JMP(instruction);
            break;
        case OP_JSR:
            aluPtr->JSR(instruction);
            break;
        case OP_LD:
            aluPtr->LD(instruction);
            break;
        case OP_LDI:
            aluPtr->LDI(instruction);
            break;
        case OP_LDR:
            aluPtr->LDR(instruction);
            break;
        case OP_LEA:
            aluPtr->LEA(instruction);
            break;
        case OP_ST:
            aluPtr->ST(instruction);
            break;
        case OP_STI:
            aluPtr->STI(instruction);
            break;
        case OP_STR:
            aluPtr->STR(instruction);
            break;
        case OP_TRAP:
            trapPtr->Proxy(instruction);
            break;
        case OP_RES:
        case OP_RTI:
        default:
            abort();
            break;
        }
    }

    osPtr->RestoreInputBuffering();
}