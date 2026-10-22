/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright (c) 2024 Mehmet Arslan
*/

#include "./../include/OS.h"

#if defined(_WIN32)
#include <windows.h>
#include <conio.h>       // _kbhit
#elif defined(__linux__)
#include <termios.h>     // termios, tcgetattr, tcsetattr, tcflush
#include <unistd.h>      // STDIN_FILENO
#include <sys/select.h>  // select, fd_set
#endif

// ===== Static members =====

volatile std::sig_atomic_t OS::interruptRequested = 0;

// ===== Console input mode =====

/// Switches the console to unbuffered, no-echo input so reads return a single
/// keypress immediately. Saves the previous mode for RestoreInputBuffering.
void OS::DisableInputBuffering() noexcept
{
#if defined(_WIN32)
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);

    // Clear echo and line-input so reads return immediately.
    fdwMode = fdwOldMode & ~(ENABLE_ECHO_INPUT | ENABLE_LINE_INPUT);

    SetConsoleMode(hStdin, fdwMode);
    FlushConsoleInputBuffer(hStdin);
#elif defined(__linux__)
    tcgetattr(hStdin, &fdwOldMode);

    // Clear ICANON and ECHO so reads return immediately and are not echoed.
    fdwMode = fdwOldMode;
    fdwMode.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(hStdin, TCSANOW, &fdwMode);
    tcflush(hStdin, TCIFLUSH);
#endif
}

/// Restores the console input mode saved by DisableInputBuffering.
void OS::RestoreInputBuffering() noexcept
{
#if defined(_WIN32)
    SetConsoleMode(hStdin, fdwOldMode);
#elif defined(__linux__)
    tcsetattr(hStdin, TCSANOW, &fdwOldMode);
#endif
}

/// Polls, without blocking, whether a key press is available.
/// @return true if a key is ready to read.
bool OS::CheckKey() noexcept
{
#if defined(_WIN32)
    return (WaitForSingleObject(hStdin, 0) == WAIT_OBJECT_0) && _kbhit();
#elif defined(__linux__)
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(hStdin, &readfds);

    timeval timeout{};  // zero timeout: poll, do not block
    return select(hStdin + 1, &readfds, nullptr, nullptr, &timeout) > 0;
#endif
}

// ===== Interrupt handling =====

/// @return true once an interrupt (Ctrl+C) has been requested.
bool OS::InterruptRequested() noexcept
{
    return interruptRequested != 0;
}

/// Signal handler for interrupts (Ctrl+C). Only records the request by setting
/// a flag, which is the one action safe to perform inside a signal handler; the
/// main loop observes the flag and shuts down through normal control flow.
/// @param signalNumber The delivered signal number (unused).
void OS::HandleInterruptWrapper([[maybe_unused]] int signalNumber) noexcept
{
    interruptRequested = 1;
}