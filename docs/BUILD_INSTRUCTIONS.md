# Build Instructions — LC-3 Virtual Machine

How to build and run the emulator. The build is driven by **CMake** (≥ 3.20)
with **GCC 15** targeting **C++23**, and works on **Linux and Windows**.

For the full toolchain setup, editor/clang-tidy integration, and a CMake
directive reference, see [README_CMAKE.md](README_CMAKE.md). This document is
the quick, task-focused version.

## Table of Contents
1. [Prerequisites](#prerequisites)
2. [Building](#building)
3. [Running the VM](#running-the-vm)
4. [Debug builds](#debug-builds)
5. [Troubleshooting](#troubleshooting)

---

## Prerequisites

| Requirement | Notes |
|-------------|-------|
| **OS** | Linux or Windows (the platform layer lives in `OS` and is selected at compile time) |
| **Compiler** | GCC 15 (`gcc-15` / `g++-15`) — the standard is set to C++23 and is **required** |
| **Build system** | CMake ≥ 3.20 (first version aware of C++23) |
| **Standard library** | a libstdc++ providing `<expected>`, `<span>`, `<bit>` |
| **Debugger** (optional) | gdb |

The C++23 features the code depends on include `std::expected`, `std::span`,
`std::byteswap`, `std::endian`, and `std::to_underlying`. A compiler older than
GCC 15 will not build it.

> On Ubuntu, GCC 15 comes from the toolchain PPA
> (`ppa:ubuntu-toolchain-r/test`). The exact `apt` steps are in
> [README_CMAKE.md](README_CMAKE.md#prerequisites).

The CMake toolchain is pinned to `gcc-15` / `g++-15` in
[CMakeLists.txt](../CMakeLists.txt). To use a different compiler, change
`CMAKE_C_COMPILER` / `CMAKE_CXX_COMPILER` there, or pass
`-DCMAKE_CXX_COMPILER=...` at configure time.

---

## Building

The simplest path is the [`makefile`](../makefile), a thin wrapper over CMake.
Run everything from the project root.

```bash
make            # configure (if needed) + build into out/
make rebuild    # clean reconfigure + build (use after changing compiler/flags)
make clean      # delete the out/ build directory
```

The binary is produced at **`out/executableBinary`**. Sources are globbed from
`src/` by CMake, so adding or removing a `.cpp` is picked up automatically on
the next build.

### Using CMake directly

```bash
cmake -B out          # configure: read CMakeLists.txt, generate build files
cmake --build out     # compile
```

The build enables `-Wall -Wextra` and exports `out/compile_commands.json` for
clang-tidy / clangd.

---

## Running the VM

The VM takes one or more image **names** on the command line. Each name is
resolved by `CPU::LoadImage` under the image directory (`images/`, the
`kImageDir` constant), and a `.obj` extension is appended when none is given —
so `2048` loads `images/2048.obj`. **Pass the bare name, not a path.**

### Convenience targets

These run the already-built binary; they do **not** rebuild (build first with
`make` / `make rebuild`):

```bash
make run-2048     # → out/executableBinary 2048   (loads images/2048.obj)
make run-rogue    # → out/executableBinary rogue  (loads images/rogue.obj)
```

### Running the binary directly

```bash
./out/executableBinary 2048
./out/executableBinary rogue
```

### Expected behavior

- **No argument** → the VM throws `no image file provided` and exits non-zero.
- **Unresolved name** → `failed to load image file` (the name did not resolve to
  a readable `images/<name>.obj`).
- **Success** → the game renders; use **WASD** to play, **Ctrl+C** to interrupt.
  The terminal's input mode is always restored on exit (an RAII guard handles
  every exit path, including exceptions).

---

## Debug builds

The default `make` build has no debug symbols. For gdb, build the Debug config:

```bash
make debug                    # cmake -B out --fresh -DCMAKE_BUILD_TYPE=Debug
gdb --args ./out/executableBinary 2048
```

VS Code debugging is preconfigured under `.vscode/` (F5 runs `make debug` via
`preLaunchTask`, then launches gdb). See
[README_CMAKE.md](README_CMAKE.md#debugging) for the details and how to pass
command-line arguments through `launch.json`.

---

## Troubleshooting

#### Configure fails: `gcc-15` / `g++-15` not found
Install GCC 15 (see Prerequisites), or point CMake at your compiler:
`cmake -B out -DCMAKE_CXX_COMPILER=g++-15`. After changing the compiler, run
`make rebuild` so the cached configuration is discarded.

#### Build errors about `std::expected`, `std::span`, or `std::byteswap`
The compiler or its standard library is too old for C++23. Confirm `g++-15
--version` and that it is the compiler CMake selected (check
`out/compile_commands.json`).

#### `failed to load image file`
The image name did not resolve. Remember the lookup rule: the argument is a bare
name resolved under `images/` with `.obj` appended. Verify the file exists:
`ls images/`. A path like `tests/2048.obj` will **not** work — it becomes
`images/tests/2048.obj`.

#### `no image file provided`
The binary was run with no argument. Pass an image name (e.g. `2048`).

#### Keyboard input is unresponsive, or the terminal is left in a weird state
Input is switched to raw/no-echo mode while running and restored on exit. If a
hard kill (e.g. `kill -9`) skips the restore, run `reset` to fix the terminal.

#### `#error "Unsupported Operating System!"`
`OS` only implements `_WIN32` and `__linux__`. Other platforms are not supported.

---

## Cross-platform status

The emulator builds and runs on **Linux and Windows**. All platform-specific
code is confined to [`OS`](../src/OS.cpp), chosen at compile time:

| Concern | Windows (`_WIN32`) | Linux (`__linux__`) |
|---------|--------------------|---------------------|
| Raw/no-echo input | Console API (`GetConsoleMode` / `SetConsoleMode`) | `termios` (clear `ICANON | ECHO`) |
| Non-blocking key poll | `WaitForSingleObject` + `_kbhit` | zero-timeout `select` |

Porting to another platform means adding a branch to `OS` for these two
concerns; nothing else is platform-dependent.

---

_Keep this document in sync with [`makefile`](../makefile),
[`CMakeLists.txt`](../CMakeLists.txt), and the sources under
[`src/`](../src) and [`include/`](../include)._
