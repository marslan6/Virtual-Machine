#ifndef MEMORYIO_H
#define MEMORYIO_H

#include <span>

#include "./../include/Lc3Types.h"
#include "CPU.h"

class CPU;
class OS;

class MemoryIO
{
public:
    // ===== Construction =====
    MemoryIO(OS* os);
    ~MemoryIO() = default;

    // ===== Memory access =====
    [[nodiscard]] Word16 Read(Address address);
    void Write(Address address, Word16 value);
    [[nodiscard]] size_t GetMemorySize();
    [[nodiscard]] std::span<Word16> ViewOfMemoryFrom(Address startAddress);

private:
    // ===== State =====
    std::array<Word16, Lc3Cpu::kMemoryMax> memory{};
    OS* os;
};

#endif // MEMORYIO_H