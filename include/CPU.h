#ifndef CPU_H
#define CPU_H

#include <cstdint>
#include <array>
#include <expected>
#include <string>

#include "./../include/Lc3Types.h"

class Trap;
class ArithmeticLogicUnit;
class MemoryIO;
class OS;

class CPU
{
public:
    // ===== Construction =====
    CPU();
    ~CPU() = default;

    // ===== Run state =====
    [[nodiscard]] bool IsRunning() const noexcept;
    void Stop() noexcept;

    // ===== Program execution =====
    void UpdateConditionFlags(std::uint16_t registerId);
    std::expected<void, ImageError> LoadImage(const std::string& fileName);

    // ===== Register access =====
    [[nodiscard]] Word16 ReadRegister(Register registerId) const;
    [[nodiscard]] Word16 ReadRegister(std::uint16_t registerId) const;
    void WriteRegister(Register registerId, Word16 value);
    void WriteRegister(std::uint16_t registerId, Word16 value);

private:
    // ===== Static helpers =====
    [[nodiscard]] static Word16 FromBigEndianToHostEndian(Word16 value) noexcept;
    [[nodiscard]] Word16 GetConditionFlag() const;
    void SetConditionFlag(Word16 value);

    // ===== State =====
    std::array<Word16, Lc3Cpu::kRegisterCount> registers{};
    std::array<Word16, Lc3Cpu::kMemoryMax>     memory{};
    bool running = true;
};

#endif // CPU_H