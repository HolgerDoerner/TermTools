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
 * fpeek - peeks the content of files.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#include "termtools.h"
#include "fpeek_version.h"

#define PEEK_TOP 0
#define PEEK_BOTTOM 1

typedef struct SETTINGS {
    int mode;
    SIZE_T cLines;
    LPWSTR pbFileName;
} SETTINGS;

void parseArgs(SETTINGS *, int, LPWSTR *);
void printHelp(void);

int wmain(int argc, LPWSTR *argv)
{
    setUnicodeLocale();

    SETTINGS settings = {
        .mode = PEEK_TOP,
        .cLines = 10,
        .pbFileName = NULL
    };

    parseArgs(&settings, argc, argv);

    FILE *file;
    _wfopen_s(&file, settings.pbFileName, L"r");

    WCHAR lines[10][1024] = {0};

    SIZE_T cbLine = sizeof(WCHAR) * 512 * 2;
    SIZE_T cvLines = sizeof(LPWSTR) * settings.cLines;
    LPWSTR *pvLines = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cvLines);
    if (!pvLines)
    {
        fwprintf_s(stderr, L"* ERROR: Memory allocation\n");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < (settings.mode ? settings.cLines : 1); ++i)
    {
        pvLines[i] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbLine);
        if (!pvLines[i])
        {
            fwprintf_s(stderr, L"* ERROR: Memory allocation\n");
            exit(EXIT_FAILURE);
        }
    }

    int vectorCounter = 0, lineCounter = 0;

    while (fgetws(pvLines[vectorCounter], 1023, file))
    {
        if (settings.mode == PEEK_BOTTOM)
        {
            if (vectorCounter < settings.cLines-1) ++vectorCounter;
            else vectorCounter = 0;

        }
        else
        {
            wprintf_s(L"%s", pvLines[vectorCounter]);
            if (lineCounter >= settings.cLines) break;
        }
        
        ++lineCounter;

        if (feof(file)) break;
    }

    fclose(file);

    if (settings.mode == PEEK_BOTTOM)
    {
        for (int i = lineCounter % settings.cLines;;)
        {
            wprintf_s(L"%s", pvLines[i++]);

            if (i == settings.cLines) i = 0;
            else if (i == (lineCounter % settings.cLines)) break;
        }
    }

    for (int i = 0; i < settings.cLines; ++i)
    {
        if (pvLines[i]) HeapFree(GetProcessHeap(), 0, pvLines[i]);
    }

    HeapFree(GetProcessHeap(), 0, pvLines);
}

void parseArgs(SETTINGS *_settings, int _argc, LPWSTR *_argv)
{
    if (_argc == 1)
    {
        printHelp();
        exit(EXIT_SUCCESS);
    }

    (*_settings).mode = PEEK_TOP;

    for (int i = 1; i < _argc; ++i)
    {
        if (_argv[i][0] == L'/')
        {
            if (_wcsicmp(_argv[i], L"/?") ==0)
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else if (_wcsicmp(_argv[i], L"/BOT") == 0)
            {
                (*_settings).mode = PEEK_BOTTOM;
            }
            else if (towupper(_argv[i][1]) == L'L')
            {
                LPWSTR tok, context;
                wcstok_s(_argv[i], L":", &context);
                tok = wcstok_s(NULL, L":", &context);

                if (tok)
                {
                    (*_settings).cLines = _wcstoi64(tok, (LPWSTR *)NULL, 10);
                }
                else
                {
                    wprintf_s(L"* Argument /L requires a value.\n");
                    printHelp();
                    exit(EXIT_FAILURE);
                }
                
            }
            else
            {
                fwprintf_s(stderr, L"* ERROR: Unkown argument: %s\n", _argv[i]);
                exit(EXIT_FAILURE);
            }
            
        }
        else
        {
            (*_settings).pbFileName = _argv[i];
        }

        if (!_settings->pbFileName)
        {
            wprintf_s(L"* Missing filename.\n");
            exit(EXIT_FAILURE);
        }
    }
}

void printHelp()
{
    wprintf_s(L"FPEEK.EXE v%hs\n", FPEEK_VERSION);
    wprintf_s(L"\n");
    wprintf_s(L"Usage:\n");
    wprintf_s(L"\tFPEEK.EXE [/? /BOT /L:n] <filename>\n");
    wprintf_s(L"\n");
    wprintf_s(L"Arguments:\n");
    wprintf_s(L"\t/?            - print help\n");
    wprintf_s(L"\t/BOT          - peek lines from the bottom\n");
    wprintf_s(L"\t/L:n          - peek <n> lines (default 10)\n");
}