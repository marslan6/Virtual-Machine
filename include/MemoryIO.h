#ifndef MEMORYIO_H
#define MEMORYIO_H

#include <cstdint>
#include <span>

#include "./../include/Lc3Types.h"

class OS;

class MemoryIO
{
private:
	std::span<Word16> memory;
	OS* osPtr;

public:
	MemoryIO(std::span<Word16> memory, OS* os);
	~MemoryIO() = default;
	std::uint16_t Read(std::uint16_t memoryAddress);
	void Write(std::uint16_t address, std::uint16_t value);
};
#endif