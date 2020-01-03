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

#ifndef UNICODE 
    #define UNICODE
#endif

#ifndef _UNICODE
    #define _UNICODE
#endif

#define WIN32_LEAN_AND_MEAN
#define USE_LIBCMT

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>

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
 * checks for blank strings.
 * 
 * 'blank' means, the string doesn't contain any printable charakters,
 * but it still MAY contain control charakters like '\n', '\r\n', '\t',
 * '\0' or ' ' (whitespace) so it is *technicaly* not empty. A string
 * is NOT blank if it contains at least 1 printable charakter.
 * An EMPTY string ("") is ALSO BLANK!
 * 
 * _IN:
 *      _line: the string to check (0-terminated)
 *      _length: the length of the string
 * 
 * _RETURNS: 0 (FALSE) if contains printable charakters, otherwise 1 (TRUE)
 */
bool isStringBlank(char *_line, size_t _length)
{
    if (!_line || _length < 1) return true;

    bool _empty = true;

    for (int i = 0; i < _length; ++i)
    {
        if (isgraph(_line[i])) _empty = false;
        else if (_line[i] == '\0') break;
    }

    return _empty;
}

/*
 * checks for blank unicode strings.
 * 
 * 'blank' means, the string doesn't contain any printable charakters,
 * but it still MAY contain control charakters like '\n', '\r\n', '\t',
 * '\0' or ' ' (whitespace) so it is *technicaly* not empty. A string
 * is NOT blank if it contains at least 1 printable charakter.
 * An EMPTY string ("") is ALSO BLANK!
 * 
 * _IN:
 *      _string: the string to check (0-terminated)
 *      _length: the length of the string
 * 
 * _RETURNS: 0 (FALSE) if contains printable charakters, otherwise 1 (TRUE)
 */
bool isStringBlankW(LPWSTR _string, SIZE_T _length)
{
    if (!_string || _length < 1) return true;

    bool _empty = true;

    for (int i = 0; i < _length; ++i)
    {
        if (iswgraph(_string[i])) _empty = false;
        else if (_string[i] == '\0') break;
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

/*
 * converts a byte-buffer into an hexadecimal wide-string.
 * 
 * _IN:
 *      _byteBuffer: the bytes to convert
 *      _size: the size of the output-buffer
 * 
 * _OUT:
 *      _outputBuffer: a wide-character buffer to write the final
 *                      string into
 * 
 * _RETURNS: 0 on success or 1 if _byteBuffer or _outputBuffer are
 *              NULL or _size is zero or negative
 */
int byteToHexStrW(PBYTE _byteBuffer, LPWSTR _outputBuffer, DWORD _size)
{
    if (!_byteBuffer || !_outputBuffer || _size < 1) return 1;

    for(unsigned int j = 0; j < _size; j++)
        wsprintfW(&_outputBuffer[2*j], L"%02X", _byteBuffer[j]);
    
    _outputBuffer[_size] = '\0';

    return 0;
}

#endif // _TERMTOOLS_H