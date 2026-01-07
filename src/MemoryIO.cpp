/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#include "MemoryIO.h"
#include "CPU.h"
#include "OS.h"


/**
 * @brief Constructs a MemoryIO object.
 *
 * This constructor initializes a MemoryIO object with the provided memory array and OS pointer.
 *
 * @param memory Pointer to the memory array.
 * @param os Pointer to the OS object.
 */
MemoryIO::MemoryIO(uint16_t* memory, OS* os)
{
    memoryPtr = memory;
    osPtr = os;
}


/**
 * @brief Reads the 16-bit value from memory at the specified address.
 *
 * This function reads a 16-bit value from memory at the specified address.
 * If the address corresponds to the keyboard status register, it checks if a key is pressed
 * and updates the keyboard status register accordingly.
 *
 * @param memoryAddress The address to read from.
 * @return The 16-bit value read from memory.
 */
uint16_t MemoryIO::Read(uint16_t memoryAddress)
{
    // Check if the memory address corresponds to the keyboard status register
    if (memoryAddress == MemoryMappedRegisters::MR_KBSR)
    {
        // If a key is pressed, set the keyboard status register's most significant bit (bit 15) to indicate input
        if (osPtr->CheckKey())
        {
            memoryPtr[MemoryMappedRegisters::MR_KBSR] = (1 << 15);
            // Read the character from the keyboard and store it in the keyboard data register
            memoryPtr[MemoryMappedRegisters::MR_KBDR] = getchar();
        }
        else
        {
            // If no key is pressed, clear the keyboard status register
            memoryPtr[MemoryMappedRegisters::MR_KBSR] = 0;
        }
    }

    // Return the value stored in memory at the specified address
    return memoryPtr[memoryAddress];
}


/**
 * @brief Writes the 16-bit value to memory at the specified address.
 *
 * This function writes a 16-bit value to memory at the specified address.
 *
 * @param address The address to write to.
 * @param value The 16-bit value to write.
 */
void MemoryIO::Write(uint16_t address, uint16_t value)
{
    memoryPtr[address] = value;
}