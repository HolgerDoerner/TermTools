/* -----------------------------------------------------------------------
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <https://unlicense.org>
 * -----------------------------------------------------------------------
 * 
 * timer - measuresprogram execution time.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#include "timer_version.h"
#include "termtools.h"

#include <time.h>
#include <sys/timeb.h>

void parseArgs(int, LPWSTR *);
void printHelp(void);

int wmain(int argc, LPWSTR *argv)
{
    parseArgs(argc, argv);

    LARGE_INTEGER start, end, diff;
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&start); // start measurement
    
    int subprocess_returncode = _wsystem(argv[1]);

    QueryPerformanceCounter(&end); // end measurement

    diff.QuadPart = end.QuadPart - start.QuadPart;
    diff.QuadPart *= 1000000; // convert ticks to microseconds
    diff.QuadPart /= frequency.QuadPart;

    LONGLONG hours = diff.QuadPart / 3600000000;
    diff.QuadPart %= 3600000000;
    LONGLONG minutes = diff.QuadPart / 60000000;
    diff.QuadPart %= 60000000;
    LONGLONG seconds = diff.QuadPart / 1000000;
    diff.QuadPart %= 1000000;
    LONGLONG milliseconds = diff.QuadPart / 1000;
    diff.QuadPart %= 1000;
    LONGLONG microseconds = diff.QuadPart;

    wprintf_s(L"Hours:        %I64d\n", hours);
    wprintf_s(L"Minutes:      %I64d\n", minutes);
    wprintf_s(L"Seconds:      %I64d\n", seconds);
    wprintf_s(L"Milliseconds: %I64d\n", milliseconds);
    wprintf_s(L"Microseconds: %I64d\n", microseconds);

    if (subprocess_returncode != EXIT_SUCCESS)
    {
        wprintf_s(L"\n");
        wprintf_s(L"Sub-Command returned with Code: %d\n", subprocess_returncode);
    }

    return EXIT_SUCCESS;
}

void parseArgs(int _argc, LPWSTR *_argv)
{
    if (_argc == 1)
    {
        printHelp();
        exit(EXIT_SUCCESS);
    }
    else if (_argv[1][0] == '/')
    {
        if (_wcsicmp(_argv[1], L"/?") == 0)
        {
            printHelp();
            exit(EXIT_SUCCESS);
        }
        else
        {
            fwprintf_s(stderr, L"* ERROR: Unknown argument: %s\n", _argv[1]);
            exit(EXIT_FAILURE);
        }
    }
}

void printHelp()
{
    wprintf_s(L"timer v%hs\n", TIMER_VERSION);
    wprintf_s(L"\n");
    wprintf_s(L"Usage:\n");
    wprintf_s(L"\ttimer.exe [/?] <COMMAND>\n");
    wprintf_s(L"\n");
    wprintf_s(L"Arguments:\n");
    wprintf_s(L"\tCOMMAND       - The command to measure\n");
    wprintf_s(L"\t/?            - Print help\n");
    wprintf_s(L"\n");
    wprintf_s(L"If the command takes Arguments by itself, it's call has to be quoted.\n");
    wprintf_s(L"Eg: timer.exe \"DIR /S C:\\Windows\\System32\"\n");
}