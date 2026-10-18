/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#include "./../include/Trap.h"
#include "./../include/CPU.h"
#include <print>
#include <utility>
#include <stdexcept>


Trap::Trap(std::span<Word16> memory, CPU* cpu) 
: memory(memory), cpu(cpu)
{
}


void Trap::Proxy(Word16 instruction)
{
    // Save the return address
    cpu->WriteRegister(Register::R7, cpu->ReadRegister(Register::PC));

    // Switch based on the trap vector
    switch (static_cast<TrapVector>(instruction & Lc3Trap::kLowByteMask))
    {
    case TrapVector::GetChar:
        GETC(); // Handle GETC trap
        break;
    case TrapVector::Out:
        OUT(); // Handle OUTC trap
        break;
    case TrapVector::PutString:
        PUTS(); // Handle PUTS trap
        break;
    case TrapVector::In:
        IN(); // Handle INC trap
        break;
    case TrapVector::PutStringP:
        PUTSP(); // Handle PUTSP trap
        break;
    case TrapVector::Halt:
        HALT(); // Handle HALT trap
        break;
    default: // TODO:
        throw std::runtime_error("Trap: unknown trap vector");
    }
}


void Trap::GETC()
{
    // Read character from console
    cpu->WriteRegister(Register::R0, static_cast<Word16>(getchar()));
    // Update condition flags based on the result
    cpu->UpdateConditionFlags(std::to_underlying(Register::R0));
}


void Trap::OUT()
{
    // Output character to console
    putc(static_cast<char>(cpu->ReadRegister(Register::R0)), stdout);
    // Flush output buffer to ensure immediate display
    fflush(stdout);
}


void Trap::PUTS()
{
    // Iterate through memory and output characters until null terminator is encountered
    std::span<Word16> memoryFromR0 = memory.subspan(cpu->ReadRegister(Register::R0));

    for (const auto& word : memoryFromR0)
    {
        if (word == 0)
        {
            break;
        }

        putc(static_cast<char>(word), stdout);
    }

    // Flush output buffer to ensure immediate display
    fflush(stdout);
}


void Trap::IN()
{
    std::println("Enter a character: ");

    // Read character from console
    char c = getchar();
    // Output character to console
    putc(c, stdout);
    // Flush output buffer to ensure immediate display
    fflush(stdout);
    // Store ASCII value of character in register R0
    cpu->WriteRegister(Register::R0, static_cast<Word16>(c));
    // Update condition flags based on the result
    cpu->UpdateConditionFlags(std::to_underlying(Register::R0));
}


void Trap::PUTSP()
{
    // Iterate through memory and output characters until null terminator is encountered
    std::span<Word16> memoryFromR0 = memory.subspan(cpu->ReadRegister(Register::R0));

    for (const auto& word : memoryFromR0)
    {
        if (word == 0)
        {
            break;
        }

        char low = static_cast<char>(word & Lc3Trap::kLowByteMask);
        putc(low, stdout);

        char high =  static_cast<char>(word >> Lc3Trap::kHighByteShift);
        if (high)
        {
            putc(high, stdout);
        }
    }

    // Flush output buffer to ensure immediate display
    fflush(stdout);
}


void Trap::HALT()
{
    puts("HALT");
    // Flush output buffer to ensure immediate display
    fflush(stdout);
    // Set 'running' flag to false to halt execution
    cpu->Stop();
}
