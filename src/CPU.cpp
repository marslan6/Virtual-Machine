/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#include <exception>
#include <stdexcept>
#include <utility>
#define _CRT_SECURE_NO_DEPRECATE


#include <iostream>


#include "./../include/Trap.h"
#include "./../include/MemoryIO.h"
#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/OS.h"
#include "./../include/CPU.h"

CPU::CPU()
{
    set_condition_flag(flag::Zero);
    write_register(Register::PC, kPcStart);
}

CPU::~CPU()
{
}

std::uint16_t CPU::read_register(Register register_id) const noexcept
{
    return registers.at(std::to_underlying(register_id));
}

std::uint16_t CPU::read_register(uint16_t register_id) const
{
    if (register_id >= kRegisterCount)
    {
        throw std::out_of_range("Register index out of range!");
    }
    
    return registers.at(register_id);
}

void CPU::write_register(Register register_id, std::uint16_t value) noexcept
{
    registers.at(std::to_underlying(register_id)) = value;
}

std::uint16_t CPU::get_condition_flag() const noexcept        
{ 
    return read_register(Register::COND); 
}

void CPU::set_condition_flag(std::uint16_t value) noexcept
{ 
    write_register(Register::COND, value); 
}

void CPU::UpdateFlags(const std::uint16_t register_id)
{
    if (register_id < 0 || register_id >= kRegisterCount)
    {
        throw std::out_of_range("Register index out of range!");
    }

    if (read_register(register_id) == 0U)
    {
        // Set the zero flag if the value in the register is zero
        set_condition_flag(flag::Zero);
    }
    else if ((read_register(register_id) >> 15) == 1)
    {
        // Set the negative flag if the leftmost bit is 1
        set_condition_flag(flag::Negative);
    }
    else
    {
        // Set the positive flag if neither zero nor negative conditions are met
        set_condition_flag(flag::Positive);
    }
}

void CPU::ReadImageFile(FILE* file, ArithmeticLogicUnit* alu)
{
    // Declare a variable to store the origin, which indicates the starting address in memory to store the image
    std::uint16_t origin;

    // Read the origin value from the image file into the variable "origin"
    fread(&origin, sizeof(origin), 1, file);

    // Convert origin to little endian format
    origin = alu->Swap16(origin);

    // The maximum file size is known, allowing for a single fread operation.
    // With the predetermined limit, the file size will not exceed this value.
    // Thus, a single file read operation is sufficient.
    std::uint16_t maxRead = MEMORY_MAX - origin;

    // Pointer to the memory location to start reading from
    std::uint16_t* p = memory + origin;

    // Read the contents of the file into memory
    size_t read = fread(p, sizeof(std::uint16_t), maxRead, file);

    // Convert each read value to little endian format
    while (read-- > 0)
    {
        *p = alu->Swap16(*p);
        ++p; // Move to the next memory location
    }
}

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