# Minimum CMake version required to configure this project.
# 3.20 is chosen because it's the first version that knows about C++23.
# Ref: https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
cmake_minimum_required(VERSION 3.20)

# Force this project to build with GCC 15.
set(CMAKE_C_COMPILER gcc-15)
set(CMAKE_CXX_COMPILER g++-15)

# Project name (and implicitly enables C and C++ support).
# Required exactly once per project.
# TODO: Configure project name
project(CMake-Cpp-23-Template)

# Emit out/compile_commands.json: the exact compiler flags for every source
# file. clang-tidy (and clangd) read it so they analyze the code with the same
# standard, warnings and include paths that the real build uses.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Target C++23. This sets the default C++ standard for all targets below.
set(CMAKE_CXX_STANDARD 23)

# Make C++23 a hard requirement: if the compiler can't do it, fail the
# configure step instead of silently falling back to an older standard.
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Define the executable "executableBinary" and list its source files.
# Add more .cpp files here as the project grows, e.g. src/OrderBook.cpp.
# TODO: Configure executable name
add_executable(executableBinary src/main.cpp)

# TODO: Configure executable name
# Warnings — after the target, scoped to it. Warnings often hint at bugs.
# Add these flags to just executableBinary.bin. Scoped. Precise.
target_compile_options(executableBinary PRIVATE -Wall -Wextra)