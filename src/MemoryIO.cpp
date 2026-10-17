#include "./../include/MemoryIO.h"
#include "./../include/CPU.h"
#include "./../include/OS.h"


MemoryIO::MemoryIO(std::span<Word16> memory, OS* os) : memory(memory)
{
    osPtr = os;
}


std::uint16_t MemoryIO::Read(const std::uint16_t memoryAddress)
{
    if (memoryAddress == Lc3MemoryIO::kKeyboardStatusAddress)
    {
        if (osPtr->CheckKey() > 0)
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = (1 << 15);
            memory[Lc3MemoryIO::kKeyboardDataAddress] = getchar();
        }
        else
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = 0U;
        }
    }

    return memory[memoryAddress];
}


void MemoryIO::Write(const std::uint16_t address, const std::uint16_t value)
{
    memory[address] = value;
}