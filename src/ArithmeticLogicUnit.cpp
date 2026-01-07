/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#include "ArithmeticLogicUnit.h"
#include "CPU.h"
#include "MemoryIO.h"


/**
 * @brief Constructs an instance of ArithmeticLogicUnit.
 *
 * Initializes pointers to memory, registers, MemoryIO, and CPU objects.
 *
 * @param memory Pointer to the memory array.
 * @param registers Pointer to the registers array.
 * @param memoryIO Pointer to the MemoryIO object.
 * @param cpu Pointer to the CPU object.
 */
ArithmeticLogicUnit::ArithmeticLogicUnit(uint16_t* memory, uint16_t* registers, MemoryIO* memoryIO, CPU* cpu)
{
    // Initialize memory pointer
    memoryPtr = memory;

    // Initialize registers pointer
    registersPtr = registers;

    // Initialize MemoryIO pointer
    memoryIOPtr = memoryIO;

    // Initialize CPU pointer
    cpuPtr = cpu;
}


/**
 * @brief Sign-extends the given immediate number.
 *
 * This method sign-extends the given immediate number to preserve its sign bit
 * when it is represented in a wider bit length, determined by immNumberLength.
 *
 * @param immNumber The immediate number to be sign-extended.
 * @param immNumberLength The length of the immediate number in bits.
 * @return The sign-extended immediate number.
 */
uint16_t ArithmeticLogicUnit::SignExtend(uint16_t immNumber, int immNumberLength) const
{
    // Determine the sign bit of the immediate number
    int signBit = (immNumber >> (immNumberLength - 1)) & 0x0001;

    // If the sign bit is set, perform sign extension
    if (signBit) // If sign bit is 1
    {
        // Extend the sign bit to the left to fill remaining bits with 1s
        return immNumber | (0xFFFF << immNumberLength);
    }

    // If sign bit is zero (0), return the original number
    return immNumber;
}


/**
 * @brief Swaps the byte order of a 16-bit value.
 *
 * This function reverses the byte order of the input 16-bit value.
 *
 * @param number The 16-bit value to be swapped.
 * @return The 16-bit value with its byte order swapped.
 */
uint16_t ArithmeticLogicUnit::Swap16(uint16_t number)
{
    // Shift the number 8 bits to the left and combine it with the number shifted 8 bits to the right
    return (number << 8) | (number >> 8);
}


/**
 * @brief Performs an addition operation based on the provided instruction.
 *
 * This function extracts operands from the instruction and performs addition.
 *
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::ADD(uint16_t instruction)
{
    // Extract Destination Register (DR), Source Register 1 (SR1), and Immediate Flag (Imm)
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t SR1 = (instruction >> 6) & 0x0007; // Source Register 1
    uint16_t Imm = (instruction >> 5) & 0x0001; // Immediate Flag

    if (Imm)
    {
        // If Immediate Flag is set, extract immediate value and perform addition
        uint16_t imm5 = (instruction) & 0x001F;
        imm5 = SignExtend(imm5, 5); // Sign-extend the immediate value
        registersPtr[DR] = registersPtr[SR1] + imm5;
    }
    else
    {
        // If Immediate Flag is not set, extract Source Register 2 (SR2) and perform addition
        uint16_t SR2 = (instruction) & 0x0007; // Source Register 2
        registersPtr[DR] = registersPtr[SR1] + registersPtr[SR2];
    }

    // Update condition flags based on the result in the destination register
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a bitwise AND operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::AND(uint16_t instruction)
{
    // Extract Destination Register (DR), Source Register 1 (SR1), and Immediate Flag (Imm)
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t SR1 = (instruction >> 6) & 0x0007; // Source Register 1
    uint16_t Imm = (instruction >> 5) & 0x0001; // Immediate Flag

    if (Imm)
    {
        // If Immediate Flag is set, extract immediate value and perform bitwise AND
        uint16_t imm5 = (instruction) & 0x001F;
        imm5 = SignExtend(imm5, 5); // Sign-extend the immediate value
        registersPtr[DR] = registersPtr[SR1] & imm5;
    }
    else
    {
        // If Immediate Flag is not set, extract Source Register 2 (SR2) and perform bitwise AND
        uint16_t SR2 = (instruction) & 0x0007; // Source Register 2
        registersPtr[DR] = registersPtr[SR1] & registersPtr[SR2];
    }

    // Update condition flags based on the result in the destination register
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a bitwise NOT operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::NOT(uint16_t instruction)
{
    // Extract Destination Register (DR) and Source Register 1 (SR1)
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t SR1 = (instruction >> 6) & 0x0007; // Source Register 1

    // Perform bitwise NOT operation on the value in source register and store it in destination register
    registersPtr[DR] = ~registersPtr[SR1];

    // Update condition flags based on the result in the destination register
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a branch operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::BR(uint16_t instruction)
{
    // Extract pcOffset and Condition Flag
    uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);
    uint16_t conditionFlag = (instruction >> 9) & 0x0007;

    // Check if condition flag is set in the Condition Register, then update PC
    if (conditionFlag & registersPtr[Registers::R_COND])
    {
        registersPtr[Registers::R_PC] += pcOffset;
    }
}


/**
 * @brief Performs a jump operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::JMP(uint16_t instruction)
{
    // Extract Source Register (SR1)
    uint16_t SR1 = (instruction >> 6) & 0x0007;

    // Set PC to the value in the source register
    registersPtr[Registers::R_PC] = registersPtr[SR1];
}


/**
 * @brief Performs a jump to subroutine operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::JSR(uint16_t instruction)
{
    // Extract Long Flag to determine the type of jump
    uint16_t longFlag = (instruction >> 11) & 0x0001;

    // Save the current PC value to R7 (Return Address Register)
    registersPtr[Registers::R_7] = registersPtr[Registers::R_PC];

    if (longFlag)
    {
        // If Long Flag is set, calculate the long PC offset and perform jump
        uint16_t longPCOffset = SignExtend(instruction & 0x07FF, 11);
        registersPtr[Registers::R_PC] += longPCOffset;  // JSR
    }
    else
    {
        // If Long Flag is not set, extract the source register and perform jump
        uint16_t SR1 = (instruction >> 6) & 0x0007;
        registersPtr[Registers::R_PC] = registersPtr[SR1]; // JSRR
    }
}


/**
 * @brief Performs a load operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::LD(uint16_t instruction)
{
    // Extract Destination Register (DR) and PC Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9);

    // Load the value from memory at the calculated address into the destination register
    registersPtr[DR] = memoryIOPtr->Read(registersPtr[Registers::R_PC] + pcOffset);

    // Update the condition flags based on the value loaded into the destination register
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a load from base register with offset operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::LDR(uint16_t instruction)
{
    // Extract Destination Register (DR), Base Register (SR1), and Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t SR1 = (instruction >> 6) & 0x0007; // Base Register
    uint16_t offset = SignExtend(instruction & 0x003F, 6); // Offset

    // Load the value from memory at the address calculated by adding base register value and offset
    registersPtr[DR] = memoryIOPtr->Read(registersPtr[SR1] + offset);

    // Update the condition flags based on the value loaded into the destination register
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a load effective address operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::LEA(uint16_t instruction)
{
    // Extract Destination Register (DR) and PC Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9); // PC Offset

    // Calculate the effective address by adding PC value and PC offset
    registersPtr[DR] = registersPtr[Registers::R_PC] + pcOffset;

    // Update the condition flags based on the effective address value
    cpuPtr->UpdateFlags(DR);
}


/**
 * @brief Performs a store operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::ST(uint16_t instruction)
{
    // Extract Destination Register (DR) and PC Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9); // PC Offset

    // Write the value from DR register to memory at the address calculated by adding PC value and PC offset
    memoryIOPtr->Write(registersPtr[Registers::R_PC] + pcOffset, registersPtr[DR]);
}


/**
 * @brief Performs an indirect store operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::STI(uint16_t instruction)
{
    // Extract Destination Register (DR) and PC Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t pcOffset = SignExtend(instruction & 0x01FF, 9); // PC Offset

    // Perform an indirect store by first reading the memory at the address calculated by adding PC value and PC offset,
    // then storing the value from DR register to the memory location read.
    memoryIOPtr->Write(memoryIOPtr->Read(registersPtr[Registers::R_PC] + pcOffset), registersPtr[DR]);
}


/**
 * @brief Performs a store register operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::STR(uint16_t instruction)
{
    // Extract Destination Register (DR), Base Register (SR1), and Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t SR1 = (instruction >> 6) & 0x0007; // Base Register
    uint16_t offset = SignExtend(instruction & 0x003F, 6); // Offset

    // Write the value from DR register to memory at the address calculated by adding SR1 register value and offset
    memoryIOPtr->Write(registersPtr[SR1] + offset, registersPtr[DR]);
}


/**
 * @brief Performs a load indirect operation based on the provided instruction.
 * @param instruction The 16-bit instruction.
 */
void ArithmeticLogicUnit::LDI(uint16_t instruction)
{
    // Extract Destination Register (DR) and PC Offset
    uint16_t DR = (instruction >> 9) & 0x0007; // Destination Register
    uint16_t pc_offset = SignExtend(instruction & 0x01FF, 9); // PC Offset

    // Calculate the effective address by adding PC value and PC offset,
    // then load the value from the memory location pointed by the calculated address into the destination register
    registersPtr[DR] = memoryIOPtr->Read(memoryIOPtr->Read(registersPtr[Registers::R_PC] + pc_offset));

    // Update the condition flags based on the value loaded into the destination register
    cpuPtr->UpdateFlags(DR);
}