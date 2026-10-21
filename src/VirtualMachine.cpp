/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/

#include <cstdlib>
#include <stdexcept>

#include "./../include/VirtualMachine.h"
#include "./../include/CPU.h"
#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/MemoryIO.h"
#include "./../include/OS.h"
#include "./../include/Trap.h"


VirtualMachine::VirtualMachine(CPU* cpu, OS* os, Trap* trap, MemoryIO* memoryIO, ArithmeticLogicUnit* alu)
{
    cpu = cpu;
    os = os;
    trap = trap;
    memoryIO = memoryIO;
    alu = alu;
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
        if (!cpu->LoadImage(argv[j]))
        {
            // Print error message if image file cannot be loaded and exit with error code 1
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }

    // Set up a signal handler for interrupt signal (Ctrl+C)
    signal(SIGINT, OS::HandleInterruptWrapper);

    // Disable input buffering to allow direct console input
    os->DisableInputBuffering();

    while (cpu->IsRunning())
    {

        // Fetch Instruction. Read the memory location pointed by program counter.

        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC)+1);
        std::uint16_t instruction = memoryIO->Read(cpu->ReadRegister(Register::PC));        

        // Extract the opcode from the instruction by considering bits [15:12]
        auto opcode = static_cast<Opcode>(instruction >> 12);

        switch (opcode)
        {
            case Opcode::ADD:  
                alu->ADD(instruction);   
                break;
            case Opcode::AND:  
                alu->AND(instruction);   
                break;
            case Opcode::NOT:  
                alu->NOT(instruction);   
                break;
            case Opcode::BR:   
                alu->BR(instruction);    
                break;
            case Opcode::JMP:  
                alu->JMP(instruction);   
                break;
            case Opcode::JSR:  
                alu->JSR(instruction);   
                break;
            case Opcode::LD:   
                alu->LD(instruction);    
                break;
            case Opcode::LDR:  
                alu->LDR(instruction);   
                break;
            case Opcode::LDI:  
                alu->LDI(instruction);   
                break;
            case Opcode::LEA:  
                alu->LEA(instruction);   
                break;
            case Opcode::ST:   
                alu->ST(instruction);    
                break;
            case Opcode::STR:  
                alu->STR(instruction);   
                break;
            case Opcode::STI:  
                alu->STI(instruction);   
                break;
            case Opcode::TRAP: 
                trap->Proxy(instruction); 
                break;
            case Opcode::RTI:
            case Opcode::RES:
            default:
                throw std::runtime_error("illegal opcode");
        }
    }

    os->RestoreInputBuffering();
}