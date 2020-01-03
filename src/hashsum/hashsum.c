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
 * hashsum - calculates and checks pbHash digests of files
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

int parseArgs(SETTINGS *, LPWSTR **, SIZE_T *, SIZE_T, LPWSTR *);
NTSTATUS initializeCryptoAPI(SETTINGS *);
void checkHashValues(SETTINGS *, LPWSTR *, SIZE_T);
SIZE_T readHashFile(LPWSTR **, LPWSTR **, LPWSTR);
LPWSTR *calculateFilehashBatch(SETTINGS *, LPWSTR *, SIZE_T);
LPWSTR calculateFileHash(SETTINGS *, LPWSTR);
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

    SIZE_T cbArgs = 0;
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
        // TODO: error handling
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

/*
 * parses the command-line arguments.
 * 
 * _IN:
 *      _argc: the total number of arguments passed on command-line
 *      _argv: the original argument-vector recieved from command-line
 * 
 * _IN_OUT:
 *      _settings: an SETTINGS-object
 * 
 * _OUT:
 *      _pvFileNames: an vector large enough to contain the pbFile names passed
 *                  on command-line
 *      _cvFileNames: the length of the _pvFileNames
 * 
 * _RETURNS: 0 on success, >0 on error
 */
int parseArgs(SETTINGS *_settings, LPWSTR **_pvFileNames, SIZE_T *_cvFileNames, SIZE_T _argc, LPWSTR *_argv)
{
    if (!_settings || !_pvFileNames || !_argv)
    {
        fwprintf_s(stderr, L"* ERROR: parseArgs(): one or more parameters are invalid\n");
        return 1;
    }
    else if (_argc == 1)
    {
        printHelp();
        exit(EXIT_SUCCESS);
    }

    (*_cvFileNames) = 0;

    for (int i = 1; i < _argc; ++i)
    {
        if ((WCHAR)_argv[i][0] == L'/')
        {
            if (_wcsicmp((LPCWSTR)_argv[i], L"/?") == 0)
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/C") == 0)
                (*_settings).mode = MODE_CHECK;
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/SHA1") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA1_ALGORITHM;
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/SHA256") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA256_ALGORITHM;
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/SHA384") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA384_ALGORITHM;
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/SHA512") == 0)
                (*_settings).pszAlgId = BCRYPT_SHA512_ALGORITHM;
            else if (_wcsicmp((LPCWSTR)_argv[i], L"/MD5") == 0)
                (*_settings).pszAlgId = BCRYPT_MD5_ALGORITHM;
            else
            {
                _fwprintf_p(stderr, L"* ERROR: Unknown parameter: %s\n", _argv[i]);
                printHelp();
                return 1;
            }
        }
        else
        {
            (*_pvFileNames)[(*_cvFileNames)++] = _argv[i];
        }
    }

    return 0;
}

/*
 * initializes the Windows Crypto API (CNG).
 * 
 * _IN_OUT:
 *      _settings: an SETTINGS-object
 * 
 * _RETURNS: 0x00000000 on success, errorcode on failure (NT Error-Codes)
 */
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
        fwprintf(stderr, L"* Error: creating pbHash failed with status: 0x%x\n", _settings->status);
        return _settings->status;
    }

    return STATUS_SUCCESSFUL;
}

/*
 * validates the hash-digests of file by comparing them to fresh calculated ones.
 * this function can handle multiple hash-files and processes them in the same order
 * as given on the command-line.
 * it tries to guess which algorithem is used by calculating the length of the given
 * hash and skips files whose algorithen could not be determined with a warning.
 * ---------------------------------------------------------------------------------
 * 
 * _IN:
 *      _pvHashFiles: a vector containing the names/paths of the hash-files
 *      _cvHashFiles: the size of _pvHashFiles
 * 
 * _IN_OUT:
 *      _settings: the application SETTINGS-object
 */
void checkHashValues(SETTINGS *_settings, LPWSTR *_pvHashFiles, SIZE_T _cvHashFiles)
{
    for (SIZE_T i = 0; i < _cvHashFiles; ++i)
    {
        LPWSTR *pbFileNames, *pbFileOutput;

        SIZE_T cPairs = readHashFile(&pbFileNames, &pbFileOutput, _pvHashFiles[i]);
        if (cPairs <= 0 || !pbFileNames || !pbFileOutput) continue;

        for (SIZE_T j = 0; j < cPairs; ++j)
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

            LPWSTR lpwCalculatedOutput = calculateFileHash(_settings, pbFileNames[j]);
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

/*
 * parses a hash-files for hash-filename-pairs.
 * 
 * skips linecomments starting with '#'.
 * --------------------------------------------
 * 
 * _IN:
 *      _fileName: the name/path of the hash-pbFile
 * 
 * _IN_OUT:
 *      _pvFileNames: the parsed filenames
 *      _pvFileHashes: the parsed hash-strings
 * 
 * _RETURNS: the size of the _pvFile*-vectors
 */
SIZE_T readHashFile(LPWSTR **_pvFileNames, LPWSTR **_pvFileHashes, LPWSTR _fileName)
{
    int allocCount = 2;
    SIZE_T counter = 0;

    if (! ((*_pvFileNames) = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR))) ||
        ! ((*_pvFileHashes) = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR))))
    {
        // we have to guard the calls to HeapFree() to make sure we don't call it with NULL
        if ((*_pvFileNames)) HeapFree(GetProcessHeap(), 0, (*_pvFileNames));
        if ((*_pvFileHashes)) HeapFree(GetProcessHeap(), 0, (*_pvFileHashes));

        (*_pvFileNames) = NULL;
        (*_pvFileHashes) = NULL;

        return counter;
    }
    
    FILE *pHashFile;
    if (_wfopen_s(&pHashFile, _fileName, L"r"))
    {
        WCHAR errMsg[BUFSIZ];
        _wcserror_s((WCHAR *)&errMsg, BUFSIZ, errno);
        _fwprintf_p(stderr, L"* %s: %s\n", _fileName, errMsg);

        HeapFree(GetProcessHeap(), 0, (*_pvFileNames));
        HeapFree(GetProcessHeap(), 0, (*_pvFileHashes));
        (*_pvFileNames) = NULL;
        (*_pvFileHashes) = NULL;
        
        return counter;
    }

    WCHAR inBuff[BUFSIZ*2];
    while (fgetws(inBuff, (BUFSIZ*2)-1, pHashFile))
    {
        if (inBuff[0] == '#') continue; // TODO: better handling
        else if (isStringBlankW(inBuff, BUFSIZ*2)) continue;

        LPWSTR pbContext;
        LPWSTR pbHash;
        LPWSTR pbFile;

        if ((pbHash = wcstok_s(inBuff, L" \t", &pbContext)) &&
            (pbFile = wcstok_s(NULL, L" \t", &pbContext)))
        {
            pbFile[wcsnlen(pbFile, BUFSIZ)-1] = '\0';
        }
        else
        {
            wprintf_s(L"* WARNING: Maleformatted Line: %zd\n", counter+1);
            continue;
        }

        SIZE_T cFile = wcsnlen(pbFile, BUFSIZ) + 1;
        SIZE_T cHash = wcsnlen(pbHash, 128) + 1;
        (*_pvFileNames)[counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * cFile); 
        (*_pvFileHashes)[counter] = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * cHash);
        swprintf_s((*_pvFileNames)[counter], cFile, L"%s", pbFile);
        swprintf_s((*_pvFileHashes)[counter], cHash, L"%s", pbHash);

        if (feof(pHashFile)) break;

        (*_pvFileNames) = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (*_pvFileNames), sizeof(LPWSTR) * allocCount);
        (*_pvFileHashes) = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (*_pvFileHashes), sizeof(LPWSTR) * allocCount);

        ++allocCount;
        ++counter;
    }

    fclose(pHashFile);

    return counter;
}

/*
 * calculates hash-digests for files in a batch.
 * ---------------------------------------------
 * TODO: error checking/handling
 * ---------------------------------------------
 * 
 * _IN:
 *      _settings: the application SETTINGS-object
 *      _pvFileNames: a vector containing the names/paths of the files to hash
 *      _cvFileNames: the size of the _pvFileNames vector
 * 
 * _RETURNS: a vector containing the calculated hashes, order matches the
 *          _pvFileNames-vector
 */
LPWSTR *calculateFilehashBatch(SETTINGS *_settings, LPWSTR *_pvFileNames, SIZE_T _cvFileNames)
{
    LPWSTR *pbOutput;
    if (! (pbOutput = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * _cvFileNames)))
        return pbOutput;

    for (int i = 0; i < _cvFileNames; ++i)
        pbOutput[i] = calculateFileHash(_settings, _pvFileNames[i]);

    return pbOutput;
}

/*
 * calculate the hash-digest of a single file.
 * -------------------------------------------
 * 
 * _IN:
 *      _fileName: the name/path of the file to hash
 * 
 * _ON_OUT:
 *      _settings: the application SETTINGS-object
 * 
 * _RETURNS: a string containing the calculated digest
 */
LPWSTR calculateFileHash(SETTINGS *_settings, LPWSTR _fileName)
{
    LPWSTR lpwOutput = NULL;

    FILE *pFile;
    if (_wfopen_s(&pFile, (LPCWSTR)_fileName, L"rb"))
    {
        WCHAR errMsg[BUFSIZ];
        _wcserror_s((WCHAR *)&errMsg, BUFSIZ, errno);
        _fwprintf_p(stderr, L"* %s: %s\n", _fileName, errMsg);
        return lpwOutput;
    }

    PBYTE buff;
    if (! (buff = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * BUFSIZ)))
        return lpwOutput;

    while (fread_s(buff, BUFSIZ, sizeof(BYTE), BUFSIZ, pFile))
    {
        // pbHash the data
        if(((*_settings).status = BCryptHashData(_settings->hHash, buff, BUFSIZ, 0)))
        {
            fwprintf(stderr, L"* ERROR: hashing %s failed with code: %#x\n", _fileName, _settings->status);
            return lpwOutput;
        }

        if (feof(pFile)) break;
    }
    
    // close the pbHash
    if(((*_settings).status = BCryptFinishHash(_settings->hHash, _settings->pbHash, _settings->cbHash, 0)))
    {
        fwprintf(stderr, L"* ERROR: finishing pbHash failed with code: %#x\n", _settings->status);
        return lpwOutput;
    }

    if (! (lpwOutput = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(LPWSTR) * _settings->cbHash+1)))
        return lpwOutput;

    if (byteToHexStrW(_settings->pbHash, lpwOutput, _settings->cbHash * 2))
    {
        HeapFree(GetProcessHeap(), 0, lpwOutput);
        return (lpwOutput = NULL);
    }

    if (_settings->mode == MODE_NORMAL)
        wprintf(L"%s  %s\n", lpwOutput, _fileName);

    HeapFree(GetProcessHeap(), 0, buff);

    return lpwOutput;
}

/*
 * determines the type of the hash-algorithem for a given hHash-string.
 * -------------------------------------------------------------------
 * 
 * _IN:
 *      _hash: a string containing a hash
 * 
 * _RETURNS: a constant string containing the name of the algorithem,
 *          or NULL if algorithem could not be determined
 */
LPCWSTR getHashType(LPWSTR _hash)
{
    switch (wcsnlen(_hash, 128))
    {
        case 32: return BCRYPT_MD5_ALGORITHM;
        case 40: return BCRYPT_SHA1_ALGORITHM;
        case 64: return BCRYPT_SHA256_ALGORITHM;
        case 96: return BCRYPT_SHA384_ALGORITHM;
        case 128: return BCRYPT_SHA512_ALGORITHM;
        default: return NULL;
    }
}


/*
 * cleans up objects and heap-space used by the Crypto-API.
 * --------------------------------------------------------
 * 
 * _IN:
 *      _settings: the application SETTINGS
 */
void cleanupCryptoAPI(SETTINGS *_settings)
{
    if(_settings->hAlg)
        BCryptCloseAlgorithmProvider(_settings->hAlg,0);

    if (_settings->hHash)    
        BCryptDestroyHash(_settings->hHash);

    if(_settings->pbHashObject)
        HeapFree(GetProcessHeap(), 0, (*_settings).pbHashObject);

    if(_settings->pbHash)
        HeapFree(GetProcessHeap(), 0, (*_settings).pbHash);
}

/*
 * simple function printing application version and
 * usage information.
 */
void printHelp()
{
    wprintf(L"HASHSUM.EXE v%hs\n", HASHSUM_VERSION);
    wprintf(L"\n");
    wprintf(L"Usage:\n");
    wprintf(L"\tHASHSUM.EXE [/MD5 /SHA1 /SHA256 /SHA384 /SHA512] <file> [files ...]\n");
    wprintf(L"\tHASHSUM.EXE [/C] <hash-file> [hash-files ...]\n");
    wprintf(L"\n");
    wprintf(L"Options:\n");
    wprintf(L"\t/?          = shows usage info\n");
    wprintf(L"\t/C          = checks digests stored in hash-file(s)\n");
    wprintf(L"\t/MD5        = generate MD5-Digest\n");
    wprintf(L"\t/SHA1       = generate SHA1-Digest\n");
    wprintf(L"\t/SHA256     = generate SHA256-Digest (default)\n");
    wprintf(L"\t/SHA385     = generate SHA384-Digest\n");
    wprintf(L"\t/SHA512     = generate SHA512-Digest\n");
}