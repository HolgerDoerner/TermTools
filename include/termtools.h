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
 * termtools.h - utility-header for applications of the TermTools-Project.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#ifndef _TERMTOOLS_H
#define _TERMTOOLS_H

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/*
 * returns the last element of a path.
 *
 * _IN:
 *      path: the full path
 *
 * RETURNS: a pointer to the start of the last element
 *           or NULL.
 */
const char *basename(const char *_path)
{
    if (!_path) return NULL;

    int pathsep = '\\'; //sorry, Windows only...

    const char *tmp = strrchr(_path, pathsep);
    if (tmp) return ++tmp;
    else return tmp;
}

/*
 * calculates the number of digits for a given number.
 *
 * _IN:
 *      number: a number
 * 
 * RETURNS: the number of digits, minimum 1
 */
unsigned char countDigits(long long _number)
{
    unsigned char count = 1;

    if (_number >= 10)
    {
        while (_number /= 10) ++count;
    }
    
    return count;
}

/*
 * checks for blank lines/strings.
 * 
 * 'blank' means, the string doesn't contain any printable charakters,
 * but it still MAY contain control charakters like '\n', '\r\n', '\t',
 * '\0' or ' ' (whitespace) so it is *technicaly* not empty. A string
 * is NOT blank if it contains at least 1 printable charakter.
 * An EMPTY string ("") is ALSO BLANK!
 * 
 * _IN:
 *      _line: the line/string to check
 * 
 * _RETURNS: 0 (FALSE) if contains printable charakters, otherwise 1 (TRUE)
 */
int isLineBlank(char *_line)
{
    if (!_line) return 0;

    size_t len = strlen(_line);
    if (len < 1) return 0;

    bool _empty = true;

    for (int i = 0; i < len; ++i)
    {
        if (iswprint(_line[i])) _empty = false;
    }

    return _empty;
}

#endif // _TERMTOOLS_H