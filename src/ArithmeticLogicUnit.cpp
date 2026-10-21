/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/CPU.h"
#include "./../include/MemoryIO.h"

// ===== Construction =====

/// Constructs the ALU with non-owning pointers to the memory interface and CPU.
/// @param memoryIO Non-owning pointer to the memory interface; must outlive this ALU.
/// @param cpu Non-owning pointer to the CPU whose registers the ALU operates on.
ArithmeticLogicUnit::ArithmeticLogicUnit(MemoryIO* memoryIO, CPU* cpu)
    : cpu(cpu), memoryIO(memoryIO)
{
}

// ===== Instruction handlers =====

/// ADD: writes SR1 plus either SR2 or a sign-extended imm5 into DR, then updates flags.
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::ADD(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t SR1 = GetSource1Register(instruction);

    if (GetImmediateFlag(instruction))
    {
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + GetImmediate5(instruction));
    }
    else
    {
        std::uint16_t SR2 = GetSource2Register(instruction);
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + cpu->ReadRegister(SR2));
    }

    cpu->UpdateConditionFlags(DR);
}

/// AND: writes the bitwise AND of SR1 with SR2 or a sign-extended imm5 into DR, then updates flags.
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::AND(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t SR1 = GetSource1Register(instruction);

    if (GetImmediateFlag(instruction))
    {
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) & GetImmediate5(instruction));
    }
    else
    {
        std::uint16_t SR2 = GetSource2Register(instruction);
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) & cpu->ReadRegister(SR2));
    }

    cpu->UpdateConditionFlags(DR);
}

/// NOT: writes the bitwise complement of SR1 into DR, then updates flags.
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::NOT(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t SR1 = GetSource1Register(instruction);

    cpu->WriteRegister(DR, ~(cpu->ReadRegister(SR1)));
    cpu->UpdateConditionFlags(DR);
}

/// BR: adds a sign-extended 9-bit offset to PC when the instruction's condition
/// codes overlap the COND register.
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::BR(Word16 instruction)
{
    std::uint16_t pcOffset = GetPcOffset9(instruction);
    std::uint16_t conditionFlag = (instruction >> 9) & 0x0007;

    if ((conditionFlag & cpu->ReadRegister(Register::COND)) != 0)
    {
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + pcOffset);
    }
}

/// JMP: sets PC to the base register (also serves as RET when the base is R7).
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::JMP(Word16 instruction)
{
    std::uint16_t SR1 = GetSource1Register(instruction);

    cpu->WriteRegister(Register::PC, cpu->ReadRegister(SR1));
}

/// JSR: saves PC in R7, then jumps by an 11-bit offset (JSR) or to a base register (JSRR).
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::JSR(Word16 instruction)
{
    std::uint16_t longFlag = (instruction >> 11) & 0x0001;
    cpu->WriteRegister(Register::R7, cpu->ReadRegister(Register::PC));

    if (longFlag != 0)
    {
        std::uint16_t longPcOffset = SignExtend(instruction & 0x07FF, 11);
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + longPcOffset);
    }
    else
    {
        std::uint16_t SR1 = GetSource1Register(instruction);
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(SR1));
    }
}

/// LD: loads the word at PC plus a 9-bit offset into DR, then updates flags.
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if the computed address is outside memory.
void ArithmeticLogicUnit::LD(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t pcOffset = GetPcOffset9(instruction);

    cpu->WriteRegister(DR, memoryIO->Read(cpu->ReadRegister(Register::PC) + pcOffset));
    cpu->UpdateConditionFlags(DR);
}

/// LDR: loads the word at the base register plus a 6-bit offset into DR, then updates flags.
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if the computed address is outside memory.
void ArithmeticLogicUnit::LDR(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t SR1 = GetSource1Register(instruction);
    std::uint16_t offset = GetOffset6(instruction);

    cpu->WriteRegister(DR, memoryIO->Read(cpu->ReadRegister(SR1) + offset));
    cpu->UpdateConditionFlags(DR);
}

/// LEA: writes the effective address PC plus a 9-bit offset into DR (no memory
/// access), then updates flags.
/// @param instruction The 16-bit instruction word.
void ArithmeticLogicUnit::LEA(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t pcOffset = GetPcOffset9(instruction);

    cpu->WriteRegister(DR, cpu->ReadRegister(Register::PC) + pcOffset);
    cpu->UpdateConditionFlags(DR);
}

/// ST: stores the source register to memory at PC plus a 9-bit offset.
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if the computed address is outside memory.
void ArithmeticLogicUnit::ST(Word16 instruction)
{
    std::uint16_t SR = GetDestinationRegister(instruction);
    std::uint16_t pcOffset = GetPcOffset9(instruction);

    memoryIO->Write(cpu->ReadRegister(Register::PC) + pcOffset, cpu->ReadRegister(SR));
}

/// STI: stores the source register to the address held at memory[PC + 9-bit offset].
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if either computed address is outside memory.
void ArithmeticLogicUnit::STI(Word16 instruction)
{
    std::uint16_t SR = GetDestinationRegister(instruction);
    std::uint16_t pcOffset = GetPcOffset9(instruction);

    memoryIO->Write(memoryIO->Read(cpu->ReadRegister(Register::PC) + pcOffset), cpu->ReadRegister(SR));
}

/// STR: stores the source register to memory at the base register plus a 6-bit offset.
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if the computed address is outside memory.
void ArithmeticLogicUnit::STR(Word16 instruction)
{
    std::uint16_t SR = GetDestinationRegister(instruction);
    std::uint16_t SR1 = GetSource1Register(instruction);
    std::uint16_t offset = GetOffset6(instruction);

    memoryIO->Write(cpu->ReadRegister(SR1) + offset, cpu->ReadRegister(SR));
}

/// LDI: loads the word at the address held at memory[PC + 9-bit offset] into DR,
/// then updates flags.
/// @param instruction The 16-bit instruction word.
/// @throws std::out_of_range if either computed address is outside memory.
void ArithmeticLogicUnit::LDI(Word16 instruction)
{
    std::uint16_t DR = GetDestinationRegister(instruction);
    std::uint16_t pcOffset = GetPcOffset9(instruction);

    cpu->WriteRegister(DR, memoryIO->Read(memoryIO->Read(cpu->ReadRegister(Register::PC) + pcOffset)));
    cpu->UpdateConditionFlags(DR);
}

// ===== Instruction field decoders =====

/// Sign-extends an immNumberLength-bit value to 16 bits.
/// @param immNumber The value whose low immNumberLength bits are significant.
/// @param immNumberLength The number of significant bits.
/// @return The value sign-extended to 16 bits.
std::uint16_t ArithmeticLogicUnit::SignExtend(std::uint16_t immNumber, std::uint32_t immNumberLength) noexcept
{
    const std::uint32_t signBit = (immNumber >> (immNumberLength - 1)) & 0x0001;

    if (signBit != 0)
    {
        return static_cast<std::uint16_t>(immNumber | (0xFFFF << immNumberLength));
    }

    return immNumber;
}

/// Extracts the destination register field (bits 11-9).
/// @param instruction The 16-bit instruction word.
/// @return Register index in the range [0, 7].
std::uint16_t ArithmeticLogicUnit::GetDestinationRegister(Word16 instruction) noexcept
{
    return (instruction >> 9) & 0x0007;
}

/// Extracts the first source register field (bits 8-6).
/// @param instruction The 16-bit instruction word.
/// @return Register index in the range [0, 7].
std::uint16_t ArithmeticLogicUnit::GetSource1Register(Word16 instruction) noexcept
{
    return (instruction >> 6) & 0x0007;
}

/// Extracts the second source register field (bits 2-0).
/// @param instruction The 16-bit instruction word.
/// @return Register index in the range [0, 7].
std::uint16_t ArithmeticLogicUnit::GetSource2Register(Word16 instruction) noexcept
{
    return instruction & 0x0007;
}

/// Reads the immediate-mode flag (bit 5).
/// @param instruction The 16-bit instruction word.
/// @return true if the instruction uses an immediate operand.
bool ArithmeticLogicUnit::GetImmediateFlag(Word16 instruction) noexcept
{
    return ((instruction >> 5) & 0x0001) != 0;
}

/// Extracts and sign-extends the 5-bit immediate operand (bits 4-0).
/// @param instruction The 16-bit instruction word.
/// @return The sign-extended immediate value.
std::uint16_t ArithmeticLogicUnit::GetImmediate5(Word16 instruction) noexcept
{
    return SignExtend(instruction & 0x001F, 5);
}

/// Extracts and sign-extends the 9-bit PC offset (bits 8-0).
/// @param instruction The 16-bit instruction word.
/// @return The sign-extended PC offset.
std::uint16_t ArithmeticLogicUnit::GetPcOffset9(Word16 instruction) noexcept
{
    return SignExtend(instruction & 0x01FF, 9);
}

/// Extracts and sign-extends the 6-bit offset (bits 5-0).
/// @param instruction The 16-bit instruction word.
/// @return The sign-extended offset.
std::uint16_t ArithmeticLogicUnit::GetOffset6(Word16 instruction) noexcept
{
    return SignExtend(instruction & 0x003F, 6);
}