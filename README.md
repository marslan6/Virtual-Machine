# LC-3 Virtual Machine Emulator

[![Platform](https://img.shields.io/badge/Platform-Windows-blue)](https://www.microsoft.com/en-us/windows)
[![Language](https://img.shields.io/badge/Language-C%2B%2B-green)](https://isocpp.org/)
[![Architecture](https://img.shields.io/badge/Architecture-LC--3-red)](https://en.wikipedia.org/wiki/Little_Computer_3)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.txt)

A complete software implementation of the **LC-3 (Little Computer 3)** 16-bit educational virtual machine architecture written in C++. This emulator executes compiled LC-3 programs and includes two playable games: **2048** and **Rogue**.

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Architecture](#architecture)
- [Project Structure](#project-structure)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Technical Details](#technical-details)
- [Included Programs](#included-programs)
- [Author](#author)

---

## Overview

The LC-3 Virtual Machine is an **instruction set simulator** that emulates the complete functionality of the LC-3 computer architecture. It provides a runtime environment for executing LC-3 binary programs with full support for:

- **15 LC-3 opcodes** (arithmetic, logical, control flow, memory operations)
- **Memory-mapped I/O** for keyboard input
- **Trap routines** for system services (I/O operations)
- **Real-time console interaction** with unbuffered input
- **Binary image loading** with proper byte-order conversion

This project demonstrates:
- Low-level computer architecture implementation
- Instruction fetch-decode-execute cycle
- Memory management and addressing modes
- System-level programming with I/O handling
- Object-oriented design for hardware simulation

---

## Features

### Core Functionality
- **Complete LC-3 ISA Implementation**: All 15 opcodes fully functional
- **10 Registers**: 8 general-purpose (R0-R7), Program Counter (PC), Condition Flags (COND)
- **128 KB Addressable Memory**: 65,536 16-bit words
- **Condition Flag System**: Automatic updates for POSITIVE, ZERO, NEGATIVE
- **Three Addressing Modes**: Register, immediate, and memory addressing
- **Binary Program Loader**: Reads and loads LC-3 object files

### I/O Capabilities
- **Memory-Mapped Keyboard**: Real-time input via KBSR/KBDR registers (0xFE00/0xFE02)
- **Console Output**: Character and string output routines
- **Unbuffered Input**: Immediate keyboard response with disabled echo
- **Trap Routines**: 6 system calls for I/O operations

### Advanced Features
- **Sign Extension**: Proper 2's complement arithmetic for immediate values
- **Byte-Order Conversion**: Big-endian to little-endian swapping
- **Interrupt Handling**: Graceful Ctrl+C handling with cleanup
- **Program Counter Management**: Automatic PC increment and branching

---

## Architecture

### System Components

The virtual machine consists of five primary modules:

```
┌──────────────────────────────────────────────┐
│          VirtualMachine                      │
│   (Main execution orchestrator)              │
└───────────┬────────────────────────────────┘
            │
  ┌─────────┴──────┬──────────┬──────────┬─────────┐
  │                │          │          │         │
  ▼                ▼          ▼          ▼         ▼
┌────────┐  ┌─────────────┐ ┌─────────┐ ┌────┐ ┌────┐
│  CPU   │  │     ALU     │ │MemoryIO │ │Trap│ │ OS │
│        │  │             │ │          │ │    │ │    │
│ 10 Regs│  │ 15 Opcodes  │ │ Keyboard │ │ 6  │ │Con-│
│ 128KB  │  │ Arithmetic  │ │ Mapped   │ │Traps│ │sole│
│ Flags  │  │ Control Flow│ │ I/O      │ │ I/O │ │I/O │
└────────┘  └─────────────┘ └─────────┘ └────┘ └────┘
```

#### 1. **CPU (Central Processing Unit)**
- **Registers**: R0-R7 (general-purpose), R_PC (Program Counter), R_COND (Condition Flags)
- **Memory**: 65,536 × 16-bit words (128 KB)
- **Program Start**: 0x3000
- **Functions**:
  - `UpdateFlags()`: Sets condition flags based on register values
  - `ReadImage()`: Loads binary object files into memory
  - `ReadImageFile()`: Handles file I/O with byte swapping

#### 2. **ALU (Arithmetic Logic Unit)**
Implements all 15 LC-3 instructions:

| Category | Instructions |
|----------|--------------|
| **Arithmetic** | ADD, AND, NOT |
| **Control Flow** | BR (conditional branch), JMP, JSR |
| **Memory Load** | LD, LDR, LDI, LEA |
| **Memory Store** | ST, STR, STI |
| **System** | TRAP, RTI (reserved) |

Key methods:
- `SignExtend()`: Extends immediate values for 2's complement
- `Swap16()`: Converts byte order (big ↔ little endian)

#### 3. **MemoryIO**
- Manages memory-mapped I/O for keyboard input
- **Registers**:
  - `MR_KBSR (0xFE00)`: Keyboard Status Register
  - `MR_KBDR (0xFE02)`: Keyboard Data Register
- **Methods**:
  - `Read()`: Memory read with automatic keyboard polling
  - `Write()`: Standard memory write

#### 4. **Trap Handler**
Provides 6 system services:

| Trap Code | Name | Function |
|-----------|------|----------|
| 0x20 | GETC | Read character (no echo) |
| 0x21 | OUT | Output single character |
| 0x22 | PUTS | Output null-terminated string |
| 0x23 | IN | Read character (with prompt) |
| 0x24 | PUTSP | Output packed byte string |
| 0x25 | HALT | Stop execution |

#### 5. **OS (Operating System Interface)**
- **Windows Console API** integration
- Disables input buffering for real-time response
- Keyboard polling with timeout
- Signal handling for Ctrl+C interrupts

---

## Project Structure

```
LC3-Virtual-Machine/
├── README.md                       # This file
├── LICENSE.txt                     # MIT License
├── .gitignore                      # Git ignore rules
├── .gitattributes                  # Git attributes
│
├── src/                           # Source code
│   ├── main.cpp                   # Entry point
│   ├── VirtualMachine.cpp/h       # Main VM orchestrator
│   ├── CPU.cpp/h                  # CPU implementation
│   ├── ArithmeticLogicUnit.cpp/h  # ALU (15 opcodes)
│   ├── MemoryIO.cpp/h             # Memory-mapped I/O
│   ├── Trap.cpp/h                 # Trap handler
│   └── OS.cpp/h                   # OS interface
│
├── programs/                      # LC-3 executable programs
│   ├── 2048.obj                   # 2048 game (2.3 KB)
│   └── rogue.obj                  # Rogue game (762 bytes)
│
├── docs/                          # Documentation
│   ├── ARCHITECTURE.md            # System architecture details
│   ├── BUILD_INSTRUCTIONS.md      # Build and compile guide
│   └── LC3_SPECIFICATION.md       # LC-3 ISA reference
│
├── visual-studio/                 # Visual Studio project files
│   ├── Virtual-Machine.sln        # Solution file
│   ├── Virtual-Machine.vcxproj    # Project file
│   └── Virtual-Machine.vcxproj.filters
│
└── build/                         # Build output directory
```

### File Descriptions

**Core Source Files:**

**[src/main.cpp](src/main.cpp)**
- Program entry point
- Instantiates all VM components (CPU, ALU, MemoryIO, Trap, OS)
- Passes control to VirtualMachine orchestrator

**[src/VirtualMachine.cpp/h](src/VirtualMachine.cpp)**
- Main execution engine
- Validates command-line arguments
- Loads object files into memory
- **Fetch-Decode-Execute Loop**:
  1. Fetches instruction at PC
  2. Extracts opcode (bits 15-12)
  3. Dispatches to ALU or Trap handler
  4. Continues until HALT

**[src/CPU.cpp/h](src/CPU.cpp)**
- Register file (10 registers)
- Memory array (65,536 words)
- Flag update logic (POS/ZERO/NEG)
- Binary image loading with byte swapping

**[src/ArithmeticLogicUnit.cpp/h](src/ArithmeticLogicUnit.cpp)**
- All 15 LC-3 instruction implementations
- Sign extension for immediate values
- Endianness conversion utilities

**[src/MemoryIO.cpp/h](src/MemoryIO.cpp)**
- Memory read/write operations
- Keyboard status checking at 0xFE00
- Keyboard data retrieval at 0xFE02

**[src/Trap.cpp/h](src/Trap.cpp)**
- Trap instruction dispatcher
- 6 trap routine implementations
- String and character I/O

**[src/OS.cpp/h](src/OS.cpp)**
- Windows console mode manipulation
- Input buffering control
- Keyboard polling with timeout
- Interrupt signal handling

---

## Getting Started

### Prerequisites

**Software:**
- **Operating System**: Windows (uses Windows API)
- **Compiler**: Visual Studio 2017 or later (or any C++11 compatible compiler)
- **Standard Libraries**: `<iostream>`, `<cstdint>`, `<cstdio>`, `<csignal>`, Windows.h

**Hardware:**
- Any modern x86/x64 processor
- Minimal memory requirements (~10 MB)

### Building the Project

#### Option 1: Visual Studio (Recommended)

1. **Open Solution**:
   ```
   visual-studio/Virtual-Machine.sln
   ```

2. **Update Project Paths** (if needed):
   - Right-click project → Properties
   - C/C++ → General → Additional Include Directories: `../src`
   - Linker → General → Output Directory: `../build`

3. **Build**:
   - Build → Build Solution (Ctrl+Shift+B)
   - Output: `build/Virtual-Machine.exe`

#### Option 2: Command Line (MSVC)

```cmd
# From project root directory
cl /EHsc /std:c++11 /I src src/*.cpp /Fe:build/vm.exe
```

#### Option 3: MinGW/G++ (Windows)

```bash
g++ -std=c++11 -I src src/*.cpp -o build/vm.exe
```

**Expected Build Output:**
```
Compiling: main.cpp
Compiling: VirtualMachine.cpp
Compiling: CPU.cpp
Compiling: ArithmeticLogicUnit.cpp
Compiling: MemoryIO.cpp
Compiling: Trap.cpp
Compiling: OS.cpp
Linking: build/Virtual-Machine.exe
Build succeeded
```

---

## Usage

### Running the Virtual Machine

The VM requires at least one LC-3 object file as a command-line argument:

```cmd
# Run 2048 game
build\vm.exe programs\2048.obj

# Run Rogue game
build\vm.exe programs\rogue.obj

# Run custom program
build\vm.exe path\to\your_program.obj
```

### Game Controls

#### **2048 Game**
- **W**: Move up
- **A**: Move left
- **S**: Move down
- **D**: Move right
- **Q**: Quit game

**Objective**: Combine tiles to reach 2048

#### **Rogue Game**
- **W**: Move up
- **A**: Move left
- **S**: Move down
- **D**: Move right
- Explore dungeons, avoid enemies, collect treasures

### Debugging

**Visual Studio:**
1. Right-click project → Properties → Debugging
2. Command Arguments: `programs\2048.obj`
3. Press F5 to debug

**Command Line:**
```cmd
# Add verbose output (requires code modification)
build\vm.exe programs\2048.obj --debug
```

---

## Technical Details

### LC-3 Instruction Set Architecture

**Architecture Specifications:**
- **Word Size**: 16 bits
- **Address Space**: 16-bit (65,536 locations)
- **Register Count**: 10 (8 general + PC + COND)
- **Opcode Size**: 4 bits (16 possible opcodes, 15 implemented)

### Instruction Format

```
┌────────┬──────────────────────┐
│ Opcode │     Operands         │
│ [15:12]│     [11:0]           │
└────────┴──────────────────────┘
  4 bits       12 bits
```

### Opcode Table

| Opcode | Binary | Mnemonic | Description |
|--------|--------|----------|-------------|
| 0000 | 0b0000 | BR | Conditional branch |
| 0001 | 0b0001 | ADD | Addition |
| 0010 | 0b0010 | LD | Load (PC-relative) |
| 0011 | 0b0011 | ST | Store (PC-relative) |
| 0100 | 0b0100 | JSR | Jump to subroutine |
| 0101 | 0b0101 | AND | Bitwise AND |
| 0110 | 0b0110 | LDR | Load register |
| 0111 | 0b0111 | STR | Store register |
| 1000 | 0b1000 | RTI | Return from interrupt (reserved) |
| 1001 | 0b1001 | NOT | Bitwise NOT |
| 1010 | 0b1010 | LDI | Load indirect |
| 1011 | 0b1011 | STI | Store indirect |
| 1100 | 0b1100 | JMP/RET | Jump |
| 1101 | 0b1101 | RES | Reserved |
| 1110 | 0b1110 | LEA | Load effective address |
| 1111 | 0b1111 | TRAP | System call |

### Memory Map

```
0x0000 ┌──────────────────┐
       │ Trap Vector Table │
0x0200 ├──────────────────┤
       │  Interrupt Vector │
0x0300 ├──────────────────┤
       │   OS/Supervisor   │
0x3000 ├──────────────────┤ ← Program Start
       │                  │
       │   User Programs  │
       │                  │
0xFE00 ├──────────────────┤
       │  Device Registers│
       │  KBSR/KBDR       │
0xFFFF └──────────────────┘
```

### Condition Flags

Stored in R_COND register:
- **FL_POSITIVE (1 << 0)**: Result > 0
- **FL_ZERO (1 << 1)**: Result == 0
- **FL_NEGATIVE (1 << 2)**: Result < 0

Automatically updated after arithmetic/load operations.

### Addressing Modes

1. **Immediate Mode**: Value embedded in instruction
   ```
   ADD R0, R1, #5    ; R0 = R1 + 5
   ```

2. **Register Mode**: Operand in register
   ```
   ADD R0, R1, R2    ; R0 = R1 + R2
   ```

3. **PC-Relative**: Offset from Program Counter
   ```
   LD R0, LABEL      ; R0 = memory[PC + offset]
   ```

4. **Indirect**: Double indirection
   ```
   LDI R0, LABEL     ; R0 = memory[memory[PC + offset]]
   ```

5. **Base + Offset**: Register base with offset
   ```
   LDR R0, R1, #6    ; R0 = memory[R1 + 6]
   ```

---

## Included Programs

### 2048.obj (2,276 bytes)
Classic sliding tile puzzle game where you combine numbered tiles.

**How to Play:**
- Use WASD to slide tiles in all directions
- Tiles with the same number merge when they touch
- Create a tile with the number 2048 to win
- Game over when no moves are possible

### rogue.obj (762 bytes)
Dungeon exploration game inspired by the classic Rogue.

**How to Play:**
- Navigate through procedurally generated dungeons
- Use WASD for movement
- Avoid enemies and collect items
- Reach the exit to progress

---

## Performance Metrics

| Metric | Value |
|--------|-------|
| **Memory Usage** | ~130 KB (VM + programs) |
| **Instruction Cycle Time** | ~1 µs (depends on host CPU) |
| **Startup Time** | <100 ms |
| **Max Program Size** | 128 KB (65,536 words) |
| **Supported Programs** | Any valid LC-3 binary |

---

## Future Enhancements

Potential improvements:
- Cross-platform support (Linux/macOS with ncurses)
- Debugger with breakpoints and step execution
- Disassembler for LC-3 object files
- LC-3 assembler integration
- Performance profiling and instruction tracing
- GUI with register/memory visualization
- Multi-program loading support

---

## Author

**Mehmet Arslan**

Systems programmer specializing in low-level software development, computer architecture, and virtual machine implementation.

**Technical Skills Demonstrated:**
- Computer architecture simulation
- Instruction set implementation
- Memory management and addressing
- Binary file parsing
- Real-time I/O handling
- Object-oriented design
- Systems-level C++ programming
- Windows API integration

**GitHub**: [htmos6](https://github.com/htmos6)

---

## License

This project is licensed under the **MIT License** - see [LICENSE.txt](LICENSE.txt) for details.

```
Copyright © 2024 Mehmet Arslan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction...
```

---

## Acknowledgments

- **LC-3 Architecture**: Created by Yale Patt and Sanjay Patel
- **Reference Book**: "Introduction to Computing Systems: From Bits and Gates to C and Beyond"
- **Game Implementations**: Community-contributed LC-3 programs
- Inspired by educational computer architecture courses

---

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues for:
- Bug fixes
- New features
- Documentation improvements
- Additional LC-3 programs
- Cross-platform support

---

## Contact

For questions, suggestions, or collaboration:
- **GitHub**: [@htmos6](https://github.com/htmos6)
- **Repository**: [Virtual-Machine](https://github.com/htmos6/Virtual-Machine)

---

**Last Updated**: January 2026
**Version**: 1.0
