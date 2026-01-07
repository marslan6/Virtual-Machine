# LC-3 Virtual Machine Architecture

This document provides an in-depth technical overview of the LC-3 Virtual Machine implementation, covering system design, data flow, instruction execution, and implementation details.

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

The LC-3 VM follows a **modular architecture** with clear separation of concerns:

```
┌──────────────────────────────────────────────────────┐
│                  VirtualMachine                      │
│         (Orchestration & Control Flow)               │
└───────┬──────────────────────────────────────────────┘
        │ Coordinates execution between components
        │
┌───────▼─────┬─────────┬──────────┬─────────┬────────┐
│             │         │          │         │        │
▼             ▼         ▼          ▼         ▼        ▼
CPU          ALU    MemoryIO     Trap      OS
(State)   (Execute) (Access)  (Services) (Platform)
```

### Key Design Principles

1. **Single Responsibility**: Each class handles one aspect (CPU state, instruction execution, I/O, etc.)
2. **Dependency Injection**: Components are passed references to shared resources
3. **Encapsulation**: Internal state is protected, accessed through methods
4. **Modularity**: Components can be tested independently

---

## Component Architecture

### 1. CPU (Central Processing Unit)

**File**: `src/CPU.h`, `src/CPU.cpp`

**Responsibilities:**
- Register file management
- Memory storage
- Condition flag updates
- Binary image loading

#### Data Structures

```cpp
class CPU {
public:
    // Register file
    uint16_t registers[REGISTER_COUNT];  // 10 registers

    // Memory (128 KB = 65,536 words)
    uint16_t memory[UINT16_MAX];

    // Execution control
    int running;
};
```

**Register Enumeration:**
```cpp
enum Register {
    R0 = 0,      // General purpose
    R1,          // General purpose
    R2,          // General purpose
    R3,          // General purpose
    R4,          // General purpose
    R5,          // General purpose
    R6,          // General purpose
    R7,          // General purpose (also used for JSR return address)
    R_PC,        // Program Counter (register 8)
    R_COND,      // Condition flags (register 9)
    REGISTER_COUNT
};
```

**Condition Flags:**
```cpp
enum Flag {
    FL_POSITIVE = 1 << 0,  // 0b001 (1)
    FL_ZERO     = 1 << 1,  // 0b010 (2)
    FL_NEGATIVE = 1 << 2   // 0b100 (4)
};
```

#### Key Methods

**UpdateFlags(uint16_t reg)**
- Called after every instruction that modifies a register
- Checks the value in `registers[reg]`
- Sets R_COND to FL_POSITIVE, FL_ZERO, or FL_NEGATIVE
- Uses 2's complement representation (bit 15 = sign bit)

```cpp
void UpdateFlags(uint16_t reg) {
    if (registers[reg] == 0) {
        registers[R_COND] = FL_ZERO;
    } else if (registers[reg] >> 15) {  // Check sign bit
        registers[R_COND] = FL_NEGATIVE;
    } else {
        registers[R_COND] = FL_POSITIVE;
    }
}
```

**ReadImage(const char* path)**
- Opens binary file
- Reads origin address (first 16-bit word)
- Loads subsequent words into memory starting at origin
- Handles byte-order conversion (big-endian → little-endian)

---

### 2. ALU (Arithmetic Logic Unit)

**File**: `src/ArithmeticLogicUnit.h`, `src/ArithmeticLogicUnit.cpp`

**Responsibilities:**
- Decode and execute all 15 LC-3 opcodes
- Sign extension for immediate values
- Byte-order conversion utilities

#### Opcode Enumeration

```cpp
enum Opcode {
    OP_BR = 0,   // Branch
    OP_ADD,      // Add
    OP_LD,       // Load
    OP_ST,       // Store
    OP_JSR,      // Jump register
    OP_AND,      // Bitwise AND
    OP_LDR,      // Load register
    OP_STR,      // Store register
    OP_RTI,      // Return from interrupt (unused)
    OP_NOT,      // Bitwise complement
    OP_LDI,      // Load indirect
    OP_STI,      // Store indirect
    OP_JMP,      // Jump
    OP_RES,      // Reserved (unused)
    OP_LEA,      // Load effective address
    OP_TRAP      // Execute trap
};
```

#### Instruction Implementation Examples

**ADD (OP_ADD = 0b0001)**

Instruction format:
```
┌────┬────┬────┬─┬──────┬───┐  Register mode
│0001│ DR │SR1 │0│  00  │SR2│
└────┴────┴────┴─┴──────┴───┘

┌────┬────┬────┬─┬─────────┐  Immediate mode
│0001│ DR │SR1 │1│  imm5   │
└────┴────┴────┴─┴─────────┘
```

Implementation:
```cpp
case OP_ADD: {
    uint16_t dest_reg = (instruction >> 9) & 0x7;     // Bits [11:9]
    uint16_t source_reg = (instruction >> 6) & 0x7;   // Bits [8:6]
    uint16_t imm_flag = (instruction >> 5) & 0x1;     // Bit [5]

    if (imm_flag) {
        // Immediate mode
        uint16_t imm5 = SignExtend(instruction & 0x1F, 5);
        registers[dest_reg] = registers[source_reg] + imm5;
    } else {
        // Register mode
        uint16_t second_reg = instruction & 0x7;      // Bits [2:0]
        registers[dest_reg] = registers[source_reg] + registers[second_reg];
    }

    UpdateFlags(dest_reg);
    break;
}
```

**BR (Conditional Branch - OP_BR = 0b0000)**

Instruction format:
```
┌────┬─┬─┬─┬────────────┐
│0000│n│z│p│  PCoffset9 │
└────┴─┴─┴─┴────────────┘
```

Implementation:
```cpp
case OP_BR: {
    uint16_t condition = (instruction >> 9) & 0x7;  // nzp flags
    uint16_t pc_offset = SignExtend(instruction & 0x1FF, 9);

    // Branch if ANY of the specified condition flags match
    if (condition & registers[R_COND]) {
        registers[R_PC] += pc_offset;
    }
    break;
}
```

**LDI (Load Indirect - OP_LDI = 0b1010)**

Double indirection:
```
R0 = memory[memory[PC + offset]]
```

Instruction format:
```
┌────┬────┬────────────┐
│1010│ DR │  PCoffset9 │
└────┴────┴────────────┘
```

Implementation:
```cpp
case OP_LDI: {
    uint16_t dest_reg = (instruction >> 9) & 0x7;
    uint16_t pc_offset = SignExtend(instruction & 0x1FF, 9);

    // First indirection: get address
    uint16_t address = memory[registers[R_PC] + pc_offset];

    // Second indirection: get value at that address
    registers[dest_reg] = memoryIO->Read(address);

    UpdateFlags(dest_reg);
    break;
}
```

#### Utility Functions

**SignExtend(uint16_t value, int bit_count)**
- Extends a smaller value to 16 bits preserving sign
- Used for immediate values (5-bit, 6-bit, 9-bit, 11-bit)
- Critical for proper 2's complement arithmetic

```cpp
uint16_t SignExtend(uint16_t value, int bit_count) {
    // Check if sign bit is set
    if ((value >> (bit_count - 1)) & 1) {
        // Extend with 1s
        value |= (0xFFFF << bit_count);
    }
    return value;
}
```

Example:
```
5-bit value: 11011 (-5 in 2's complement)
SignExtend(0b11011, 5) = 0b1111111111111011 = 0xFFFB = -5
```

**Swap16(uint16_t value)**
- Converts between big-endian and little-endian
- LC-3 files are big-endian, x86 is little-endian

```cpp
uint16_t Swap16(uint16_t value) {
    return (value << 8) | (value >> 8);
}
```

---

### 3. MemoryIO (Memory Access Layer)

**File**: `src/MemoryIO.h`, `src/MemoryIO.cpp`

**Responsibilities:**
- Intercept memory reads for device registers
- Handle keyboard input via memory-mapped I/O
- Provide transparent access to regular memory

#### Memory-Mapped Registers

```cpp
enum MemoryMappedRegister {
    MR_KBSR = 0xFE00,  // Keyboard Status Register
    MR_KBDR = 0xFE02   // Keyboard Data Register
};
```

**KBSR (Keyboard Status Register - 0xFE00):**
- Bit 15: Ready bit (1 = key available, 0 = no key)
- Bits 14-0: Unused

**KBDR (Keyboard Data Register - 0xFE02):**
- Bits 7-0: ASCII character code
- Bits 15-8: Unused

#### Read Implementation

```cpp
uint16_t Read(uint16_t address) {
    if (address == MR_KBSR) {
        // Check if keyboard has input
        if (os->CheckKey()) {
            memory[MR_KBSR] = (1 << 15);    // Set ready bit
            memory[MR_KBDR] = getchar();     // Read character
        } else {
            memory[MR_KBSR] = 0;             // Clear ready bit
        }
    }
    return memory[address];
}
```

**Write Implementation:**
```cpp
void Write(uint16_t address, uint16_t value) {
    memory[address] = value;
}
```

---

### 4. Trap Handler

**File**: `src/Trap.h`, `src/Trap.cpp`

**Responsibilities:**
- Provide system services via TRAP instruction
- Handle I/O operations (character, string)
- Manage program termination

#### Trap Codes

```cpp
enum TrapCode {
    TRAP_GETC  = 0x20,  // Get character (no echo)
    TRAP_OUT   = 0x21,  // Output character
    TRAP_PUTS  = 0x22,  // Output string
    TRAP_IN    = 0x23,  // Get character (with echo)
    TRAP_PUTSP = 0x24,  // Output packed byte string
    TRAP_HALT  = 0x25   // Halt execution
};
```

#### Trap Implementations

**TRAP_GETC (0x20)**
```cpp
// Read single character, no echo
// Store in R0
int c = getchar();
registers[R0] = (uint16_t)c;
```

**TRAP_PUTS (0x22)**
```cpp
// Output null-terminated string
// R0 contains starting address
uint16_t* ptr = memory + registers[R0];
while (*ptr) {
    putc((char)*ptr, stdout);
    ptr++;
}
fflush(stdout);
```

**TRAP_PUTSP (0x24)**
```cpp
// Output packed byte string (2 chars per word)
// R0 contains starting address
uint16_t* ptr = memory + registers[R0];
while (*ptr) {
    char char1 = (*ptr) & 0xFF;         // Low byte
    putc(char1, stdout);

    char char2 = (*ptr) >> 8;           // High byte
    if (char2) putc(char2, stdout);

    ptr++;
}
fflush(stdout);
```

**TRAP_HALT (0x25)**
```cpp
// Stop execution
cpu->running = 0;
puts("HALT");
fflush(stdout);
```

---

### 5. OS (Operating System Interface)

**File**: `src/OS.h`, `src/OS.cpp`

**Responsibilities:**
- Platform-specific console I/O
- Input buffering control
- Keyboard polling
- Signal handling

#### Windows Console Mode Management

**DisableInputBuffering()**
```cpp
void DisableInputBuffering() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    // Disable echo and line input
    mode &= ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, mode);

    FlushConsoleInputBuffer(hStdin);
}
```

**RestoreInputBuffering()**
```cpp
void RestoreInputBuffering() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    // Re-enable echo and line input
    mode |= (ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);
    SetConsoleMode(hStdin, mode);
}
```

#### Keyboard Polling

**CheckKey()**
```cpp
uint16_t CheckKey() {
    // Wait for input with 1 second timeout
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
}
```

---

## Instruction Execution Pipeline

### Fetch-Decode-Execute Cycle

```
┌─────────────┐
│   FETCH     │  1. Read instruction from memory[PC]
│ instruction │  2. Increment PC
│   @ PC      │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   DECODE    │  1. Extract opcode (bits 15-12)
│   opcode    │  2. Extract operands (bits 11-0)
│             │
└──────┬──────┘
       │
       ▼
┌─────────────┐
│  EXECUTE    │  1. Dispatch to ALU or Trap
│ instruction │  2. Perform operation
│             │  3. Update registers/memory
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   UPDATE    │  1. Update condition flags (if needed)
│    FLAGS    │  2. Check for interrupts (future)
│             │
└──────┬──────┘
       │
       └──────→ Repeat until HALT
```

### Main Execution Loop

```cpp
void RunVirtualMachine(int argc, const char* argv[]) {
    // 1. Load program
    cpu->ReadImage(argv[1]);

    // 2. Set PC to start address
    cpu->registers[R_PC] = PC_START;  // 0x3000

    // 3. Main loop
    while (cpu->running) {
        // FETCH
        uint16_t instruction = cpu->memory[cpu->registers[R_PC]++];

        // DECODE
        uint16_t opcode = instruction >> 12;  // Top 4 bits

        // EXECUTE
        switch (opcode) {
            case OP_ADD:  alu->ADD(instruction); break;
            case OP_AND:  alu->AND(instruction); break;
            case OP_BR:   alu->BR(instruction); break;
            // ... other opcodes ...
            case OP_TRAP: trap->Proxy(instruction); break;
        }
    }
}
```

---

## Memory Organization

### Memory Layout

```
Address Range    │ Size   │ Contents
─────────────────┼────────┼──────────────────────────
0x0000 - 0x00FF  │ 256 B  │ Trap Vector Table
0x0100 - 0x01FF  │ 256 B  │ Interrupt Vector Table
0x0200 - 0x2FFF  │ 11.5KB │ OS / Supervisor Space
0x3000 - 0xFDFF  │ 51.5KB │ User Program Space
0xFE00 - 0xFE01  │ 2 B    │ Keyboard Status (KBSR)
0xFE02 - 0xFE03  │ 2 B    │ Keyboard Data (KBDR)
0xFE04 - 0xFFFF  │ 506 B  │ Device Registers (reserved)
```

### Memory Access Patterns

**Direct Addressing:**
```
LD R0, LABEL    ; R0 = memory[PC + offset]
```

**Indirect Addressing:**
```
LDI R0, LABEL   ; R0 = memory[memory[PC + offset]]
```

**Register Offset:**
```
LDR R0, R1, #6  ; R0 = memory[R1 + 6]
```

---

## I/O System

### Input Flow

```
┌──────────────┐
│  Keyboard    │
└──────┬───────┘
       │ User presses key
       ▼
┌──────────────┐
│   OS.cpp     │
│  CheckKey()  │  Polls with timeout
└──────┬───────┘
       │ Key detected
       ▼
┌──────────────┐
│ MemoryIO.cpp │
│   Read()     │  Sets KBSR bit 15
└──────┬───────┘  Stores char in KBDR
       │
       ▼
┌──────────────┐
│   Program    │
│  reads KBSR  │  Checks bit 15
│  reads KBDR  │  Gets character
└──────────────┘
```

### Output Flow

```
┌──────────────┐
│   Program    │
│ TRAP_OUT or  │
│  TRAP_PUTS   │
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Trap.cpp   │
│   Proxy()    │  Dispatches to handler
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   putc() or  │
│   puts()     │  C standard library
└──────┬───────┘
       │
       ▼
┌──────────────┐
│   Console    │  Character displayed
└──────────────┘
```

---

## Implementation Details

### Performance Optimizations

1. **Direct array access**: No pointer indirection for memory/registers
2. **Switch-based dispatch**: Faster than function pointer table
3. **Inline flag checks**: Minimal overhead for condition flags
4. **Minimal abstraction**: Direct hardware simulation, no virtualization layers

### Limitations

1. **No interrupt system**: RTI instruction is reserved but not implemented
2. **Windows-only**: Platform-specific console APIs
3. **Single-threaded**: No concurrent execution
4. **No privilege modes**: All code runs at same level

### Extension Points

**Adding New Devices:**
1. Define new memory-mapped register addresses
2. Extend `MemoryIO::Read()` with device-specific logic
3. Implement device-specific trap codes

**Adding Debugging:**
1. Add breakpoint support in main loop
2. Implement register/memory inspection
3. Add step execution mode

---

**Document Version**: 1.0
**Last Updated**: January 2026
