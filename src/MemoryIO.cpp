#include "./../include/MemoryIO.h"
#include "./../include/CPU.h"
#include "./../include/OS.h"

#include <stdexcept>
#include <cstdio>

// ========================
// ===== Construction =====
// ========================

/// Constructs the memory interface over an existing memory span and a
/// non-owning pointer to the OS used for keyboard polling.
/// @param memory View of the machine's memory; must outlive this MemoryIO.
/// @param os Non-owning pointer to the OS; must remain valid for reads.
MemoryIO::MemoryIO(std::span<Word16> memory, OS* os)
    : memory(memory), osPtr(os)
{
}

// ============================
// ====== Memory access =======
// ============================

/// Reads a word from memory. Not const: reading the keyboard-status address
/// polls the keyboard and updates the memory-mapped status/data registers.
/// @param address Address to read, in the range [0, memory.size()).
/// @return The word stored at the address.
/// @throws std::out_of_range if address is outside the memory span.
/// @throws std::runtime_error if the OS pointer is null.
Word16 MemoryIO::Read(const Address address)
{
    if (address >= memory.size())
    {
        throw std::out_of_range("MemoryIO::Read: address exceeds memory size");
    }

    if (osPtr == nullptr)
    {
        throw std::runtime_error("MemoryIO::Read: OS pointer is null, keyboard polling unavailable");
    }

    if (address == Lc3MemoryIO::kKeyboardStatusAddress)
    {
        if (osPtr->CheckKey())
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = static_cast<Word16>(1U << Lc3MemoryIO::kKeyboardReadyBit);
            memory[Lc3MemoryIO::kKeyboardDataAddress] = static_cast<Word16>(getchar());
        }
        else
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = 0;
        }
    }

    return memory[address];
}

/// Writes a word to memory.
/// @param address Address to write, in the range [0, memory.size()).
/// @param value The word to store.
/// @throws std::out_of_range if address is outside the memory span.
void MemoryIO::Write(const Address address, const Word16 value)
{
    if (address >= memory.size())
    {
        throw std::out_of_range("MemoryIO::Write: address exceeds memory size");
    }

    memory[address] = value;
}