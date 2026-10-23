# LC-3 Virtual Machine Emulator

[![Platform](https://img.shields.io/badge/Platform-Linux%20%7C%20Windows-blue)](https://en.wikipedia.org/wiki/Cross-platform_software)
[![Language](https://img.shields.io/badge/C%2B%2B-23-00599C)](https://isocpp.org/)
[![Compiler](https://img.shields.io/badge/GCC-15-orange)](https://gcc.gnu.org/)
[![Build](https://img.shields.io/badge/Build-CMake%203.20%2B-064F8C)](https://cmake.org/)
[![Architecture](https://img.shields.io/badge/Architecture-LC--3-red)](https://en.wikipedia.org/wiki/Little_Computer_3)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![CI](https://github.com/marslan6/Virtual-Machine/actions/workflows/ci.yml/badge.svg)](https://github.com/marslan6/Virtual-Machine/actions/workflows/ci.yml)

A software implementation of the **LC-3 (Little Computer 3)** 16-bit
computer architecture, written in **modern C++23**. The emulator executes
compiled LC-3 object files and ships with two playable programs: **2048** and
**Rogue**.

The codebase has moved well beyond its original C++11 / Windows-only form: it is
now cross-platform (Linux and Windows), built with CMake and GCC 15, organized
around small single-responsibility components wired together by dependency
injection, and it leans on current standard-library facilities
(`std::expected`, `std::span`, `std::byteswap`, `std::endian`). See the
[Modern C++ Features](#modern-c-features) table for the full list.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Technical Details](#technical-details)
- [Included Programs](#included-programs)
- [**Used Modern C++ Features**](#modern-c-features)
- [Tooling](#tooling)
- [Documentation](#documentation)
- [Author](#author)
- [License](#license)

---

## Overview

The LC-3 Virtual Machine is an **instruction set simulator** that emulates the
complete functionality of the LC-3 computer architecture. It provides a runtime
environment for executing LC-3 binary programs with full support for:

- **15 LC-3 opcodes** (arithmetic, logical, control flow, memory operations)
- **Memory-mapped I/O** for keyboard input
- **Trap routines** for system services (character and string I/O)
- **Real-time console interaction** with unbuffered (raw) input
- **Binary image loading** with host byte-order conversion

This project demonstrates:
- Low-level computer architecture implementation
- The instruction fetch-decode-execute cycle
- Memory management and addressing modes
- System-level, cross-platform programming with I/O handling
- Clean component design via dependency injection and RAII

---

## Features

### Core Functionality
- **Complete LC-3 ISA**: all 15 opcodes implemented
- **10 registers**: 8 general-purpose (R0-R7), Program Counter (PC), Condition flags (COND)
- **128 KB addressable memory**: 65,536 × 16-bit words
- **Condition-flag system**: automatic POSITIVE / ZERO / NEGATIVE updates
- **Multiple addressing modes**: register, immediate, PC-relative, indirect, base+offset
- **Image loader** with typed error reporting via `std::expected<bool, ImageError>`

### I/O Capabilities
- **Memory-mapped keyboard**: real-time input via `KBSR`/`KBDR` (`0xFE00`/`0xFE02`)
- **Console output**: character, null-terminated string, and packed-string routines
- **Unbuffered input**: immediate key response with echo disabled
- **Six trap routines**: `GETC`, `OUT`, `PUTS`, `IN`, `PUTSP`, `HALT`

### Engineering
- **Cross-platform I/O layer**: Windows Console API and POSIX `termios`, selected at compile time
- **Dependency injection**: components are constructed in `main` and wired by non-owning pointers
- **RAII**: raw-input mode is restored on every exit path (including exceptions) via a scope guard
- **Compile-time endianness handling**: `if constexpr (std::endian::native …)` with `std::byteswap`
- **Graceful Ctrl+C handling**: async-signal-safe interrupt flag (`volatile std::sig_atomic_t`)

---

## Architecture

### System Components

The VM is composed of small, single-responsibility classes. `main` constructs
each one and injects its dependencies as raw (non-owning) pointers; no component
owns another.

```
                       ┌─────────────────────────────┐
                       │       VirtualMachine         │
                       │  fetch → decode → dispatch   │
                       └──────────────┬──────────────┘
          ┌───────────────┬───────────┼───────────┬───────────────┐
          ▼               ▼           ▼           ▼               ▼
     ┌─────────┐  ┌──────────────┐ ┌─────────┐ ┌────────┐   ┌──────────┐
     │   CPU   │  │     ALU      │ │ MemoryIO│ │  Trap  │   │    OS    │
     │ regs +  │  │ 13 handlers  │ │ 64K mem │ │ 6 svc  │   │ console  │
     │ flags + │  │ sign-extend  │ │ + MMIO  │ │ routin.│   │ + raw io │
     │ images  │  │ field decode │ │ keyboard│ │        │   │ + signal │
     └─────────┘  └──────────────┘ └─────────┘ └────────┘   └──────────┘
              shared vocabulary: include/Lc3Types.h (enums, aliases, constants)
```

#### `VirtualMachine`
Orchestrates execution. Validates arguments, loads the program image(s), installs
the interrupt handler, and runs the fetch-decode-execute loop, dispatching each
opcode to the ALU or the trap handler until the CPU halts, an interrupt arrives,
or an illegal opcode is hit (which throws).

#### `CPU`
Owns the register file (`std::array<Word16, 10>`), the run flag, condition-flag
logic, and image loading (`LoadImage` → `std::expected<bool, ImageError>`, with
big-endian → host conversion).

#### `ArithmeticLogicUnit` (ALU)
Implements the instruction handlers and the instruction-field decoders
(destination/source registers, immediate flag, sign-extended offsets).

| Category | Instructions |
|----------|--------------|
| **Arithmetic / Logic** | `ADD`, `AND`, `NOT` |
| **Control Flow** | `BR`, `JMP`/`RET`, `JSR`/`JSRR` |
| **Memory Load** | `LD`, `LDR`, `LDI`, `LEA` |
| **Memory Store** | `ST`, `STR`, `STI` |

> `TRAP` is handled by `Trap`; `RTI` and the reserved opcode throw as illegal.

#### `MemoryIO`
Owns the 65,536-word memory array and mediates all reads/writes, including
memory-mapped keyboard polling at `KBSR`/`KBDR`. Exposes `std::span` views of
memory (`ViewOfMemoryFrom`) for the loader and trap routines.

#### `Trap`
Dispatches the `TRAP` instruction to one of six service routines:

| Trap | Vector | Function |
|------|--------|----------|
| `GETC`  | `0x20` | Read one char (not echoed) |
| `OUT`   | `0x21` | Write one char |
| `PUTS`  | `0x22` | Write a one-char-per-word string |
| `IN`    | `0x23` | Prompt and read one char (echoed) |
| `PUTSP` | `0x24` | Write a two-chars-per-word (packed) string |
| `HALT`  | `0x25` | Stop execution |

#### `OS`
The platform layer. Enables/restores raw console input and polls the keyboard,
using the **Windows Console API** on `_WIN32` and POSIX **`termios`** on
`__linux__` (an unsupported platform fails the build with `#error`). Also owns the
`SIGINT` handler and the interrupt flag.

---

## Project Structure

```
Virtual-Machine/
├── README.md               # This file
├── LICENSE                 # MIT License
├── CMakeLists.txt          # CMake build (C++23, GCC 15, -Wall -Wextra)
├── makefile                # Convenience wrapper around CMake (build/run targets)
├── .clang-tidy             # clang-tidy check configuration
├── llvm.sh                 # LLVM/clang toolchain installer helper
│
├── include/                # Public headers
│   ├── Lc3Types.h          # Shared types: enums, aliases, architectural constants
│   ├── VirtualMachine.h    # Execution orchestrator
│   ├── CPU.h               # Registers, flags, image loading
│   ├── ArithmeticLogicUnit.h # Instruction handlers + field decoders
│   ├── MemoryIO.h          # Memory + memory-mapped I/O
│   ├── Trap.h              # Trap dispatch + routines
│   └── OS.h                # Cross-platform console / signal layer
│
├── src/                    # Implementations (one .cpp per header + main.cpp)
│   └── main.cpp            # Entry point: constructs and wires all components
│
├── images/                 # Program images the VM loads from (kImageDir)
│   ├── 2048.obj            # 2048 game (2,276 bytes)
│   └── rogue.obj           # Rogue game (762 bytes)
│
├── tests/                  # Test fixtures / sample images
├── bench/                  # Benchmarks (placeholder)
├── data/                   # Runtime data (placeholder)
├── docs/                   # Documentation (see Documentation section)
└── out/                    # CMake build output (executableBinary)
```

> **Image lookup:** `CPU::LoadImage` resolves a name against `kImageDir`
> (`images/`) and appends `kImageExt` (`.obj`) when no extension is given — so
> `2048` resolves to `images/2048.obj`. Pass the bare name, not a path.

---

## Getting Started

### Prerequisites

- **OS**: Linux or Windows
- **Compiler**: GCC 15 (`g++-15`) — the standard is set to C++23 and is required
- **Build system**: CMake ≥ 3.20
- **Standard library**: a libstdc++ providing `<expected>`, `<span>`, `<bit>`

> The CMake toolchain is pinned to `gcc-15`/`g++-15`. To build with a different
> compiler, adjust `CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` in
> [CMakeLists.txt](CMakeLists.txt). The optional [llvm.sh](llvm.sh) helper can
> install an LLVM/clang toolchain.

### Building

The simplest path is the `makefile`, a thin wrapper over CMake:

```bash
make            # configure + build into out/
make rebuild    # clean reconfigure + build (use after changing compiler/flags)
make debug      # configure + build a Debug build
make clean      # remove the out/ build directory
```

Or drive CMake directly:

```bash
cmake -B out
cmake --build out
# binary: out/executableBinary
```

See [docs/BUILD_INSTRUCTIONS.md](docs/BUILD_INSTRUCTIONS.md) and
[docs/README_CMAKE.md](docs/README_CMAKE.md) for details and alternatives.

---

## Usage

The VM takes one or more image names on the command line. Each is resolved under
`images/` with a `.obj` extension (see the note in
[Project Structure](#project-structure)).

```bash
# Convenience targets (run the already-built binary; they do NOT rebuild)
make run-2048       # → out/executableBinary 2048   (loads images/2048.obj)
make run-rogue      # → out/executableBinary rogue  (loads images/rogue.obj)

# Or run the binary directly
./out/executableBinary 2048
./out/executableBinary rogue
```

> `make run-2048` / `make run-rogue` only launch the binary. Build first with
> `make` (or `make rebuild`).

### Game Controls

Both games use **WASD** for movement:

| Key | Action |
|-----|--------|
| `W` | Up |
| `A` | Left |
| `S` | Down |
| `D` | Right |

- **2048** — slide tiles; equal tiles merge; reach the 2048 tile to win.
- **Rogue** — explore the dungeon, avoid enemies, reach the exit.

Press **Ctrl+C** at any time to interrupt; the console input mode is always
restored on exit.

---

## Technical Details

### Architecture Specifications
- **Word size**: 16 bits
- **Address space**: 16-bit (65,536 locations)
- **Registers**: 10 (8 general-purpose + PC + COND)
- **Opcode size**: 4 bits (bits `[15:12]`), 16 encodings, 15 used
- **Program start**: `0x3000`

### Opcode Table

| Opcode | Mnemonic | Description |
|--------|----------|-------------|
| 0 | `BR` | Conditional branch |
| 1 | `ADD` | Addition |
| 2 | `LD` | Load (PC-relative) |
| 3 | `ST` | Store (PC-relative) |
| 4 | `JSR`/`JSRR` | Jump to subroutine |
| 5 | `AND` | Bitwise AND |
| 6 | `LDR` | Load register (base + offset) |
| 7 | `STR` | Store register (base + offset) |
| 8 | `RTI` | Return from interrupt (unused — throws) |
| 9 | `NOT` | Bitwise NOT |
| 10 | `LDI` | Load indirect |
| 11 | `STI` | Store indirect |
| 12 | `JMP`/`RET` | Jump |
| 13 | `RES` | Reserved (unused — throws) |
| 14 | `LEA` | Load effective address |
| 15 | `TRAP` | System call |

### Memory Map

```
0x0000 ┌────────────────────┐
       │ Trap Vector Table  │
0x0200 ├────────────────────┤
       │ Interrupt Vectors  │
0x0300 ├────────────────────┤
       │ OS / Supervisor    │
0x3000 ├────────────────────┤ ← Program start (PC)
       │                    │
       │  User Program      │
       │                    │
0xFE00 ├────────────────────┤
       │ Device Registers   │
       │ KBSR (0xFE00)      │
       │ KBDR (0xFE02)      │
0xFFFF └────────────────────┘
```

### Condition Flags (`COND`)
Exactly one is set after each arithmetic/load result:

- `Flag::Positive` (`0x0001`)
- `Flag::Zero` (`0x0002`)
- `Flag::Negative` (`0x0004`)

### Addressing Modes
Immediate, register, PC-relative (`PC + sext(offset9)`), indirect
(`mem[mem[PC + offset]]`), and base+offset (`mem[base + sext(offset6)]`).

---

## Included Programs

### 2048 — `images/2048.obj` (2,276 bytes)
Classic sliding-tile puzzle. Slide with WASD; equal tiles merge on contact;
reach **2048** to win; game over when no move is possible.

### Rogue — `images/rogue.obj` (762 bytes)
Dungeon-crawl inspired by the classic *Rogue*. Move with WASD, avoid enemies,
reach the exit.

---

## Modern C++ Features

Standard-library and language features the codebase relies on, newest first.
The project targets **C++23** (`CMAKE_CXX_STANDARD 23`, built with **GCC 15**).

| Standard | Feature | Header | Where / Why |
|----------|---------|--------|-------------|
| **C++23** | `std::expected` / `std::unexpected` | `<expected>` | `CPU::LoadImage` returns `std::expected<bool, ImageError>` — typed image-load errors without exceptions or out-params |
| **C++23** | `std::byteswap` | `<bit>` | `FromBigEndianToHostEndian` — swaps image words to host byte order |
| **C++20** | `std::span` | `<span>` | Non-owning views: `argv` in `RunVirtualMachine`, memory regions (`ViewOfMemoryFrom`), and strings in `Trap` |
| **C++20** | `std::endian` (`std::endian::native`) | `<bit>` | Compile-time host-endianness detection in the byte-swap path |
| **C++17** | `inline constexpr` variables | — | Header-only architectural constants in `Lc3Types.h` with no ODR violations |
| **C++17** | `if constexpr` | — | Selects the byte-swap branch only on little-endian hosts, at compile time |
| **C++17** | `[[nodiscard]]`, `[[maybe_unused]]` | — | Attribute hygiene on accessors and helpers |
| **C++17** | `std::string_view` | `<string_view>` | `kImageDir` / `kImageExt` constants without allocation |
| **C++11** | `enum class` with fixed underlying type | — | `Opcode`, `Register`, `TrapVector`, `ImageError` — strongly-typed, sized enums |
| **C++11** | `std::array` | `<array>` | Register file and 64 K memory as fixed-size, bounds-aware arrays |
| **C++11** | `= default` / `= delete` | — | Defaulted destructors; deleted copy/assign on the RAII input guard |
| **C++11** | `noexcept` | — | Marks non-throwing accessors and the platform I/O routines |
| **C++11** | Type aliases (`using`) | — | `Word16` / `Address` for `std::uint16_t` |
| **C++11** | Fixed-width integers | `<cstdint>` | Exact 16-bit word and address modeling |
| **C++11** | `volatile std::sig_atomic_t` | `<csignal>` | Async-signal-safe Ctrl+C interrupt flag |

> C++14 introduced no feature used exclusively here; the jump is effectively
> C++11 → C++17 → C++20 → C++23.

---

## Tooling

- **CMake** emits `out/compile_commands.json` (`CMAKE_EXPORT_COMPILE_COMMANDS`)
  so analyzers see the exact build flags.
- **clang-tidy** — configured in [.clang-tidy](.clang-tidy) (bugprone, cppcoreguidelines,
  modernize, performance, portability, readability families).
- **cpplint** — Google C++ style checks; see
  [docs/README_STYLE_GUIDE.md](docs/README_STYLE_GUIDE.md) and
  [docs/cpplint.py](docs/cpplint.py).
- **Warnings**: the target is built with `-Wall -Wextra`.

---

## Documentation

| Document | Purpose |
|----------|---------|
| [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) | In-depth system design, data flow, and instruction-execution pipeline |
| [docs/BUILD_INSTRUCTIONS.md](docs/BUILD_INSTRUCTIONS.md) | Detailed build and run instructions across environments |
| [docs/README_CMAKE.md](docs/README_CMAKE.md) | The CMake project template (C++23 / GCC 15) this build is based on |
| [docs/CODING_STYLE.md](docs/CODING_STYLE.md) | House style: file layout, member ordering, comment conventions |
| [docs/README_STYLE_GUIDE.md](docs/README_STYLE_GUIDE.md) | Linting with cpplint against the Google C++ style guide |

---

## Future Enhancements

- A debugger with breakpoints, stepping, and register/memory inspection
- A disassembler and an integrated LC-3 assembler
- Instruction tracing and performance profiling
- A test suite and benchmarks under `tests/` and `bench/`

---

## Author

**Mehmet Arslan** — systems programmer focused on low-level software, computer
architecture, and virtual machine implementation.

**Demonstrated here:** computer-architecture simulation, ISA implementation,
memory management and addressing, binary image parsing, real-time and
cross-platform I/O, modern C++ (C++23), and clean component design (dependency
injection + RAII).

**GitHub**: [@htmos6](https://github.com/htmos6) ·
**Repository**: [Virtual-Machine](https://github.com/htmos6/Virtual-Machine)

---

## License

Licensed under the **MIT License** — see [LICENSE](LICENSE).

---

## Acknowledgments

- **LC-3 architecture** by Yale Patt and Sanjay Patel
- *Introduction to Computing Systems: From Bits and Gates to C and Beyond*
- Community-contributed LC-3 game images (2048, Rogue)
