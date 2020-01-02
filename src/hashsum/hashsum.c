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
 * hashsum - calculates and checks hash digests
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#include "hashsum_version.h"
#include "termtools.h"

#include <bcrypt.h>

#pragma comment(lib, "bcrypt.lib")

#define STATUS_SUCCESSFUL 0x00000000L
#define STATUS_UNSUCCESSFUL 0xC0000001L
#define MODE_NORMAL 0
#define MODE_CHECK 1

typedef struct SETTINGS {
    BCRYPT_ALG_HANDLE hAlg;
    BCRYPT_HASH_HANDLE hHash;
    LPCWSTR pszAlgId;
    DWORD cbData;
    DWORD cbHash;
    DWORD cbHashObject;
    PBYTE pbHashObject;
    PBYTE pbHash;
    NTSTATUS status;
    short mode;
} SETTINGS;

int parseArgs(SETTINGS *, LPWSTR **, size_t *, size_t, LPWSTR *);
NTSTATUS initializeCryptoAPI(SETTINGS *);
void checkHashValues(SETTINGS *, LPWSTR *, size_t);
size_t readHashFile(LPWSTR **, LPWSTR **, LPWSTR);
LPWSTR *calculateFilehashBatch(SETTINGS *, LPWSTR *, size_t);
LPWSTR calculateFilehash(SETTINGS *, LPWSTR);
LPCWSTR getHashType(LPWSTR);
void cleanupCryptoAPI(SETTINGS *);
void printHelp(void);

int wmain(int argc, LPWSTR *argv)
{
    setlocale(LC_ALL, "");

    SETTINGS settings = {
        .hAlg = NULL,
        .hHash = NULL,
        .pszAlgId = BCRYPT_SHA256_ALGORITHM,
        .cbData = 0,
        .cbHash = 0,
        .cbHashObject = 0,
        .pbHashObject = NULL,
        .pbHash = NULL,
        .status = STATUS_SUCCESSFUL,
        .mode = MODE_NORMAL
    };

    size_t cbArgs = 0;
    LPWSTR *pbArgs = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * argc-1);
    if (parseArgs(&settings, &pbArgs, &cbArgs, argc, argv))
        return EXIT_FAILURE;

    if (initializeCryptoAPI(&settings) != STATUS_SUCCESSFUL)
    {
        HeapFree(GetProcessHeap(), 0, pbArgs);
        cleanupCryptoAPI(&settings);
        exit(EXIT_FAILURE);
    }

    switch (settings.mode)
    {
        case MODE_CHECK:
            checkHashValues(&settings, pbArgs, cbArgs);
            break;
        default:
            calculateFilehashBatch(&settings, pbArgs, cbArgs);
            break;
    }
    
    HeapFree(GetProcessHeap(), 0, pbArgs);
    cleanupCryptoAPI(&settings);

    return 0;
}

int parseArgs(SETTINGS *_settings, LPWSTR **_filenames, size_t *_length, size_t _argc, LPWSTR *_argv)
{
    if (!_settings || !_filenames || !_argv)
    {
        fwprintf_s(stderr, L"* ERROR: parseArgs(): one or more parameters are invalid\n");
        return 1;
    }
    else if (_argc == 1)
    {
        printHelp();
        exit(EXIT_SUCCESS);
    }

    *_length = 0;

    for (int i = 1; i < _argc; ++i)
    {
        if ((wchar_t)_argv[i][0] == L'/')
        {
            if (_wcsicmp((const wchar_t *)_argv[i], L"/?") == 0)
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/C") == 0)
                (*_settings).mode = MODE_CHECK;
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/SHA1") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA1_ALGORITHM;
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/SHA256") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA256_ALGORITHM;
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/SHA384") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA384_ALGORITHM;
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/SHA512") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA512_ALGORITHM;
            else if (_wcsicmp((const wchar_t *)_argv[i], L"/MD5") == 0)
                (*_settings).pszAlgId = BCRYPT_MD5_ALGORITHM;
            else
            {
                _fwprintf_p(stderr, L"* ERROR: Unknown parameter: %s\n", (const wchar_t *)_argv[i]);
                printHelp();
                return 1;
            }
        }
        else
        {
            (*_filenames)[(*_length)++] = _argv[i];
        }
    }

    return 0;
}

NTSTATUS initializeCryptoAPI(SETTINGS *_settings)
{
    if(((*_settings).status = BCryptOpenAlgorithmProvider(&_settings->hAlg, _settings->pszAlgId, NULL, BCRYPT_HASH_REUSABLE_FLAG)))
    {
        fwprintf(stderr, L"* Error: open algorythm provider failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    // calculate size of buffer for the hash
    if(((*_settings).status = BCryptGetProperty(_settings->hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&_settings->cbHashObject, sizeof(DWORD), &_settings->cbData, 0)))
    {
        fwprintf(stderr, L"* Error: getting crypto properties failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    (*_settings).pbHashObject = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(BYTE) * _settings->cbHashObject);
    if(!_settings->pbHashObject)
    {
        (*_settings).status = STATUS_UNSUCCESSFUL;
        fwprintf(stderr, L"* Error: allocating memory for hash object failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

   // calculate length of hash
    if(((*_settings).status = BCryptGetProperty(_settings->hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&_settings->cbHash, sizeof(DWORD), &_settings->cbData, 0)))
    {
        fwprintf(stderr, L"* Error: getting crypto properties failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    (*_settings).pbHash = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PBYTE) * _settings->cbHash);
    if(!_settings->pbHash)
    {
        (*_settings).status = STATUS_UNSUCCESSFUL;
        fwprintf(stderr, L"* Error: allocating memory for hash failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    if(((*_settings).status = BCryptCreateHash(_settings->hAlg, &_settings->hHash, _settings->pbHashObject, _settings->cbHashObject, NULL, 0, 0)))
    {
        fwprintf(stderr, L"* Error: creating hash failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    return STATUS_SUCCESSFUL;
}

void checkHashValues(SETTINGS *_settings, LPWSTR *_pbHashFiles, size_t _cbHashFiles)
{
    for (size_t i = 0; i < _cbHashFiles; ++i)
    {
        LPWSTR *pbFileNames, *pbFileOutput;

        size_t cPairs = readHashFile(&pbFileNames, &pbFileOutput, _pbHashFiles[i]);
        if (cPairs <= 0 || !pbFileNames || !pbFileOutput) continue;

        for (size_t j = 0; j < cPairs; ++j)
        {
            LPCWSTR newPszLastAlgId = getHashType(pbFileOutput[j]);
            if (!newPszLastAlgId) 
            {
                wprintf_s(L"* WARNING: %s: Unknown Hash-Algorithem\n", pbFileNames[j]);
                continue;
            }
            else if (_wcsicmp(_settings->pszAlgId, newPszLastAlgId) != 0)
            {
                (*_settings).pszAlgId = newPszLastAlgId;

                if (initializeCryptoAPI(_settings) != STATUS_SUCCESSFUL)
                {
                    fwprintf_s(stderr, L"* ERROR: Re-Initialisation of Crypto-API failed\n");
                    cleanupCryptoAPI(_settings);
                    exit(EXIT_FAILURE);
                }
            }

            LPWSTR lpwCalculatedOutput = calculateFilehash(_settings, pbFileNames[j]);
            LPCWSTR cmpResult = _wcsicmp(lpwCalculatedOutput, pbFileOutput[j]) ? L"FAILED" : L"OK";

            wprintf_s(L"%s: %s\n", pbFileNames[j], cmpResult);

            HeapFree(GetProcessHeap(), 0, pbFileNames[j]);
            HeapFree(GetProcessHeap(), 0, pbFileOutput[j]);
            HeapFree(GetProcessHeap(), 0, lpwCalculatedOutput);
        }

        HeapFree(GetProcessHeap(), 0, pbFileNames);
        HeapFree(GetProcessHeap(), 0, pbFileOutput);
    }
}

size_t readHashFile(LPWSTR **_pbFileNames, LPWSTR **_pbFileOutput, LPWSTR _fileName)
{
    size_t counter = 0;

    if (! ((*_pbFileNames) = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * 20)) ||
        ! ((*_pbFileOutput) = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * 20)))
    {
        // we have to guard the calls to HeapFree to make sure we don't call it with NULL
        if ((*_pbFileNames)) HeapFree(GetProcessHeap(), 0, (*_pbFileNames));
        if ((*_pbFileOutput)) HeapFree(GetProcessHeap(), 0, (*_pbFileOutput));

        (*_pbFileNames) = NULL;
        (*_pbFileOutput) = NULL;

        return counter;
    }
    
    FILE *pHashFile;
    if (_wfopen_s(&pHashFile, _fileName, L"r"))
    {
        WCHAR errMsg[BUFSIZ];
        _wcserror_s((WCHAR *)&errMsg, BUFSIZ, errno);
        _fwprintf_p(stderr, L"* %s: %s\n", _fileName, errMsg);

        HeapFree(GetProcessHeap(), 0, (*_pbFileNames));
        HeapFree(GetProcessHeap(), 0, (*_pbFileOutput));
        (*_pbFileNames) = NULL;
        (*_pbFileOutput) = NULL;
        
        return counter;
    }

    wchar_t inBuff[BUFSIZ*2];
    while (fgetws(inBuff, (BUFSIZ*2)-1, pHashFile))
    {
        if (inBuff[0] == '#') continue; // TODO: better handling

        wchar_t *tmp;

        wchar_t *hash = wcstok_s(inBuff, L" \t", &tmp);
        wchar_t *file = wcstok_s(NULL, L" \t", &tmp);
        file[wcslen(file)-1] = '\0';

        size_t cFile = wcslen(file) + 1;
        size_t cHash = wcslen(hash) + 1;
        (*_pbFileNames)[counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * cFile); 
        (*_pbFileOutput)[counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * cHash);
        swprintf_s((*_pbFileNames)[counter], cFile, L"%s", file);
        swprintf_s((*_pbFileOutput)[counter], cHash, L"%s", hash);

        if (feof(pHashFile)) break;
        ++counter;
    }

    fclose(pHashFile);

    return counter;
}

LPWSTR *calculateFilehashBatch(SETTINGS *_settings, LPWSTR *_fileNames, size_t _cbArgs)
{
    LPWSTR *pbOutput = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * _cbArgs);
    if (!pbOutput) return pbOutput;

    for (int i = 0; i < _cbArgs; ++i)
        pbOutput[i] = calculateFilehash(_settings, _fileNames[i]);

    return pbOutput;
}

LPWSTR calculateFilehash(SETTINGS *_settings, LPWSTR _fileName)
{
    LPWSTR lpwOutput = NULL;

    FILE *pFile;
    if (_wfopen_s(&pFile, (const WCHAR *)_fileName, L"rb"))
    {
        WCHAR errMsg[BUFSIZ];
        _wcserror_s((WCHAR *)&errMsg, BUFSIZ, errno);
        _fwprintf_p(stderr, L"* %s: %s\n", _fileName, errMsg);
        return lpwOutput;
    }

    PBYTE buff = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * BUFSIZ);
    if (!buff)return lpwOutput;

    while (fread_s(buff, BUFSIZ, sizeof(BYTE), BUFSIZ, pFile))
    {
        // hash the data
        if(((*_settings).status = BCryptHashData(_settings->hHash, buff, BUFSIZ, 0)))
        {
            fwprintf(stderr, L"* ERROR: hashing %s failed with code: %#x\n", _fileName, _settings->status);
            return lpwOutput;
        }

        if (feof(pFile)) break;
    }
    
    // close the hash
    if(((*_settings).status = BCryptFinishHash(_settings->hHash, _settings->pbHash, _settings->cbHash, 0)))
    {
        fwprintf(stderr, L"* ERROR: finishing hash failed with code: %#x\n", _settings->status);
        return lpwOutput;
    }

    lpwOutput = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * _settings->cbHash+1);
    if (!lpwOutput) return lpwOutput;

    byteToHexStrW(_settings->pbHash, lpwOutput, _settings->cbHash * 2);

    if (_settings->mode == MODE_NORMAL)
        wprintf(L"%s %s\n", lpwOutput, _fileName);

    HeapFree(GetProcessHeap(), 0, buff);

    return lpwOutput;
}

LPCWSTR getHashType(LPWSTR _hash)
{
    switch (wcslen(_hash))
    {
        case 32: return BCRYPT_MD5_ALGORITHM;
        case 40: return BCRYPT_SHA1_ALGORITHM;
        case 64: return BCRYPT_SHA256_ALGORITHM;
        case 96: return BCRYPT_SHA384_ALGORITHM;
        case 128: return BCRYPT_SHA512_ALGORITHM;
        default: return NULL;
    }
}

void cleanupCryptoAPI(SETTINGS *_settings)
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
        HeapFree(GetProcessHeap(), 0, _settings->pbHashObject);
    }

    if(_settings->pbHash)
    {
        HeapFree(GetProcessHeap(), 0, _settings->pbHash);
    }
}

void printHelp()
{
    wprintf(L"hashsum.exe v%hs\n", HASHSUM_VERSION);
    wprintf(L"\n");
    wprintf(L"Usage:\n");
    wprintf(L"\thashsum.exe <file> [files ...]\n");
}