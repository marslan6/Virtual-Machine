#ifndef LC3_TYPES_H
#define LC3_TYPES_H

#include <cstddef>
#include <cstdint>
#include <string_view>

/// A 16-bit machine word: a register value, memory cell, or instruction.
using Word16 = std::uint16_t;

/// A 16-bit memory address into the LC-3 address space.
using Address = std::uint16_t;

/// Core architectural constants of the LC-3 machine.
namespace Lc3Cpu
{
    /// Size of main memory: 65536 addressable 16-bit words.
    inline constexpr std::size_t kMemoryMax = std::size_t{1} << 16;

    /// Total number of registers (R0-R7, PC, COND).
    inline constexpr Word16 kRegisterCount {10};

    /// Address the program counter starts at on reset.
    inline constexpr Word16 kPcStart {0x3000};

    /// Bit 15 is the sign bit of a 16-bit word; shift right by this to test it.
    inline constexpr std::uint16_t kSignBitShift {15};
}

/// Filesystem defaults for locating program image files.
namespace Lc3CpuImage
{
    /// Directory that program images are loaded from.
    inline constexpr std::string_view kImageDir {"images"};

    /// Default extension appended to an image name when none is given.
    inline constexpr std::string_view kImageExt {".obj"};
}

/// Addresses of the LC-3 memory-mapped device registers. Reads and writes to
/// these addresses touch hardware (the keyboard), not ordinary memory.
namespace Lc3MemoryIO
{
    /// Keyboard status: bit 15 is set when a key press is ready to be read.
    inline constexpr Address kKeyboardStatusAddress {0xFE00};

    /// Keyboard data: the ASCII value of the most recent key press.
    inline constexpr Address kKeyboardDataAddress {0xFE02};

    inline constexpr std::uint16_t kKeyboardReadyBit {15};
}

/// The LC-3 register file: eight general-purpose registers plus the
/// program counter and condition-flags register.
enum class Register : std::uint8_t
{
    R0, R1, R2, R3, R4, R5, R6, R7,
    PC,    // 8 - program counter
    COND,  // 9 - condition flags
};

/// Errors that can occur while loading a program image from disk.
enum class ImageError : std::uint8_t
{
    FileNotFound,    ///< No file exists at the resolved image path.
    FileNameInvalid, ///< The file is in invalid format, space included or empty
    CannotOpen,      ///< The file exists but could not be opened.
    InvalidImage,    ///< The file is too short or otherwise malformed.
};

/// Condition flags written into the COND register. Exactly one is set at a
/// time, reflecting whether the last result was positive, zero, or negative.
namespace Flag
{
    inline constexpr Word16 Positive {0x0001};
    inline constexpr Word16 Zero     {0x0002};
    inline constexpr Word16 Negative {0x0004};
}

#endif // LC3_TYPES_H