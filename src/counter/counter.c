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
#include "counter_version.h"

#define CLINES 0
#define CWORDS 1

short parseArgs(int, LPWSTR *, LPWSTR *);
SIZE_T count(LPCWSTR, short);
void usage(void);

int wmain(int argc, LPWSTR *argv)
{
    setUnicodeLocale();
    
    if (argc == 1)
    {
        usage();
        exit(0);
    }

    SIZE_T ret;
    LPWSTR fileName;

    switch (parseArgs(argc, argv, &fileName))
    {
        case -1:
            exit(1);
        case 1:
            ret = count(fileName, CLINES);
            break;
        case 2:
            ret = count(fileName, CWORDS);
            break;
        default:
            usage();
            exit(0);
    }

    wprintf(L"%zd\n", ret);
    
    return 0;
}

short parseArgs(int argc, LPWSTR *argv, LPWSTR *fileName)
{
    int switchCount = 0;
    short retCode = 1;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '/')
        {
            if (_wcsicmp(argv[i], L"/?") == 0)
            {
                retCode = 0;
                break;
            }
            else if (_wcsicmp(argv[i], L"/w") == 0)
            {
                ++switchCount;
                retCode = 2;
            }
            else
            {
                wprintf(L"Unkown parameter '%s'\n", argv[i]);
                return -1;
            }
        }
        else
        {
            *fileName = argv[i];
        }
    }

    if (switchCount > 1)
    {
        retCode = -1;
    }

    return retCode;
}

SIZE_T count(LPCWSTR fileName, short mode)
{
    FILE *pFile;
    if (_wfopen_s(&pFile, fileName, L"r"))
    {
        WCHAR errMsg[BUFSIZ];
        _wcserror_s((WCHAR *)&errMsg, BUFSIZ, errno);
        _fwprintf_p(stderr, L"* %s: %s\n", fileName, errMsg);
        exit(1);
    }

    SIZE_T counter = 0;
    LPCWSTR delimeter = L" \t\n";
    LPWSTR tok, context;

    WCHAR buffer[BUFSIZ];
    while (fgetws(buffer, BUFSIZ-1, pFile))
    {
        if (isStringBlankW(buffer, BUFSIZ)) continue;

        if (mode == CWORDS)
        {
            tok = wcstok_s(buffer, delimeter, &context);

            while (tok)
            {
                ++counter;
                tok = wcstok_s(NULL, delimeter, &context);
            }
        }

        if (mode == CLINES) ++counter;

        if (feof(pFile)) break;
    }

    if (fclose(pFile))
    {
        perror("* ERROR");
        exit(1);
    }

    return counter;
}

void usage()
{
    wprintf(L"counter - Counts Lines or Words\n");
    wprintf(L"\n");
    wprintf(L"Usage:\n");
    wprintf(L"\tcounter.exe [/W] <filename>\n");
    wprintf(L"\n");
    wprintf(L"Parameter:\n");
    wprintf(L"\t/W            = counts words instead of lines\n");
    wprintf(L"\t<filename>    = path/name of the file to read\n");
}