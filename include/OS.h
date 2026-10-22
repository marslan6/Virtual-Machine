/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#ifndef OS_H
#define OS_H

#include <csignal>  // std::sig_atomic_t

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <termios.h>
#include <unistd.h>
#else
#error "Unsupported Operating System!"
#endif

class OS
{
public:
    // ===== Console input mode =====
    void DisableInputBuffering() noexcept;
    void RestoreInputBuffering() noexcept;
    [[nodiscard]] bool CheckKey() noexcept;

    // ===== Interrupt handling =====
    [[nodiscard]] static bool InterruptRequested() noexcept;
    static void HandleInterruptWrapper(int signalNumber) noexcept;

private:
    // ===== State =====
    static volatile std::sig_atomic_t interruptRequested;

#if defined(_WIN32)
    HANDLE hStdin = INVALID_HANDLE_VALUE;
    DWORD fdwMode = 0;
    DWORD fdwOldMode = 0;
#elif defined(__linux__)
    int hStdin = STDIN_FILENO;
    termios fdwMode{};
    termios fdwOldMode{};
#endif
};

#endif // OS_H