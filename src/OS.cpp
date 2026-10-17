/*
Author: Mehmet Arslan
GitHub: https://github.com/htmos6

This code is licensed under the MIT License.

Copyright � 2024 Mehmet Arslan
*/


#include "./../include/OS.h"

#include <cstdio>

#if defined(_WIN32)
    #include <Windows.h>
    #include <conio.h>  // _kbhit
#elif defined(__linux__)
    #include <termios.h>     // termios, tcgetattr, tcsetattr, tcflush
    #include <unistd.h>      // STDIN_FILENO
    #include <sys/select.h>  // select, fd_set
    #include <cstdlib>       // exit
#else
    #error "Unsupported Operating System!"
#endif


OS::OS()
{

}


void OS::DisableInputBuffering()
{
#if defined(_WIN32)
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);

    // Disable ECHO (characters echoed back to screen) and LINE_INPUT
    // (so reads return immediately instead of waiting for Enter).
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT ^ ENABLE_LINE_INPUT;

    SetConsoleMode(hStdin, fdwMode);
    FlushConsoleInputBuffer(hStdin);
#elif defined(__linux__)
    tcgetattr(hStdin, &fdwOldMode);

    // Clear ICANON (canonical/line-buffered mode) and ECHO so reads return
    // immediately and typed characters are not echoed back to the screen.
    fdwMode = fdwOldMode;
    fdwMode.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(hStdin, TCSANOW, &fdwMode);
    tcflush(hStdin, TCIFLUSH);
#endif
}


void OS::RestoreInputBuffering()
{
#if defined(_WIN32)
    SetConsoleMode(this->hStdin, fdwOldMode);
#elif defined(__linux__)
    tcsetattr(this->hStdin, TCSANOW, &this->fdwOldMode);
#endif
}


std::uint16_t OS::CheckKey()
{
#if defined(_WIN32)
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
#elif defined(__linux__)
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(hStdin, &readfds);

    // Wait up to 1 second, mirroring the WaitForSingleObject timeout on Windows.
    struct timeval timeout = {};
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    return static_cast<std::uint16_t>(select(hStdin + 1, &readfds, nullptr, nullptr, &timeout) > 0);
#endif
}


void OS::HandleInterrupt(int signal)
{
    (void)signal; // Required by the signal-handler signature but unused here
    RestoreInputBuffering();
    printf("\n");
    exit(-2);
}


void OS::HandleInterruptWrapper(int signal)
{
    OS osWrapper;
    osWrapper.HandleInterrupt(signal);
}
