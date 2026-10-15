/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#ifndef MEMORYIO_H
#define MEMORYIO_H


#include <cstdint>


class OS;


enum MemoryMappedRegisters : std::uint16_t
{
	MR_KBSR = 0xFE00, // keyboard status
	MR_KBDR = 0xFE02  // keyboard data
};


class MemoryIO
{
private:
	std::uint16_t* memoryPtr;
	OS* osPtr;

public:
	MemoryIO(std::uint16_t* memory, OS* os);

	std::uint16_t Read(std::uint16_t memoryAddress);
	void Write(std::uint16_t address, std::uint16_t value);
};
#endif