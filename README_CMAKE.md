# CMAKE Project Template

A minimal CMake project template targeting C++23 with GCC 15.

## Prerequisites

### Build (required)

- **CMake 3.20+** — first version that knows about C++23.
  ```sh
  sudo apt install cmake
  ```
- **GCC 15** (`gcc-15` / `g++-15`) — required for C++23 support (`<print>` and
  friends). Not in Ubuntu 24.04's default repos, so add the toolchain PPA:
  ```sh
  sudo add-apt-repository ppa:ubuntu-toolchain-r/test
  sudo apt update
  sudo apt install gcc-15 g++-15
  ```
- **gdb** — for debugging.
  ```sh
  sudo apt install gdb
  ```

### Editor tooling (optional — clang-tidy and IntelliSense)

**LLVM 22** supplies `clangd-22` (language server) and `clang-tidy-22` (static
analysis). Prefer a recent LLVM: Ubuntu 24.04's own package is clang-tidy 18,
which predates GCC 15's libstdc++ and may not parse its C++23 headers. Install
from apt.llvm.org with the bundled `llvm.sh`:

```sh
chmod +x llvm.sh
sudo ./llvm.sh 22
sudo apt install clangd-22 clang-tidy-22
```

Binaries are version-suffixed (`clangd-22`, not `clangd`); the project config
refers to them by full path, so no symlinks are needed.

### VS Code extensions

| Extension | ID | Used for |
|-----------|----|----------|
| **clangd** | `llvm-vs-code-extensions.vscode-clangd` | IntelliSense, and live clang-tidy findings in the Problems panel |
| **C/C++** | `ms-vscode.cpptools` | Debugging only (gdb integration) |

> These two conflict over IntelliSense — run only one. This project uses clangd
> for IntelliSense and cpptools purely as a debugger front end, which means
> user settings should keep `"C_Cpp.intelliSenseEngine": "disabled"`. The clangd
> extension offers to set that for you the first time both are enabled.

## Project layout

| Path             | Purpose                          |
|------------------|----------------------------------|
| `CMakeLists.txt` | Build configuration              |
| `makefile`       | Convenience wrapper around CMake  |
| `.clang-tidy`    | Static analysis rules            |
| `src/`           | Source files (`main.cpp`)         |
| `include/`       | Public headers                   |
| `tests/`         | Tests                            |
| `bench/`         | Benchmarks                       |
| `docs/`          | Documentation                    |
| `out/`           | Generated build files (git-ignored) |

## Build & run

Everything goes through the `makefile` (run from the project root):

```sh
make          # configure + build
make run      # run the program
make debug    # fresh configure + build with debug info (CMAKE_BUILD_TYPE=Debug)
make rebuild  # clean rebuild
make clean    # delete the build folder
```

| Target         | Action                                            |
|----------------|---------------------------------------------------|
| `make` / `make all` | Configure (if needed) and build              |
| `make run`     | Run the built program                              |
| `make debug`   | Fresh configure + build with `CMAKE_BUILD_TYPE=Debug` (debug symbols) |
| `make rebuild` | Throw away cached config, reconfigure, rebuild     |
| `make clean`   | Delete the whole build folder                      |

## Static analysis (clang-tidy)

Rules live in `.clang-tidy` at the project root — most check families enabled,
a few noisy ones switched back off.

clangd runs clang-tidy as you type and reports findings in the **Problems**
panel. There is no build step or task to run. It reads the compiler flags from
`out/compile_commands.json`, which CMake writes on every configure (see
`CMAKE_EXPORT_COMPILE_COMMANDS` in `CMakeLists.txt`), so run `make` once on a
fresh clone before expecting diagnostics.

To silence a check, add it to the disabled list in `.clang-tidy`:

```yaml
  -cppcoreguidelines-pro-bounds-pointer-arithmetic,
```

Running `clang-tidy-22` directly reports slightly more than the editor does:
clangd skips checks needing whole-program analysis (`bugprone-exception-escape`,
for one) as too slow for live editing.

```sh
clang-tidy-22 -p out src/main.cpp
```

## Debugging

The default `make` build has no `-g` symbols — always build with `make debug` first.

### Command line (gdb)

```sh
make debug
gdb ./out/executableBinary
```

### VS Code

The `.vscode/` folder is preconfigured. Requires the **C/C++** extension
(`ms-vscode.cpptools`) and `gdb` on `PATH`. cpptools is used only as the
debugger front end here — clangd provides IntelliSense (see *Prerequisites*).

1. Open a source file and click the gutter to set a breakpoint.
2. Open the Run panel (`Ctrl+Shift+D`), select **Debug** in the dropdown,
   and press `F5`.
3. `F5` runs the `make debug` task first (via `preLaunchTask`), then launches
   gdb on `out/executableBinary`.

Stepping: `F5` continue, `F10` step over, `F11` step into, `Shift+F11` step out.
Inspect locals in the **Variables** pane or type expressions in the
**Debug Console** (`-exec p argc`).

To pass command-line arguments, edit `"args"` in `.vscode/launch.json`:

```json
"args": ["Istanbul"]
```

| File | Role |
|------|------|
| `.vscode/launch.json` | Debugger config — which binary, gdb, `preLaunchTask` |
| `.vscode/tasks.json`  | `make` wrappers; `make debug` is the pre-launch build |
| `.vscode/c_cpp_properties.json` | cpptools IntelliSense config — inert while clangd owns IntelliSense |
| `.vscode/settings.json` | Points clangd at `clangd-22` with `--clang-tidy`; disables CMake Tools auto-configure and the C/C++ "debug active file" shortcut |
| `.clang-tidy` | Which clang-tidy checks run (project root, not `.vscode/`) |

---

## Appendix: using CMake directly

The `makefile` targets above are thin wrappers around these commands. Use them
directly only if you need a step the wrapper doesn't cover.

### Configure
Reads `CMakeLists.txt` and generates the build files into the `out` folder.
Run from the same directory that contains `CMakeLists.txt`:
```sh
cmake -B out
```

### Build
Compiles the generated build system in the `out` folder:
```sh
cmake --build out
```

### Run
```sh
./out/executableBinary
```

### Debug build
Reconfigure with debug symbols (no optimization, full `-g`):
```sh
cmake -B out --fresh -DCMAKE_BUILD_TYPE=Debug
cmake --build out
```

### Clean rebuild
Discards the cached configuration and reconfigures from scratch. Use this
after changing the compiler or other cached settings:
```sh
cmake -B out --fresh
cmake --build out
```

### Setting variables on the command line
Any CMake variable can be set at configure time with `-D`:
```sh
cmake -B out -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

## Appendix: CMake directive reference

Reference for the directives used (or usable) in `CMakeLists.txt`.

### `CXX_STANDARD`
The `CXX_STANDARD` property specifies which C++ standard to use. CMake version
requirements by standard:
- C++11/C++14: CMake 3.1+
- C++17: CMake 3.8+
- C++20: CMake 3.12+
- C++23: CMake 3.20+ (required for this project)
- C++26: CMake 3.25+

See [CMake CXX_STANDARD documentation](https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html) for details.

### `add_executable`
```cmake
add_executable(myprogram a.cpp b.cpp)
```
Defines an executable named `myprogram` built from `a.cpp` and `b.cpp`.
List every `.cpp` file that makes up the program.

### `add_compile_options`
```cmake
add_compile_options(-Wall -Wextra)
```
Adds flags to the compiler for all targets. `-Wall -Wextra` enable the common
warning sets and are strongly recommended for catching bugs early.
(`-Wpedantic` can be added to warn about non-standard extensions.)

### `set(CMAKE_CXX_COMPILER ...)`
```cmake
set(CMAKE_CXX_COMPILER clang++)
```
Chooses the C++ compiler. Must be set **before** `project()` to take effect.
Prefer passing it on the command line instead, which keeps `CMakeLists.txt`
portable:
```sh
cmake -B out -DCMAKE_CXX_COMPILER=clang++
```

### `set(CMAKE_BUILD_TYPE ...)`
```cmake
set(CMAKE_BUILD_TYPE Debug)
```
Sets the build type, which controls optimization and debug info:

| Type | Optimization | Debug info | Use for |
|------|--------------|------------|---------|
| `Debug` | none | full | development |
| `Release` | full | none | shipping |
| `RelWithDebInfo` | full | yes | profiling / crash reports |

> Note: `CMAKE_BUILD_TYPE` applies to single-config generators like the Unix
> Makefiles used here. Multi-config generators (Visual Studio, Ninja
> Multi-Config) instead pick the type at build time:
> `cmake --build out --config Release`.
