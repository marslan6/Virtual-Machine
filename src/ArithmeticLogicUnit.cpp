/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/CPU.h"
#include "./../include/MemoryIO.h"


ArithmeticLogicUnit::ArithmeticLogicUnit(MemoryIO* memoryIO, CPU* cpu)
    : cpu(cpu), memoryIO(memoryIO)
{
}


std::uint16_t ArithmeticLogicUnit::SignExtend(std::uint16_t immNumber, int immNumberLength) const
{
    int signBit = (immNumber >> (immNumberLength - 1)) & 0x0001;

    if (signBit)
    {
        return immNumber | (0xFFFF << immNumberLength);
    }

    return immNumber;
}


void ArithmeticLogicUnit::ADD(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    Address SR1 = (instruction >> 6) & 0x0007;
    std::uint16_t Imm = (instruction >> 5) & 0x0001;

    if (Imm)
    {
        std::uint16_t imm5 = (instruction) & 0x001F;
        imm5 = SignExtend(imm5, 5);
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + imm5);
    }
    else
    {
        std::uint16_t SR2 = (instruction) & 0x0007;
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + cpu->ReadRegister(SR2));
    }

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::AND(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    Address SR1 = (instruction >> 6) & 0x0007;
    std::uint16_t Imm = (instruction >> 5) & 0x0001;

    if (Imm)
    {
        std::uint16_t imm5 = (instruction) & 0x001F;
        imm5 = SignExtend(imm5, 5);
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) & imm5);
    }
    else
    {
        std::uint16_t SR2 = (instruction) & 0x0007;
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) & cpu->ReadRegister(SR2));
    }

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::NOT(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    Address SR1 = (instruction >> 6) & 0x0007;

    cpu->WriteRegister(DR, ~(cpu->ReadRegister(SR1)));

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::BR(Word16 instruction)
{
    std::uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);
    std::uint16_t conditionFlag = (instruction >> 9) & 0x0007;

    if (conditionFlag & cpu->ReadRegister(Register::COND))
    {
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + pcOffset);
    }
}


void ArithmeticLogicUnit::JMP(Word16 instruction)
{
    Address SR1 = (instruction >> 6) & 0x0007;

    cpu->WriteRegister(Register::PC, cpu->ReadRegister(SR1));
}


void ArithmeticLogicUnit::JSR(Word16 instruction)
{
    std::uint16_t longFlag = (instruction >> 11) & 0x0001;

    cpu->WriteRegister(Register::R7, cpu->ReadRegister(Register::PC));

    if (longFlag)
    {
        std::uint16_t longPCOffset = SignExtend(instruction & 0x07FF, 11);
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + longPCOffset);
    }
    else
    {
        Address SR1 = (instruction >> 6) & 0x0007;
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(SR1));
    }
}


void ArithmeticLogicUnit::LD(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    std::uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);

    cpu->WriteRegister(DR, memoryIO->Read(cpu->ReadRegister(Register::PC) + pcOffset));

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::LDR(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    Address SR1 = (instruction >> 6) & 0x0007;
    std::uint16_t offset = SignExtend(instruction & 0x003F, 6);

    cpu->WriteRegister(DR, memoryIO->Read(cpu->ReadRegister(SR1) + offset));

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::LEA(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    std::uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);

    cpu->WriteRegister(DR, cpu->ReadRegister(Register::PC) + pcOffset);

    cpu->UpdateConditionFlags(DR);
}


void ArithmeticLogicUnit::ST(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    std::uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);

    memoryIO->Write(cpu->ReadRegister(Register::PC) + pcOffset, cpu->ReadRegister(DR));
}


void ArithmeticLogicUnit::STI(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    std::uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);

    memoryIO->Write(memoryIO->Read(cpu->ReadRegister(Register::PC) + pcOffset), cpu->ReadRegister(DR));
}


void ArithmeticLogicUnit::STR(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    Address SR1 = (instruction >> 6) & 0x0007;
    std::uint16_t offset = SignExtend(instruction & 0x003F, 6);

    memoryIO->Write(cpu->ReadRegister(SR1) + offset, cpu->ReadRegister(DR));
}


void ArithmeticLogicUnit::LDI(Word16 instruction)
{
    Address DR = (instruction >> 9) & 0x0007;
    std::uint16_t pc_offset = SignExtend(instruction & 0x01FF, 9);

    cpu->WriteRegister(DR, memoryIO->Read(memoryIO->Read(cpu->ReadRegister(Register::PC) + pc_offset)));

    cpu->UpdateConditionFlags(DR);
}