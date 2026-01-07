/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#define _CRT_SECURE_NO_DEPRECATE


#include <iostream>


#include "Trap.h"
#include "MemoryIO.h"
#include "ArithmeticLogicUnit.h"
#include "OS.h"
#include "CPU.h"


/**
 * @brief Initializes the CPU object.
 *
 * This constructor initializes the CPU object by setting the default condition flag to zero
 * and setting the Program Counter (PC) to the starting position.
 */
CPU::CPU()
{
    // Set the default condition flag to zero
    registers[Registers::R_COND] = ConditionFlags::FL_ZERO;

    // Set the Program Counter (PC) to the starting position (default: 0x3000)
    registers[Registers::R_PC] = PC::PC_START;
}


/**
 * @brief Destroys the CPU object.
 *
 * This destructor does not perform any specific actions as the CPU object is being destroyed.
 */
CPU::~CPU()
{
}


/**
 * @brief Updates the condition flags based on the value in the specified register.
 *
 * This function updates the condition flags based on the value stored in the specified destination register.
 *
 * @param DR The index of the destination register whose value is used to update the flags.
 */
void CPU::UpdateFlags(uint16_t DR)
{
    if (registers[DR] == 0)
    {
        // Set the zero flag if the value in the register is zero
        registers[Registers::R_COND] = ConditionFlags::FL_ZERO;
    }
    else if ((registers[DR] >> 15) == 1)
    {
        // Set the negative flag if the leftmost bit is 1
        registers[Registers::R_COND] = ConditionFlags::FL_NEGATIVE;
    }
    else
    {
        // Set the positive flag if neither zero nor negative conditions are met
        registers[Registers::R_COND] = ConditionFlags::FL_POSITIVE;
    }
}


/**
 * @brief Reads the contents of an image file and stores them in the memory array.
 *
 * This function reads the contents of the specified image file and stores them in the memory array
 * starting from the specified origin address. It also performs byte order conversion as necessary.
 *
 * @param file Pointer to the image file to be read.
 * @param alu Pointer to the ArithmeticLogicUnit object used for byte swapping.
 */
void CPU::ReadImageFile(FILE* file, ArithmeticLogicUnit* alu)
{
    // Declare a variable to store the origin, which indicates the starting address in memory to store the image
    uint16_t origin;

    // Read the origin value from the image file into the variable "origin"
    fread(&origin, sizeof(origin), 1, file);

    // Convert origin to little endian format
    origin = alu->Swap16(origin);

    // The maximum file size is known, allowing for a single fread operation.
    // With the predetermined limit, the file size will not exceed this value.
    // Thus, a single file read operation is sufficient.
    uint16_t maxRead = MEMORY_MAX - origin;

    // Pointer to the memory location to start reading from
    uint16_t* p = memory + origin;

    // Read the contents of the file into memory
    size_t read = fread(p, sizeof(uint16_t), maxRead, file);

    // Convert each read value to little endian format
    while (read-- > 0)
    {
        *p = alu->Swap16(*p);
        ++p; // Move to the next memory location
    }
}


/**
 * @brief Opens the specified image file and reads its contents into memory.
 *
 * This function opens the image file specified by the provided path and reads its contents into memory.
 *
 * @param imagePath The path to the image file to be read.
 * @return Returns 1 if the image file was successfully read into memory, 0 otherwise.
 */
int CPU::ReadImage(const char* imagePath, ArithmeticLogicUnit* alu)
{
    // Open the image file specified by the imagePath in binary read mode ("rb") and store the file pointer in "file"
    FILE* file = fopen(imagePath, "rb");

    // Check if the file was successfully opened. If not, return 0 to indicate failure.
    if (!file)
    {
        return 0;
    }

    // Call the ReadImageFile function to read the contents of the image file into memory
    ReadImageFile(file, alu);

    // Close the file stream to release resources
    fclose(file);

    // Return 1 to indicate that the image file was successfully read into memory
    return 1;
}