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

#include "tt_version.h"
// #define TT_VERSION_MAJOR 0
// #define TT_VERSION_MINOR 3
// #define TT_VERSION_PATCH 0

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

/*
 * converts bytes to kilobytes.
 * 
 * _IN:
 *      _bytes: number of bytes to be converted
 * 
 * _RETURNS: the converted number in kilobytes
 */
long double byteToKilo(unsigned long long _bytes)
{
    return _bytes / 1024.0l;
}

/*
 * converts bytes to megabytes.
 * 
 * _IN:
 *      _bytes: number of bytes to be converted
 * 
 * _RETURNS: the converted number in megabytes
 */
long double byteToMega(unsigned long long _bytes)
{
    return _bytes / 1024.0l / 1024.0l;
}

/*
 * converts bytes to gigabytes.
 * 
 * _IN:
 *      _bytes: number of bytes to be converted
 * 
 * _RETURNS: the converted number in gigabytes
 */
long double byteToGiga(unsigned long long _bytes)
{
    return _bytes / 1024.0l / 1024.0l / 1024.0l;
}

/*
 * converts megaherz to gigaherz.
 * 
 * _IN:
 *      _bytes: number of megaherz to be converted
 * 
 * _RETURNS: the converted number in gigaherz
 */
double mhzToGhz(long _hz)
{
    return _hz / 1000.0l;
}

/*
 * takes a substring out of a bigger string and writes
 * it to a new location.
 * 
 * the new string will be null-terminated, so the target
 * location has to be at least _length + 1.
 * source string is not altered.
 * 
 * _IN:
 *      _string: the source-string
 *      _start: the position where the substring starts
 *      _length: the lengts of the substring
 * 
 * _OUT:
 *      _substring: the location to store the substring into
 * 
 * _RETURNS: the number of charakters copied or (0) on error
 */
int subString(char *_string, char *_substring, int _start, int _length)
{
    size_t aLength = sizeof(_substring) / sizeof(_substring[0]);

    if (!_string || !_substring || _start < 0 || _length <= 0 || _length > aLength-1)
        return 0;

    int i;

    for(i = 0; i < _length; ++i)
    {
        _substring[i] = _string[_start + i];
    }

    _substring[_length] = '\0';

    return i;
}

#endif // _TERMTOOLS_H