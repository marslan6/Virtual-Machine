#include "./../include/MemoryIO.h"
#include "./../include/CPU.h"
#include "./../include/OS.h"


MemoryIO::MemoryIO(std::uint16_t* memory, OS* os)
{
    memoryPtr = memory;
    osPtr = os;
}


std::uint16_t MemoryIO::Read(std::uint16_t memoryAddress)
{
    if (memoryAddress == MemoryMappedRegisters::MR_KBSR)
    {
        if (osPtr->CheckKey())
        {
            memoryPtr[MemoryMappedRegisters::MR_KBSR] = (1 << 15);
            memoryPtr[MemoryMappedRegisters::MR_KBDR] = getchar();
        }
        else
        {
            memoryPtr[MemoryMappedRegisters::MR_KBSR] = 0;
        }
    }

    return memoryPtr[memoryAddress];
}


void MemoryIO::Write(std::uint16_t address, std::uint16_t value)
{
    memoryPtr[address] = value;
}