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
/// @param os Non-owning pointer to the OS; must remain valid for reads.
MemoryIO::MemoryIO(OS* os)
    : os(os)
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

    if (os == nullptr)
    {
        throw std::runtime_error("MemoryIO::Read: OS pointer is null, keyboard polling unavailable");
    }

    if (address == Lc3MemoryIO::kKeyboardStatusAddress)
    {
        if (os->CheckKey())
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = static_cast<Word16>(1U << Lc3MemoryIO::kKeyboardReadyBit);
            memory[Lc3MemoryIO::kKeyboardDataAddress] = static_cast<Word16>(getchar());
        }
        else
        {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = 0;
        }
    }

    return memory.at(address);
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

    memory.at(address) = value;
}

/// @return The size of memory, in 16-bit words.
std::size_t MemoryIO::GetMemorySize()
{
    return memory.size();
}

/// Returns a view of memory from the given address to the end.
/// @param startAddress The first address the view covers.
/// @return A span over memory starting at startAddress.
/// @throws std::out_of_range if startAddress is outside the memory span.
std::span<Word16> MemoryIO::ViewOfMemoryFrom(Address startAddress)
{
    if (startAddress >= GetMemorySize())
    {
        throw std::out_of_range("MemoryIO: start address out of range");
    }

    return std::span<Word16>{memory}.subspan(startAddress);
}
