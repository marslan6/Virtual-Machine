/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include "./../include/Trap.h"
#include "./../include/CPU.h"

#include <iostream>
#include <utility>
#include <stdexcept>

// ===== Construction =====

/// Constructs the trap handler over the machine's memory and the CPU whose
/// registers the trap routines read and write.
/// @param memory View of the machine's memory; must outlive this Trap.
/// @param cpu Non-owning pointer to the CPU; must remain valid for all traps.
Trap::Trap(std::span<Word16> memory, CPU* cpu)
    : memory(memory), cpu(cpu)
{
}

// ===== Trap dispatch =====

/// Saves the return address in R7 and dispatches to the trap routine named by
/// the low byte (trap vector) of the instruction.
/// @param instruction The full TRAP instruction word.
/// @throws std::runtime_error if the trap vector is not recognized.
void Trap::Proxy(Word16 instruction)
{
    // Trap routines may clobber the PC, so preserve the return address in R7.
    cpu->WriteRegister(Register::R7, cpu->ReadRegister(Register::PC));

    switch (static_cast<TrapVector>(instruction & Lc3Trap::kLowByteMask))
    {
        case TrapVector::GetChar:
            GETC();
            break;

        case TrapVector::Out:
            OUTC();
            break;

        case TrapVector::PutString:
            PUTS();
            break;

        case TrapVector::In:
            INC();
            break;

        case TrapVector::PutStringP:
            PUTSP();
            break;

        case TrapVector::Halt:
            HALT();
            break;

        default:
            throw std::runtime_error("Trap::Proxy: unknown trap vector");
    }
}

// ===== Trap routines =====

/// GETC: reads one character from the keyboard (not echoed) into R0.
void Trap::GETC()
{
    const int character = std::cin.get();
    cpu->WriteRegister(Register::R0, static_cast<Word16>(character));
    cpu->UpdateConditionFlags(std::to_underlying(Register::R0));
}

/// OUTC: writes the character in R0 to the console.
void Trap::OUTC()
{
    std::cout.put(static_cast<char>(cpu->ReadRegister(Register::R0)));
    std::cout.flush();
}

/// PUTS: writes a string stored one character per word, starting at the
/// address in R0, until a zero word is reached.
void Trap::PUTS()
{
    const Address start = cpu->ReadRegister(Register::R0);
    if (start >= memory.size())
    {
        throw std::out_of_range("Trap::PUTS: string address out of range");
    }

    for (const Word16 word : memory.subspan(start))
    {
        if (word == 0)
        {
            break;
        }

        std::cout.put(static_cast<char>(word));
    }

    std::cout.flush();
}

/// INC: prompts for and reads one character (echoed) into R0.
void Trap::INC()
{
    std::cout << "Enter a character: ";
    std::cout.flush();

    const int character = std::cin.get();
    std::cout.put(static_cast<char>(character));
    std::cout.flush();

    cpu->WriteRegister(Register::R0, static_cast<Word16>(character));
    cpu->UpdateConditionFlags(std::to_underlying(Register::R0));
}

/// PUTSP: writes a string packed two characters per word (low byte first,
/// then high byte), starting at the address in R0, until a zero word.
void Trap::PUTSP()
{
    const Address start = cpu->ReadRegister(Register::R0);
    if (start >= memory.size())
    {
        throw std::out_of_range("Trap::PUTSP: string address out of range");
    }

    for (const Word16 word : memory.subspan(start))
    {
        if (word == 0)
        {
            break;
        }

        const char low = static_cast<char>(word & Lc3Trap::kLowByteMask);
        std::cout.put(low);

        const char high = static_cast<char>(word >> Lc3Trap::kHighByteShift);
        if (high)
        {
            std::cout.put(high);
        }
    }

    std::cout.flush();
}

/// HALT: prints a halt message and stops the CPU.
void Trap::HALT()
{
    std::cout << "HALT\n";
    std::cout.flush();
    cpu->Stop();
}