# Minimum CMake version required to configure this project.
# 3.20 is chosen because it's the first version that knows about C++23.
# Ref: https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
cmake_minimum_required(VERSION 3.20)

# Pin GCC 15 on non-Windows hosts, unless the user already chose a compiler
# (e.g. via -DCMAKE_CXX_COMPILER or a toolchain file). On Windows the native
# toolchain (MSVC, or MinGW if the user selects it) is used instead, so the
# project configures out of the box there too. CMAKE_HOST_WIN32 is set before
# project(), unlike WIN32, so it is safe to test here.
if(NOT CMAKE_HOST_WIN32 AND NOT DEFINED CMAKE_CXX_COMPILER)
    set(CMAKE_C_COMPILER gcc-15)
    set(CMAKE_CXX_COMPILER g++-15)
endif()

# Project name (and implicitly enables C and C++ support).
# Required exactly once per project.
# TODO: Configure project name
project(Virtual-Machine)

# Emit out/compile_commands.json: the exact compiler flags for every source
# file. clang-tidy (and clangd) read it so they analyze the code with the same
# standard, warnings and include paths that the real build uses.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Target C++23. This sets the default C++ standard for all targets below.
set(CMAKE_CXX_STANDARD 23)

# Make C++23 a hard requirement: if the compiler can't do it, fail the
# configure step instead of silently falling back to an older standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Collect every .cpp under src/ into SOURCES, then build them into one
# executable. GLOB avoids listing all 20 files by hand. CONFIGURE_DEPENDS makes
# CMake re-check the glob on each build, so adding or deleting a .cpp is picked
# up automatically without manually re-running cmake — the main downside of
# globbing. Ref: https://cmake.org/cmake/help/latest/command/file.html#filesystem
file(GLOB SOURCES CONFIGURE_DEPENDS "src/*.cpp")
add_executable(executableBinary ${SOURCES})

# TODO: Configure executable name
# Warnings — after the target, scoped to it. Warnings often hint at bugs.
# The flags differ per compiler: GCC/Clang use -Wall -Wextra, MSVC uses /W4.
if(MSVC)
    target_compile_options(executableBinary PRIVATE /W4)
else()
    target_compile_options(executableBinary PRIVATE -Wall -Wextra)
endif()