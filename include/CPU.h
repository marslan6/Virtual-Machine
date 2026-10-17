#ifndef CPU_H
#define CPU_H

#include <cstdio>
#include <cstdint>
#include <bit>
#include <array>

#if defined(_WIN32)
    #include <Windows.h>
    #include <conio.h>
#endif

// LC-3 model: 65536 16-bit words of memory, 10 registers.
inline constexpr std::size_t   kMemoryMax     = std::size_t{1} << 16;  // 65536 words
inline constexpr std::uint16_t kRegisterCount = 10;

// Address the program counter starts at on reset.
inline constexpr std::uint16_t kPcStart = 0x3000;

// Condition flags written into the COND register. Exactly one is set at a
// time (result is positive, zero, or negative).
namespace flag
{
    inline constexpr std::uint16_t Positive = 0b001;
    inline constexpr std::uint16_t Zero     = 0b010;
    inline constexpr std::uint16_t Negative = 0b100;
}

class Trap;
class ArithmeticLogicUnit;
class MemoryIO;
class OS;

// Index of each register in the register file.
enum class Register : std::uint8_t
{
    R0, R1, R2, R3, R4, R5, R6, R7,
    PC,    // 8 - program counter
    COND,  // 9 - condition flags
};

class CPU
{
    std::array<std::uint16_t, kRegisterCount> registers{};
    std::array<std::uint16_t, kMemoryMax>     memory{};

    [[nodiscard]] std::uint16_t read_register(Register register_id) const noexcept;
    [[nodiscard]] std::uint16_t read_register(uint16_t register_id) const;
    void write_register(Register register_id, std::uint16_t value) noexcept;
    
    [[nodiscard]] std::uint16_t get_condition_flag() const noexcept;
    void set_condition_flag(std::uint16_t value) noexcept;

public:
    bool running = true;

    CPU();
    ~CPU();
    void UpdateFlags(const std::uint16_t register_id);
    void ReadImageFile(FILE* file, ArithmeticLogicUnit* alu);
    int  ReadImage(const char* imagePath, ArithmeticLogicUnit* alu);
};

#endif