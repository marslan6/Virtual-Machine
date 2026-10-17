/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#ifndef OS_H
#define OS_H

#include <cstdint>

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
private:
    #if defined(_WIN32)
        HANDLE hStdin = INVALID_HANDLE_VALUE;
        DWORD fdwMode, fdwOldMode;
    #elif defined(__linux__)
        int hStdin = STDIN_FILENO;
        struct termios fdwMode, fdwOldMode;
    #endif
    
public:
    OS();
    void DisableInputBuffering();
    void RestoreInputBuffering();
    std::uint16_t CheckKey();
    void HandleInterrupt(int signal);
    static void HandleInterruptWrapper(int signal);
};
#endif