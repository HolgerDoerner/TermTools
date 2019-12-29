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


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/timeb.h>

#include "timer_version.h"
#include "termtools.h"

void parseArgs(int, char**);
void printHelp(void);

int main(int argc, char **argv)
{
    parseArgs(argc, argv);

    LARGE_INTEGER start, end, diff;
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);

    QueryPerformanceCounter(&start); // start measurement

    FILE *pStdout = _popen(argv[1],"rt");
    if (!pStdout)
    {
        if (errno) perror("* ERROR");
        exit(EXIT_FAILURE);
    }

    char dummy[BUFSIZ];
    while (fgets(dummy, BUFSIZ, pStdout))
        if (feof(pStdout)) break;

    int subprocess_returncode = _pclose(pStdout);

    QueryPerformanceCounter(&end); // end measurement

    LONGLONG ticks = diff.QuadPart = end.QuadPart - start.QuadPart;
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

    printf("Ticks:        %I64d\n", ticks);
    printf("\n");
    printf("Hours:        %I64d\n", hours);
    printf("Minutes:      %I64d\n", minutes);
    printf("Seconds:      %I64d\n", seconds);
    printf("Milliseconds: %I64d\n", milliseconds);
    printf("Microseconds: %I64d\n", microseconds);

    if (subprocess_returncode != EXIT_SUCCESS)
    {
        printf("\n");
        printf("Sub-Command returned with Code: %d\n", subprocess_returncode);
    }

    return EXIT_SUCCESS;
}

void parseArgs(int _argc, char **_argv)
{
    if (_argc == 1)
    {
        printHelp();
        exit(EXIT_SUCCESS);
    }
    else if (_argv[1][0] == '/')
    {
        if (_stricmp(_argv[1], "/?") == 0)
        {
            printHelp();
            exit(EXIT_SUCCESS);
        }
        else
        {
            fprintf(stderr, "* ERROR: Unknown argument: %s\n", _argv[1]);
            exit(EXIT_FAILURE);
        }
    }
}

void printHelp()
{
    printf("timer v%s\n", TIMER_VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("\ttimer.exe [/?] <COMMAND>\n");
    printf("\n");
    printf("Arguments:\n");
    printf("\tCOMMAND       - The command to measure\n");
    printf("\t/?            - Print help\n");
    printf("\n");
    printf("If the command takes Arguments ba itself, it's call has to be quoted\n");
    printf("eg: \"DIR /S C:\\Windows\\System32\"\n");
}