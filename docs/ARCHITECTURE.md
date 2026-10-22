# LC-3 Virtual Machine Architecture

This document is an in-depth technical overview of the LC-3 Virtual Machine
implementation: system design, data flow, instruction execution, and the
modern-C++ facilities the code relies on. It reflects the current C++23 /
GCC 15 codebase.

## Table of Contents
1. [System Overview](#system-overview)
2. [Component Architecture](#component-architecture)
3. [Instruction Execution Pipeline](#instruction-execution-pipeline)
4. [Memory Organization](#memory-organization)
5. [I/O System](#io-system)
6. [Implementation Details](#implementation-details)

---

## System Overview

### Design Philosophy

The VM is a set of small, single-responsibility classes that are constructed in
`main` and wired together by **dependency injection**: each component receives
non-owning raw pointers to the collaborators it needs. No component owns
another, and there are no globals for machine state.

```
┌──────────────────────────────────────────────────────┐
│                   VirtualMachine                     │
│          (orchestration & control flow)              │
└───────┬──────────────────────────────────────────────┘
        │ drives the fetch-decode-execute loop
        │
┌───────▼─────┬──────────┬──────────┬──────────┬────────┐
▼             ▼          ▼          ▼          ▼
CPU          ALU      MemoryIO     Trap        OS
(registers) (execute) (memory +   (services)  (platform
            + decode)  MMIO)                    console)
                       │
          shared vocabulary: include/Lc3Types.h
          (aliases, enum class types, constants)
```

### Key Design Principles

1. **Single responsibility** — each class owns one concern (CPU holds the
   register file; `MemoryIO` holds memory; `OS` isolates the platform).
2. **Dependency injection** — collaborators are passed as constructor
   arguments (non-owning pointers), so components can be wired or replaced
   independently.
3. **Encapsulation** — internal state is private and reached only through
   methods (`ReadRegister` / `WriteRegister`, `Read` / `Write`); there is no
   raw array access from outside a class.
4. **Type safety** — scoped enums (`enum class`), fixed-width aliases
   (`Word16`, `Address`), and bounds-checked containers (`std::array::at`)
   replace raw integers and C arrays.
5. **Cross-platform** — all OS-specific code is confined to `OS`, selected at
   compile time (`_WIN32` vs. `__linux__`).

Shared types live in [`include/Lc3Types.h`](../include/Lc3Types.h):

```cpp
using Word16  = std::uint16_t;   // a machine word / register / memory cell
using Address = std::uint16_t;   // a 16-bit address

enum class Register : std::uint8_t { R0, R1, R2, R3, R4, R5, R6, R7, PC, COND };
enum class Opcode   : Word16 { BR = 0, ADD, LD, ST, JSR, AND, LDR, STR,
                               RTI, NOT, LDI, STI, JMP, RES, LEA, TRAP };
enum class ImageError : std::uint8_t { FileNotFound, FileNameInvalid,
                                       CannotOpen, InvalidImage };

namespace Flag {
    inline constexpr Word16 Positive {0x0001};
    inline constexpr Word16 Zero     {0x0002};
    inline constexpr Word16 Negative {0x0004};
}
```

---

## Component Architecture

### 1. CPU

**Files:** [`include/CPU.h`](../include/CPU.h), [`src/CPU.cpp`](../src/CPU.cpp)

**Responsibilities:** register file, condition flags, run state, and image
loading. Memory itself lives in `MemoryIO`; the CPU reaches it through its
injected `MemoryIO*`.

#### State

```cpp
class CPU {
    // ...
private:
    std::array<Word16, Lc3Cpu::kRegisterCount> registers{};  // 10 registers
    MemoryIO* memoryIO;                                        // non-owning
    bool running = true;
};
```

The constructor puts the CPU in a reset state — COND set to `Zero`, PC set to
the LC-3 start address `0x3000`:

```cpp
CPU::CPU(MemoryIO* memoryIO) : memoryIO(memoryIO) {
    SetConditionFlag(Flag::Zero);
    WriteRegister(Register::PC, Lc3Cpu::kPcStart);
}
```

#### Register access

Registers are reached only through accessors, which bounds-check with
`std::array::at`. There are two overloads — one typed (`Register`) and one by
raw index (for a register number decoded from an instruction):

```cpp
Word16 CPU::ReadRegister(Register registerId) const {
    return registers.at(std::to_underlying(registerId));   // C++23
}

Word16 CPU::ReadRegister(std::uint16_t registerId) const {
    if (registerId >= Lc3Cpu::kRegisterCount)
        throw std::out_of_range("Register index out of range!");
    return registers.at(registerId);
}
```

#### Condition flags

`UpdateConditionFlags` sets exactly one of `Zero` / `Negative` / `Positive`
from the sign of a register's value (bit 15 is the sign bit):

```cpp
void CPU::UpdateConditionFlags(std::uint16_t registerId) {
    const Word16 value = ReadRegister(registerId);
    if (value == 0U)                                  SetConditionFlag(Flag::Zero);
    else if ((value >> Lc3Cpu::kSignBitShift) == 1)   SetConditionFlag(Flag::Negative);
    else                                              SetConditionFlag(Flag::Positive);
}
```

#### Image loading

`LoadImage` reports failures with `std::expected<bool, ImageError>` instead of
return codes or exceptions. It resolves the name under `kImageDir` (`images/`),
appends `.obj` when no extension is given, reads the big-endian load address,
copies the rest into a `std::span` view of memory, and converts each word to
host byte order:

```cpp
std::expected<bool, ImageError> CPU::LoadImage(const std::string& fileName) {
    if (fileName.empty() || fileName.contains(" "))
        return std::unexpected(ImageError::FileNameInvalid);

    fs::path imagePath = fs::path(Lc3CpuImage::kImageDir) / fileName;
    if (!imagePath.has_extension()) imagePath += Lc3CpuImage::kImageExt;
    if (!fs::exists(imagePath))     return std::unexpected(ImageError::FileNotFound);

    std::ifstream file(imagePath, std::ios::binary);
    if (!file)                      return std::unexpected(ImageError::CannotOpen);

    Address loadAddress{};
    if (!file.read(reinterpret_cast<char*>(&loadAddress), sizeof(loadAddress)))
        return std::unexpected(ImageError::InvalidImage);
    loadAddress = FromBigEndianToHostEndian(loadAddress);
    if (loadAddress >= memoryIO->GetMemorySize())
        return std::unexpected(ImageError::InvalidImage);

    std::span<Word16> region = memoryIO->ViewOfMemoryFrom(loadAddress);
    file.read(reinterpret_cast<char*>(region.data()), region.size_bytes());
    const auto wordsRead = static_cast<std::size_t>(file.gcount()) / sizeof(Word16);
    for (Word16& word : region.first(wordsRead))
        word = FromBigEndianToHostEndian(word);

    return true;
}
```

Byte-order conversion is resolved at compile time via `if constexpr` and
`std::endian`, using the C++23 `std::byteswap`:

```cpp
Word16 CPU::FromBigEndianToHostEndian(Word16 value) noexcept {
    if constexpr (std::endian::native == std::endian::little)
        return std::byteswap(value);
    return value;                     // host already big-endian: no-op
}
```

---

### 2. ALU (Arithmetic Logic Unit)

**Files:** [`include/ArithmeticLogicUnit.h`](../include/ArithmeticLogicUnit.h),
[`src/ArithmeticLogicUnit.cpp`](../src/ArithmeticLogicUnit.cpp)

**Responsibilities:** one handler per instruction, plus static field decoders.
The ALU never touches registers or memory directly — it calls
`cpu->ReadRegister` / `cpu->WriteRegister` and `memoryIO->Read` / `Write`.

#### Instruction handlers

Each opcode has a method (`ADD`, `AND`, `NOT`, `BR`, `JMP`, `JSR`, `LD`, `LDR`,
`LDI`, `LEA`, `ST`, `STR`, `STI`). Example — `ADD`:

```cpp
void ArithmeticLogicUnit::ADD(Word16 instruction) {
    std::uint16_t DR  = GetDestinationRegister(instruction);  // bits [11:9]
    std::uint16_t SR1 = GetSource1Register(instruction);      // bits [8:6]

    if (GetImmediateFlag(instruction)) {                      // bit [5]
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + GetImmediate5(instruction));
    } else {
        std::uint16_t SR2 = GetSource2Register(instruction);  // bits [2:0]
        cpu->WriteRegister(DR, cpu->ReadRegister(SR1) + cpu->ReadRegister(SR2));
    }
    cpu->UpdateConditionFlags(DR);
}
```

`BR` branches when the instruction's condition codes overlap the COND register:

```cpp
void ArithmeticLogicUnit::BR(Word16 instruction) {
    std::uint16_t pcOffset      = GetPcOffset9(instruction);
    std::uint16_t conditionFlag = (instruction >> 9) & 0x0007;
    if ((conditionFlag & cpu->ReadRegister(Register::COND)) != 0)
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + pcOffset);
}
```

`JMP` doubles as `RET` (jump to the base register, R7 for `RET`); `JSR` saves
the return address in R7 and jumps by an 11-bit offset (`JSR`) or to a base
register (`JSRR`).

#### Field decoders

Instruction fields are decoded by `static` helpers, keeping the handlers
declarative. `SignExtend` extends a narrow value to 16 bits for 2's-complement
arithmetic:

```cpp
std::uint16_t ArithmeticLogicUnit::SignExtend(std::uint16_t value,
                                              std::uint32_t bitCount) noexcept {
    if (((value >> (bitCount - 1)) & 0x0001) != 0)            // sign bit set
        return static_cast<std::uint16_t>(value | (0xFFFF << bitCount));
    return value;
}
```

Other decoders: `GetDestinationRegister` (`[11:9]`), `GetSource1Register`
(`[8:6]`), `GetSource2Register` (`[2:0]`), `GetImmediateFlag` (`[5]`),
`GetImmediate5`, `GetPcOffset9`, and `GetOffset6` (the last three sign-extend).

---

### 3. MemoryIO

**Files:** [`include/MemoryIO.h`](../include/MemoryIO.h),
[`src/MemoryIO.cpp`](../src/MemoryIO.cpp)

**Responsibilities:** owns the 65,536-word memory array and mediates every
access, transparently handling memory-mapped keyboard I/O.

```cpp
class MemoryIO {
    // ...
private:
    std::array<Word16, Lc3Cpu::kMemoryMax> memory{};   // 64K words
    OS* os;                                             // non-owning
};
```

`Read` is non-`const`: reading the keyboard-status address (`0xFE00`) polls the
keyboard through the OS and updates the status/data registers. Every access is
bounds-checked:

```cpp
Word16 MemoryIO::Read(const Address address) {
    if (address >= memory.size())
        throw std::out_of_range("MemoryIO::Read: address exceeds memory size");
    if (os == nullptr)
        throw std::runtime_error("MemoryIO::Read: OS pointer is null");

    if (address == Lc3MemoryIO::kKeyboardStatusAddress) {
        if (os->CheckKey()) {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] =
                static_cast<Word16>(1U << Lc3MemoryIO::kKeyboardReadyBit);
            memory[Lc3MemoryIO::kKeyboardDataAddress] =
                static_cast<Word16>(getchar());
        } else {
            memory[Lc3MemoryIO::kKeyboardStatusAddress] = 0;
        }
    }
    return memory.at(address);
}
```

`ViewOfMemoryFrom(startAddress)` hands out a `std::span<Word16>` from an address
to the end of memory — used by the image loader and by the trap string routines,
so neither reaches into the array directly.

---

### 4. Trap

**Files:** [`include/Trap.h`](../include/Trap.h), [`src/Trap.cpp`](../src/Trap.cpp)

**Responsibilities:** the six LC-3 service routines, dispatched from the `TRAP`
instruction. Output uses C++ iostreams (`std::cout` / `std::cin`), not C stdio.

`Proxy` preserves the return address in R7 (routines may clobber the PC) and
switches on the trap vector (the low byte of the instruction):

```cpp
void Trap::Proxy(Word16 instruction) {
    cpu->WriteRegister(Register::R7, cpu->ReadRegister(Register::PC));
    switch (static_cast<TrapVector>(instruction & Lc3Trap::kLowByteMask)) {
        case TrapVector::GetChar:    GETC();  break;
        case TrapVector::Out:        OUTC();  break;
        case TrapVector::PutString:  PUTS();  break;
        case TrapVector::In:         INC();   break;
        case TrapVector::PutStringP: PUTSP(); break;
        case TrapVector::Halt:       HALT();  break;
        default: throw std::runtime_error("Trap::Proxy: unknown trap vector");
    }
}
```

| Vector | Routine | Behavior |
|--------|---------|----------|
| `0x20` | `GETC`  | Read one char (not echoed) into R0 |
| `0x21` | `OUTC`  | Write the char in R0 |
| `0x22` | `PUTS`  | Write a one-char-per-word string from `[R0]` until a 0 word |
| `0x23` | `INC`   | Prompt, read one char (echoed) into R0 |
| `0x24` | `PUTSP` | Write a two-chars-per-word (packed) string from `[R0]` |
| `0x25` | `HALT`  | Print `HALT` and call `cpu->Stop()` |

The string routines walk a span rather than a raw pointer:

```cpp
std::span<Word16> Trap::StringAtR0() {
    return memoryIO->ViewOfMemoryFrom(cpu->ReadRegister(Register::R0));
}

void Trap::PUTS() {
    for (const Word16 word : StringAtR0()) {
        if (word == 0) break;
        std::cout << static_cast<char>(word);
    }
    std::cout << std::flush;
}
```

---

### 5. OS (platform layer)

**Files:** [`include/OS.h`](../include/OS.h), [`src/OS.cpp`](../src/OS.cpp)

**Responsibilities:** console input mode, non-blocking keyboard polling, and
interrupt handling. This is the only platform-specific code; an unsupported
platform fails the build with `#error`.

```cpp
#if defined(_WIN32)
    #include <windows.h>
#elif defined(__linux__)
    #include <termios.h>
    #include <unistd.h>
#else
    #error "Unsupported Operating System!"
#endif
```

`DisableInputBuffering` switches the console to unbuffered, no-echo input and
saves the previous mode; `RestoreInputBuffering` puts it back. On Windows this
uses the Console API (`GetConsoleMode` / `SetConsoleMode`); on Linux it uses
`termios` (clearing `ICANON | ECHO`). `CheckKey` polls without blocking —
`WaitForSingleObject` + `_kbhit` on Windows, a zero-timeout `select` on Linux —
and returns `bool`:

```cpp
bool OS::CheckKey() noexcept {
#if defined(_WIN32)
    return (WaitForSingleObject(hStdin, 0) == WAIT_OBJECT_0) && _kbhit();
#elif defined(__linux__)
    fd_set readfds; FD_ZERO(&readfds); FD_SET(hStdin, &readfds);
    timeval timeout{};                          // zero timeout: poll, don't block
    return select(hStdin + 1, &readfds, nullptr, nullptr, &timeout) > 0;
#endif
}
```

Interrupts (Ctrl+C) are handled safely: the signal handler only sets an
async-signal-safe flag, which the main loop observes through normal control
flow:

```cpp
volatile std::sig_atomic_t OS::interruptRequested = 0;

void OS::HandleInterruptWrapper([[maybe_unused]] int signalNumber) noexcept {
    interruptRequested = 1;
}
```

---

## Instruction Execution Pipeline

### Fetch-Decode-Execute Cycle

```
┌─────────────┐
│   FETCH     │  read memory[PC], then PC = PC + 1
└──────┬──────┘
       ▼
┌─────────────┐
│   DECODE    │  opcode = instruction >> 12   (bits [15:12])
└──────┬──────┘
       ▼
┌─────────────┐
│  EXECUTE    │  dispatch to the ALU handler or Trap::Proxy
└──────┬──────┘
       ▼
┌─────────────┐
│   FLAGS     │  the handler updates COND when it writes a register
└──────┬──────┘
       └──────→ repeat while running and no interrupt
```

### Main Execution Loop

`RunVirtualMachine` takes the command line as a `std::span<const char*>`, loads
every named image, installs the interrupt handler, enables raw input through an
RAII guard, then runs the loop. An unused opcode (`RTI`, `RES`) throws.

```cpp
void VirtualMachine::RunVirtualMachine(std::span<const char*> args) {
    if (args.size() < 2) throw std::runtime_error("no image file provided");

    for (const char* imagePath : args.subspan(1))            // args[0] is argv[0]
        if (!cpu->LoadImage(imagePath))
            throw std::runtime_error("failed to load image file");

    std::signal(SIGINT, OS::HandleInterruptWrapper);
    ScopedRawInput rawInput(os);                             // RAII: restores on exit

    while (cpu->IsRunning() && !OS::InterruptRequested()) {
        Word16 instruction = memoryIO->Read(cpu->ReadRegister(Register::PC));
        cpu->WriteRegister(Register::PC, cpu->ReadRegister(Register::PC) + 1);

        switch (static_cast<Opcode>(instruction >> 12)) {
            case Opcode::ADD:  alu->ADD(instruction);  break;
            case Opcode::AND:  alu->AND(instruction);  break;
            // ... remaining ALU opcodes ...
            case Opcode::TRAP: trap->Proxy(instruction); break;
            case Opcode::RTI:
            case Opcode::RES:
            default: throw std::runtime_error("illegal opcode");
        }
    }
}
```

`ScopedRawInput` is a small RAII guard defined in an anonymous namespace: it
disables input buffering in its constructor and restores it in its destructor,
so the terminal is reset on **every** exit path, including stack unwinding from
a thrown exception.

---

## Memory Organization

### Memory Layout

```
Address Range    │ Size   │ Contents
─────────────────┼────────┼──────────────────────────
0x0000 - 0x00FF  │ 256 B  │ Trap Vector Table
0x0100 - 0x01FF  │ 256 B  │ Interrupt Vector Table
0x0200 - 0x2FFF  │ 11.5KB │ OS / Supervisor Space
0x3000 - 0xFDFF  │ 51.5KB │ User Program Space  (PC starts at 0x3000)
0xFE00 - 0xFE01  │ 2 B    │ Keyboard Status (KBSR)
0xFE02 - 0xFE03  │ 2 B    │ Keyboard Data   (KBDR)
0xFE04 - 0xFFFF  │ 506 B  │ Device Registers (reserved)
```

Memory is a single `std::array<Word16, 65536>` owned by `MemoryIO`. The
device-register addresses and the keyboard-ready bit are named constants in the
`Lc3MemoryIO` namespace of `Lc3Types.h`.

### Addressing Modes

```
LD  R0, LABEL     ; R0 = memory[PC + sext(offset9)]          (PC-relative)
LDI R0, LABEL     ; R0 = memory[memory[PC + sext(offset9)]]  (indirect)
LDR R0, R1, #6    ; R0 = memory[R1 + sext(offset6)]          (base + offset)
```

---

## I/O System

### Input Flow

```
Keyboard ──▶ OS::CheckKey() ──▶ MemoryIO::Read(0xFE00) ──▶ program
 keypress    non-blocking poll   sets KBSR ready bit,        reads KBSR,
                                 stores char in KBDR          reads KBDR
```

A program reads `KBSR` (`0xFE00`); the `Read` interceptor polls the keyboard and,
if a key is ready, sets bit 15 and loads the character into `KBDR` (`0xFE02`).

### Output Flow

```
program ──▶ Trap::Proxy() ──▶ OUTC/PUTS/PUTSP ──▶ std::cout ──▶ console
 TRAP        dispatch          routine             (flushed)
```

Output traps write through `std::cout` and flush, so characters appear
immediately even in raw-input mode.

---

## Implementation Details

### Modern C++ facilities used

| Standard | Facility | Role in the VM |
|----------|----------|----------------|
| C++23 | `std::expected` / `std::unexpected` | Typed image-load errors from `LoadImage` |
| C++23 | `std::byteswap` | Endian conversion of image words |
| C++23 | `std::to_underlying` | Register/enum → index without a cast |
| C++20 | `std::span` | Views over `argv` and memory regions |
| C++20 | `std::endian` | Compile-time host-endianness check |
| C++17 | `if constexpr`, inline `constexpr`, `[[nodiscard]]` | Compile-time branches, header constants, attribute hygiene |
| C++11 | `enum class`, `std::array`, `noexcept`, `= default`/`= delete` | Type-safe enums, bounds-checked storage, RAII guard |

See the project [README](../README.md#modern-c-features) for the full table.

### Current limitations

1. **No interrupt instruction** — `RTI` is reserved; executing it throws.
2. **Single-threaded** — one fetch-decode-execute loop, no concurrency.
3. **No privilege modes** — all code runs at one level.

### Extension points

- **New devices:** add memory-mapped addresses in `Lc3Types.h` and extend
  `MemoryIO::Read` / `Write`.
- **New services:** add a `TrapVector` entry and a routine in `Trap`.
- **Debugging:** hook the main loop in `RunVirtualMachine` for breakpoints,
  single-step, and register/memory inspection.

---

_This document tracks the C++23 implementation; keep it in sync with the sources
under [`src/`](../src) and [`include/`](../include)._
