/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright © 2024 Mehmet Arslan
*/


#ifndef OS_H
#define OS_H


#include <Windows.h>
#include <cstdint>


class OS
{
private:
    // Handle to the standard input device.
    HANDLE hStdin = INVALID_HANDLE_VALUE;
    // Variables to store the input mode flags.
    DWORD fdwMode, fdwOldMode;

public:
    OS();
    void DisableInputBuffering();
    void RestoreInputBuffering();
    uint16_t CheckKey();
    void HandleInterrupt(int signal);
    static void HandleInterruptWrapper(int signal);
};
#endif