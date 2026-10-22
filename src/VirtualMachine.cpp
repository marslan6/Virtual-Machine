/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include <cstdlib>
#include <stdexcept>
#include <print>
#include <span>
#include <csignal>

#include "./../include/VirtualMachine.h"
#include "./../include/CPU.h"
#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/MemoryIO.h"
#include "./../include/OS.h"
#include "./../include/Trap.h"

namespace
{
    /// RAII guard: disables console input buffering for its lifetime and
    /// restores it on scope exit, including during exception unwinding.
    class ScopedRawInput
    {
    public:
        explicit ScopedRawInput(OS* os) : os(os) { os->DisableInputBuffering(); }
        ~ScopedRawInput() { os->RestoreInputBuffering(); }

        ScopedRawInput(const ScopedRawInput&) = delete;
        ScopedRawInput& operator=(const ScopedRawInput&) = delete;

    private:
        OS* os;
    };
}

// ===== Construction =====

/// Constructs the virtual machine from its components (non-owning pointers).
/// @param cpu Register file, program counter, and image loading.
/// @param os Platform console setup and interrupt handling.
/// @param trap Trap (service routine) handler.
/// @param memoryIO Memory read/write interface.
/// @param alu Arithmetic/logic and control instruction handlers.
VirtualMachine::VirtualMachine(CPU* cpu, OS* os, Trap* trap, MemoryIO* memoryIO, ArithmeticLogicUnit* alu)
    : cpu(cpu), os(os), trap(trap), memoryIO(memoryIO), alu(alu)
{
}

// ===== Execution =====

/// Loads the program images named on the command line, then runs the
/// fetch-decode-execute loop until the CPU halts.
/// @param args Command-line arguments: program name followed by image files.
/// @throws std::runtime_error if no image is given, an image fails to load,
///         or an illegal opcode is encountered.
void VirtualMachine::RunVirtualMachine(std::span<char* const> args)
{
    if (args.size() < 2)
    {
        throw std::runtime_error("no image file provided");
    }

    // args[0] is the program name; the rest are image files.
    for (const char* imagePath : args.subspan(1))
    {
        if (!cpu->LoadImage(imagePath))
        {
            throw std::runtime_error("failed to load image file");
        }
    }

    std::signal(SIGINT, OS::HandleInterruptWrapper);

    // Raw input stays enabled for the whole run and is restored on any exit.
    ScopedRawInput rawInput(os);

    while (cpu->IsRunning())
    {
        // Fetch: read the instruction at PC, then advance PC to the next word.
        Word16 instruction = memoryIO->Read(cpu->ReadRegister(Register::PC));
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + 1);

        // Decode: the opcode is bits [15:12].
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
}