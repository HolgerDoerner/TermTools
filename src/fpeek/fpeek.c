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
 * counter - a simple line- and wordcounter for text-files.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#include "termtools.h"
#include "fpeek_version.h"

int parseArgs(int, LPWSTR *, LPWSTR *);

int wmain(int argc, LPWSTR *argv)
{
    setUnicodeLocale();

    LPWSTR pbFileName = NULL;
    int mode = parseArgs(argc, argv, &pbFileName);
    if (!pbFileName) return 1;

    FILE *file;
    _wfopen_s(&file, pbFileName, L"r");

    WCHAR lines[10][1024] = {0};

    int j=0, k=0;

    while (fgetws(lines[j], 1023, file))
    {
        if (mode == 1)
        {
            if (j < 9) ++j;
            else j = 0;

        }
        else
        {
            wprintf_s(L"%s", lines[j]);
            if (k == 9) break;
        }
        
        k++;

        if (feof(file)) break;
    }

    fclose(file);

    // printf("%d %d\n", k, k%10);

    if (mode == 1)
    {
        for (int i = k%10;;)
        {
            wprintf_s(L"%s", lines[i++]);

            if (i == 10) i = 0;
            else if (i == (k%10)) break;
        }
    }
}

int parseArgs(int _argc, LPWSTR *_argv, LPWSTR *_fileName)
{
    if (_argc == 1)
    {
        // TODO
    }

    int mode = 0;

    for (int i = 1; i < _argc; ++i)
    {
        if (_argv[i][0] == L'/')
        {
            if (_wcsicmp(_argv[i], L"/?") ==0)
            {
                // TODO
            }
            else if (_wcsicmp(_argv[i], L"/TOP") == 0)
            {
                // TODO
            }
            else if (_wcsicmp(_argv[i], L"/BOT") == 0)
            {
                mode = 1;
            }
            else if (_wcsicmp(_argv[i], L"/L") == 0)
            {
                // TODO
            }
            else
            {
                // TODO
            }
            
        }
        else
        {
            (*_fileName) = _argv[i];
        }
        
    }

    return mode;
}