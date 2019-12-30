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
 * pager - a simple terminal pager
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define USE_LIBCMT

#include <windows.h>
#include <bcrypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>

#include "termtools.h"
#include "hashsum_version.h"

#pragma comment(lib, "bcrypt.lib")

#define STATUS_SUCCESSFUL 0x00000000L
#define STATUS_UNSUCCESSFUL 0xC0000001L

typedef struct SETTINGS {
    BCRYPT_ALG_HANDLE hAlg;
    BCRYPT_HASH_HANDLE hHash;
    DWORD cbData;
    DWORD cbHash;
    DWORD cbHashObject;
    PBYTE pbHashObject;
    PBYTE pbHash;
    NTSTATUS status;
} SETTINGS;

NTSTATUS initializeCryptoAPI(SETTINGS *);
void cleanup(SETTINGS *);

int wmain(int argc, LPWSTR **argv)
{
    SETTINGS settings = {
        .hAlg = NULL,
        .hHash = NULL,
        .cbData = 0,
        .cbHash = 0,
        .cbHashObject = 0,
        .pbHashObject = NULL,
        .pbHash = NULL,
        .status = STATUS_SUCCESSFUL
    };

    for (int i = 1; i < argc; ++i)
    {
        if (initializeCryptoAPI(&settings) != STATUS_SUCCESSFUL) exit(EXIT_FAILURE);

        FILE *pFile = _wfopen((const WCHAR*)argv[i], L"rb");
        if (!pFile)
        {
            printf("error open file\n");
        }

        PBYTE buff = malloc(sizeof(LPWSTR) * BUFSIZ);
        if (!buff)
        {
            printf("error allocating buff\n");
        }

        while (fread(buff, sizeof(BYTE), BUFSIZ, pFile))
        {
            //hash some data
            if((settings.status = BCryptHashData(settings.hHash, buff, BUFSIZ, 0)))
            {
                fwprintf(stderr, L"* ERROR: hashing data failed with code: %ld\n", settings.status);
                exit(EXIT_FAILURE);
            }

            if (feof(pFile)) break;
        }
        
        //close the hash
        if(BCryptFinishHash(settings.hHash, settings.pbHash, settings.cbHash, 0))
        {
            fwprintf(stderr, L"* ERROR: in BCryptFinishHash()\n");
        }

        settings.cbHash *= 2;

        LPWSTR lpwOutput = malloc(sizeof(LPWSTR) * settings.cbHash + 1);
        for(unsigned int j = 0; j < settings.cbHash; j++)
            wsprintfW(&lpwOutput[2*j], L"%02X", settings.pbHash[j]);
        lpwOutput[settings.cbHash] = '\0';

        wprintf(L"%-15s %-20s %s\n", BCRYPT_SHA256_ALGORITHM, (WCHAR *)argv[i], lpwOutput);
        // wprintf(L"%s\n", lpwOutput);

        cleanup(&settings);
    }

    return 0;
}

NTSTATUS initializeCryptoAPI(SETTINGS *_settings)
{
    //open an algorithm handle
    if((_settings->status = BCryptOpenAlgorithmProvider(&_settings->hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0)))
    {
        fwprintf(stderr, L"* Error: open algorythm provider failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    //calculate the size of the buffer to hold the hash object
    if((_settings->status = BCryptGetProperty(_settings->hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&_settings->cbHashObject, sizeof(DWORD), &_settings->cbData, 0)))
    {
        fwprintf(stderr, L"* Error: getting crypto properties failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    //allocate the hash object on the heap
    // pbHashObject = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbHashObject);
    _settings->pbHashObject = malloc(_settings->cbHashObject);
    if(!_settings->pbHashObject)
    {
        _settings->status = STATUS_UNSUCCESSFUL;
        fwprintf(stderr, L"* Error: allocating memory for hash object failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

   //calculate the length of the hash
    if((_settings->status = BCryptGetProperty(_settings->hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&_settings->cbHash, sizeof(DWORD), &_settings->cbData, 0)))
    {
        fwprintf(stderr, L"* Error: getting crypto properties failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    //allocate the hash buffer on the heap
    // pbHash = (PBYTE)HeapAlloc (GetProcessHeap (), 0, cbHash);
    _settings->pbHash = malloc(sizeof(PBYTE) *_settings->cbHash);
    if(!_settings->pbHash)
    {
        _settings->status = STATUS_UNSUCCESSFUL;
        fwprintf(stderr, L"* Error: allocating memory for hash failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    //create a hash
    if((_settings->status = BCryptCreateHash(_settings->hAlg, &_settings->hHash, _settings->pbHashObject, _settings->cbHashObject, NULL, 0, 0)))
    {
        fwprintf(stderr, L"* Error: creating hash failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    return 0x00000000L;
}

void cleanup(SETTINGS *_settings)
{
    if(_settings->hAlg)
    {
        BCryptCloseAlgorithmProvider(_settings->hAlg,0);
    }

    if (_settings->hHash)    
    {
        BCryptDestroyHash(_settings->hHash);
    }

    if(_settings->pbHashObject)
    {
        free(_settings->pbHashObject);
    }

    if(_settings->pbHash)
    {
        free(_settings->pbHash);
    }
}