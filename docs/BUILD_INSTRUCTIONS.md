# Build Instructions - LC-3 Virtual Machine

This document provides detailed instructions for building and running the LC-3 Virtual Machine emulator across different development environments.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Building with Visual Studio](#building-with-visual-studio)
3. [Building with Command Line](#building-with-command-line)
4. [Project Configuration](#project-configuration)
5. [Running the VM](#running-the-vm)
6. [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Software Requirements

**Operating System:**
- Windows 7, 8, 10, or 11 (64-bit or 32-bit)
- Uses Windows-specific APIs (Windows.h, console manipulation)

**Compiler Options:**

#### Option 1: Microsoft Visual Studio (Recommended)
- **Version**: Visual Studio 2017 or later
- **Edition**: Community (free), Professional, or Enterprise
- **Workload**: Desktop development with C++
- **Download**: https://visualstudio.microsoft.com/downloads/

#### Option 2: Visual Studio Build Tools
- Standalone compiler without IDE
- Download: https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio

#### Option 3: MinGW-w64 (GCC for Windows)
- Free and open-source
- Download: https://www.mingw-w64.org/downloads/
- Or via MSYS2: https://www.msys2.org/

### C++ Standard Requirements
- **Minimum**: C++11
- **Recommended**: C++14 or later
- Features used:
  - `std::uint16_t`, `std::uint32_t` (from `<cstdint>`)
  - Lambda expressions (if extended)
  - Range-based for loops (potential)

---

## Building with Visual Studio

### Step 1: Open the Solution

1. **Launch Visual Studio**
2. **Open Solution File**:
   - File → Open → Project/Solution
   - Navigate to: `visual-studio/Virtual-Machine.sln`
   - Click Open

### Step 2: Update Project Paths

Since files have been reorganized, you'll need to update the project configuration:

#### Update Source Files

1. **Remove old file references**:
   - Right-click on each `.cpp`/`.h` file in Solution Explorer
   - Select Remove (DO NOT delete from disk)

2. **Add new file references**:
   - Right-click on project → Add → Existing Item
   - Navigate to `../src/` directory
   - Select all `.cpp` and `.h` files:
     - `main.cpp`
     - `VirtualMachine.cpp` / `VirtualMachine.h`
     - `CPU.cpp` / `CPU.h`
     - `ArithmeticLogicUnit.cpp` / `ArithmeticLogicUnit.h`
     - `MemoryIO.cpp` / `MemoryIO.h`
     - `Trap.cpp` / `Trap.h`
     - `OS.cpp` / `OS.h`
   - Click Add

#### Configure Include Directories

1. **Right-click project** → Properties
2. **Configuration**: All Configurations
3. **Platform**: All Platforms
4. Navigate to: **C/C++** → **General**
5. **Additional Include Directories**: Add `$(ProjectDir)\..\src`
6. Click OK

#### Configure Output Directory

1. **Right-click project** → Properties
2. Navigate to: **General**
3. **Output Directory**: `$(SolutionDir)\..\build\`
4. **Intermediate Directory**: `$(SolutionDir)\..\build\intermediate\$(Configuration)\`
5. Click Apply → OK

### Step 3: Build Configuration

#### Debug Configuration
- **Optimization**: Disabled (/Od)
- **Debug Information**: Full (/Zi)
- **Runtime Library**: Multi-threaded Debug DLL (/MDd)

#### Release Configuration
- **Optimization**: Maximize Speed (/O2)
- **Inline Function Expansion**: Any Suitable (/Ob2)
- **Runtime Library**: Multi-threaded DLL (/MD)
- **Whole Program Optimization**: Yes

### Step 4: Build the Project

1. **Select Configuration**:
   - Debug (for development)
   - Release (for distribution)

2. **Build**:
   - Build → Build Solution (Ctrl+Shift+B)
   - Or right-click project → Build

3. **Check Output**:
   ```
   1>------ Build started: Project: Virtual-Machine ------
   1>main.cpp
   1>VirtualMachine.cpp
   1>CPU.cpp
   1>ArithmeticLogicUnit.cpp
   1>MemoryIO.cpp
   1>Trap.cpp
   1>OS.cpp
   1>Generating Code...
   1>Virtual-Machine.vcxproj -> C:\...\build\Virtual-Machine.exe
   ========== Build: 1 succeeded, 0 failed ==========
   ```

4. **Verify Output**:
   - Executable location: `build/Virtual-Machine.exe`
   - Size: ~50-100 KB (depends on configuration)

---

## Building with Command Line

### Option 1: MSVC Command Line

#### Setup MSVC Environment

**Option A: Visual Studio Developer Command Prompt**
1. Start → Visual Studio 2022 → Developer Command Prompt for VS 2022
2. Navigate to project root: `cd C:\path\to\Virtual-Machine`

**Option B: Manual Setup**
```cmd
"C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
```

#### Compile and Link

```cmd
# Navigate to project root
cd C:\path\to\Virtual-Machine

# Create build directory
mkdir build

# Compile all source files
cl /EHsc /std:c++14 /O2 /W3 /I src ^
   src\main.cpp ^
   src\VirtualMachine.cpp ^
   src\CPU.cpp ^
   src\ArithmeticLogicUnit.cpp ^
   src\MemoryIO.cpp ^
   src\Trap.cpp ^
   src\OS.cpp ^
   /Fe:build\vm.exe

# Expected output:
# Microsoft (R) C/C++ Optimizing Compiler ...
# main.cpp
# VirtualMachine.cpp
# CPU.cpp
# ArithmeticLogicUnit.cpp
# MemoryIO.cpp
# Trap.cpp
# OS.cpp
# Generating Code...
# Microsoft (R) Incremental Linker ...
```

**Compiler Flags Explanation:**
- `/EHsc`: Enable C++ exception handling
- `/std:c++14`: Use C++14 standard
- `/O2`: Optimize for speed
- `/W3`: Warning level 3
- `/I src`: Include directory for headers
- `/Fe:build\vm.exe`: Output executable name

#### Debug Build

```cmd
cl /EHsc /std:c++14 /Od /Zi /W4 /I src ^
   src\*.cpp ^
   /Fe:build\vm_debug.exe /Fd:build\vm_debug.pdb
```

**Debug Flags:**
- `/Od`: Disable optimizations
- `/Zi`: Generate debug information
- `/W4`: Warning level 4 (more strict)
- `/Fd`: PDB file for debugging

### Option 2: MinGW/GCC

#### Install MinGW-w64

**Via MSYS2 (Recommended):**
```bash
# Install MSYS2 from https://www.msys2.org/
# Open MSYS2 terminal
pacman -S mingw-w64-x86_64-gcc
```

**Standalone:**
- Download from https://sourceforge.net/projects/mingw-w64/
- Add `bin/` directory to PATH

#### Compile

```bash
# Navigate to project root
cd /c/path/to/Virtual-Machine

# Create build directory
mkdir -p build

# Compile
g++ -std=c++14 -O2 -Wall -Wextra -I src \
    src/main.cpp \
    src/VirtualMachine.cpp \
    src/CPU.cpp \
    src/ArithmeticLogicUnit.cpp \
    src/MemoryIO.cpp \
    src/Trap.cpp \
    src/OS.cpp \
    -o build/vm.exe

# Expected output:
# (No output if successful)
# build/vm.exe created
```

**GCC Flags:**
- `-std=c++14`: C++14 standard
- `-O2`: Optimization level 2
- `-Wall`: Enable all warnings
- `-Wextra`: Extra warnings
- `-I src`: Include directory

#### Debug Build

```bash
g++ -std=c++14 -g -O0 -Wall -Wextra -I src \
    src/*.cpp -o build/vm_debug.exe
```

**Debug Flags:**
- `-g`: Generate debug symbols
- `-O0`: No optimizations

---

## Project Configuration

### Creating a Makefile (Optional)

Create `Makefile` in project root for easier builds:

```makefile
# Compiler
CXX = g++
CXXFLAGS = -std=c++14 -Wall -Wextra -I src

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# Executable
TARGET = $(BUILD_DIR)/vm.exe

# Release build
all: CXXFLAGS += -O2
all: $(TARGET)

# Debug build
debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(TARGET)

# Link
$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@
	@echo "Build complete: $@"

# Compile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Create build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Clean
clean:
	rm -rf $(BUILD_DIR)

# Run 2048
run: all
	$(TARGET) programs/2048.obj

# Run Rogue
run-rogue: all
	$(TARGET) programs/rogue.obj

.PHONY: all debug clean run run-rogue
```

**Usage:**
```bash
make              # Release build
make debug        # Debug build
make clean        # Clean build artifacts
make run          # Build and run 2048
make run-rogue    # Build and run Rogue
```

---

## Running the VM

### Command Line Execution

```cmd
# From project root
build\vm.exe programs\2048.obj

# Or navigate to build directory
cd build
vm.exe ..\programs\2048.obj
```

### Running from Visual Studio

1. **Set Command Arguments**:
   - Right-click project → Properties
   - Debugging → Command Arguments
   - Enter: `programs\2048.obj`

2. **Run**:
   - Press F5 (Debug mode)
   - Or Ctrl+F5 (Run without debugging)

### Verifying Execution

**Successful Launch:**
```
[2048 game screen appears]
Use WASD to play
Score: 0
```

**Error - No Arguments:**
```
usage: vm [image-file1] ...
```

**Error - File Not Found:**
```
failed to load image: programs/2048.obj
```

---

## Troubleshooting

### Build Errors

#### Error: "Cannot open include file: 'Windows.h'"

**Solution:**
- Install Visual Studio with "Desktop development with C++" workload
- Or install Windows SDK

#### Error: "Unresolved external symbol"

**Cause**: Not all source files are included in build

**Solution:**
- Verify all `.cpp` files are added to project
- Check that no files are excluded from build

#### Error: "C2143: syntax error: missing ';'"

**Cause**: Header include order or missing includes

**Solution:**
- Ensure all headers are properly included
- Check for circular dependencies

### Linker Errors

#### LNK2019: Unresolved external symbol 'main'

**Solution:**
- Verify `main.cpp` is included in build
- Check project type is "Console Application"

#### LNK1104: Cannot open file 'build\vm.exe'

**Solution:**
- Close running VM instance
- Disable antivirus temporarily
- Check file permissions

### Runtime Errors

#### "The application was unable to start correctly (0xc000007b)"

**Cause**: 32-bit/64-bit mismatch

**Solution:**
- Rebuild for correct architecture (x64 or x86)
- Match all dependencies to same architecture

#### Immediate crash on launch

**Solution:**
- Run in debugger (F5 in Visual Studio)
- Check exception details
- Verify file paths are correct

### Performance Issues

#### Slow execution

**Solutions:**
1. Build with Release configuration (`/O2` optimization)
2. Disable debug symbols in release build
3. Enable whole program optimization

#### Console flickering

**Solution:**
- Check console buffer settings
- Verify `DisableInputBuffering()` is called once

---

## Build Performance Metrics

| Configuration | Build Time | Executable Size | Optimization |
|---------------|------------|-----------------|--------------|
| Debug | ~5 sec | ~150 KB | None (/Od) |
| Release | ~8 sec | ~50 KB | Max (/O2) |
| MinGW Debug | ~3 sec | ~200 KB | -O0 |
| MinGW Release | ~4 sec | ~80 KB | -O2 |

---

## Testing the Build

### Quick Test

```cmd
# Should display usage message
build\vm.exe

# Should run 2048
build\vm.exe programs\2048.obj

# Test Ctrl+C handling
build\vm.exe programs\rogue.obj
# Press Ctrl+C - should exit cleanly
```

### Validation Checklist

- [ ] Executable builds without errors
- [ ] Executable size is reasonable (< 200 KB)
- [ ] Usage message appears when no arguments provided
- [ ] 2048.obj loads and runs correctly
- [ ] rogue.obj loads and runs correctly
- [ ] Keyboard input is responsive
- [ ] Ctrl+C exits cleanly
- [ ] No memory leaks (check with debugger)

---

## Cross-Platform Considerations

**Current Status:** Windows-only due to:
- `Windows.h` usage in `OS.cpp`
- Console API calls (`SetConsoleMode`, `WaitForSingleObject`)
- `_kbhit()` and `_getch()` functions

**Porting to Linux/macOS would require:**
- Replace Windows console API with `termios` (Linux/macOS)
- Use `poll()` or `select()` for keyboard polling
- Replace `_kbhit()` with custom implementation
- Use ncurses library for terminal control

---

## Additional Resources

- **MSVC Compiler Options**: https://docs.microsoft.com/en-us/cpp/build/reference/compiler-options
- **GCC Documentation**: https://gcc.gnu.org/onlinedocs/
- **C++ Standards**: https://isocpp.org/std/the-standard
- **Debugging in Visual Studio**: https://docs.microsoft.com/en-us/visualstudio/debugger/

---

**Document Version**: 1.0
**Last Updated**: January 2026
**Tested Compilers**: MSVC 2022, MinGW-w64 GCC 11.2
