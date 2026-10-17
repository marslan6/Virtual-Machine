#ifndef MEMORYIO_H
#define MEMORYIO_H


#include <cstdint>


class OS;


enum MemoryMappedRegisters : std::uint16_t
{
	MR_KBSR = 0xFE00,
	MR_KBDR = 0xFE02
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