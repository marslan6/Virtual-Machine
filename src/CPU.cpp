/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include <stdexcept>
#include <utility>
#include <filesystem>
#include <fstream>
#include <span>
#include <bit>

#include "./../include/Trap.h"
#include "./../include/ArithmeticLogicUnit.h"
#include "./../include/CPU.h"

namespace fs = std::filesystem;

// ========================
// ===== Construction =====
// ========================
/// Constructs the CPU in reset state: condition flag set to Zero and the
/// program counter initialized to the LC-3 start address (kPcStart).
CPU::CPU()
{
    SetConditionFlag(Flag::Zero);
    WriteRegister(Register::PC, Lc3Cpu::kPcStart);
}

// ========================
// ===== Run state ========
// ========================
/// @return true while the CPU is running; false once Stop() has been called.
bool CPU::IsRunning() const noexcept
{
    return running;
}

/// Halts the CPU. After this call IsRunning() returns false.
void CPU::Stop() noexcept
{
    running = false;
}

// =============================
// ===== Program execution =====
// =============================

/// Updates the condition (COND) register from the sign of a register's value,
/// setting exactly one of Zero, Negative, or Positive.
/// @param registerId Index of the register whose value is evaluated.
/// @throws std::out_of_range if registerId is not a valid register index.
void CPU::UpdateConditionFlags(std::uint16_t registerId)
{
    if (registerId >= Lc3Cpu::kRegisterCount)
    {
        throw std::out_of_range("Register index out of range!");
    }

    const Word16 registerValue = ReadRegister(registerId);
    if (registerValue == 0U)
    {
        // Result is zero.
        SetConditionFlag(Flag::Zero);
    }
    else if ((registerValue >> Lc3Cpu::kSignBitShift) == 1)
    {
        // Leftmost bit set => negative.
        SetConditionFlag(Flag::Negative);
    }
    else
    {
        // Neither zero nor negative => positive.
        SetConditionFlag(Flag::Positive);
    }
}

/// Loads an LC-3 object image into memory. The file's first word is the
/// big-endian load address; the remaining words are placed from there on and
/// converted to host byte order.
/// @param fileName Image name; kImageExt is appended if it has no extension.
/// @return Nothing on success, or an ImageError describing the failure.
std::expected<bool, ImageError> CPU::LoadImage(const std::string& fileName)
{
    if (fileName.empty() || fileName.contains(" "))
    {
        return std::unexpected(ImageError::FileNameInvalid);
    }

    fs::path imagePath = fs::path(Lc3CpuImage::kImageDir) / fileName;
    if (!imagePath.has_extension())
    {
        imagePath += Lc3CpuImage::kImageExt;
    }

    if (!fs::exists(imagePath))
    {
        return std::unexpected(ImageError::FileNotFound);
    }

    std::ifstream file(imagePath, std::ios::binary);
    if (!file)
    {
        return std::unexpected(ImageError::CannotOpen);
    }

    // First word is the load address (big-endian on disk).
    Address programLoadAddress{};
    if (!file.read(reinterpret_cast<char*>(&programLoadAddress), sizeof(programLoadAddress)))
    {
        return std::unexpected(ImageError::InvalidImage);
    }

    programLoadAddress = FromBigEndianToHostEndian(programLoadAddress);
    if (programLoadAddress >= memory.size())
    {
        return std::unexpected(ImageError::InvalidImage);
    }

    // The region of memory the binary loads into, from the load address on.
    std::span<Word16> binaryLoadedRegion = std::span<Word16>{memory}.subspan(programLoadAddress);
    file.read(reinterpret_cast<char*>(binaryLoadedRegion.data()), binaryLoadedRegion.size_bytes());
    const auto wordsRead = static_cast<std::size_t>(file.gcount()) / sizeof(Word16);

    for (Word16& word : binaryLoadedRegion.first(wordsRead))
    {
        word = FromBigEndianToHostEndian(word);
    }

    return true;
}

// ==========================
// ===== Static helpers =====
// ==========================

/// Converts a word from big-endian (image byte order on disk) to the host's
/// byte order. No-op when the host is already big-endian.
/// @param value Word16 as read from the image, in big-endian order.
/// @return The word in host byte order.
Word16 CPU::FromBigEndianToHostEndian(Word16 value) noexcept
{
    if constexpr (std::endian::native == std::endian::little)
    {
        return std::byteswap(value);
    }
    return value;
}

// ===========================
// ===== Register access =====
// ===========================

/// Reads a register by its typed identifier.
/// @param registerId The register to read.
/// @return The 16-bit value held in that register.
Word16 CPU::ReadRegister(Register registerId) const
{
    return registers.at(std::to_underlying(registerId));
}

/// Reads a register by raw index, e.g. a register number decoded from an
/// instruction. Validates the index at runtime.
/// @param registerId Register index in the range [0, kRegisterCount).
/// @return The 16-bit value held in that register.
/// @throws std::out_of_range if registerId is not a valid register index.
Word16 CPU::ReadRegister(std::uint16_t registerId) const
{
    if (registerId >= Lc3Cpu::kRegisterCount)
    {
        throw std::out_of_range("Register index out of range!");
    }
    return registers.at(registerId);
}

/// Writes a value into a register.
/// @param registerId The register to write.
/// @param value The 16-bit value to store.
void CPU::WriteRegister(Register registerId, Word16 value)
{
    registers.at(std::to_underlying(registerId)) = value;
}

/// Writes a value into a register.
/// @param registerId Register index in the range [0, kRegisterCount).
/// @param value The 16-bit value to store.
void CPU::WriteRegister(std::uint16_t registerId, Word16 value)
{
    if (registerId >= Lc3Cpu::kRegisterCount)
    {
        throw std::out_of_range("Register index out of range!");
    }
    
    registers.at(registerId) = value;
}

/// @return The current contents of the condition (COND) register.
Word16 CPU::GetConditionFlag() const
{
    return ReadRegister(Register::COND);
}

/// Sets the condition (COND) register.
/// @param value The condition-flag value to store (see namespace Flag).
void CPU::SetConditionFlag(Word16 value)
{
    WriteRegister(Register::COND, value);
}