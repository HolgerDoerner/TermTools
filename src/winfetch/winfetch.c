/**-----------------------------------------------------------------------
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
 * winfetch - displays system information on the terminal.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */

#include "winfetch_version.h"
#include "termtools.h"

#include <time.h>

#define BUFFERSIZE 128
#define STRINGLENTH 256
typedef struct SYSINFO {
	LPWSTR OS_Caption;
    LPWSTR OS_LastBootUpTime;
	LPWSTR OS_OSArchitecture;
	LPWSTR OS_Version;
    LPWSTR BASEBOARD_Manufacturer;
	LPWSTR BASEBOARD_Product;
    LPWSTR BIOS_Manufacturer;
	LPWSTR BIOS_Name;
    LPWSTR COMPUTERSYSTEM_Name;
	LPWSTR COMPUTERSYSTEM_NumberOfProcessors;
	LPWSTR COMPUTERSYSTEM_SystemType;
	LPWSTR COMPUTERSYSTEM_TotalPhysicalMemory;
	LPWSTR COMPUTERSYSTEM_UserName;
    LPWSTR CPU_Manufacturer;
	LPWSTR CPU_MaxClockSpeed;
	LPWSTR CPU_Name;
	LPWSTR CPU_NumberOfCores;
	LPWSTR CPU_SocketDesignation;
    LPWSTR TIMEZONE_Caption;
    LPWSTR GPU_AdapterRAM;
    LPWSTR GPU_DriverVersion;
    LPWSTR GPU_Name;
    LPWSTR GPU_VideoModeDescription;
    int term_rows;
} SYSINFO;

void parseArgs(int, LPWSTR *);
void printHelp(void);
void printOutput(SYSINFO *);
int getOSInfo(SYSINFO *);
int getBASEBOARDInfo(SYSINFO *);
int getBIOSInfo(SYSINFO *);
int getCOMPUTERSYSTEMInfo(SYSINFO *);
int getCPUInfo(SYSINFO *);
int getTIMEZONEInfo(SYSINFO *);
int getGPUInfo(SYSINFO *);
LPWSTR calculateUptime(SYSINFO *);
void cleanup(SYSINFO *);

int wmain(int argc, LPWSTR *argv)
{
    setUnicodeLocale();

    SYSINFO sysinfo = {
        .term_rows = 0,
        .OS_Caption = L"\n",
        .OS_LastBootUpTime = L"\n",
        .OS_OSArchitecture = L"\n",
        .OS_Version = L"\n",
        .BASEBOARD_Manufacturer = L"\n",
        .BASEBOARD_Product = L"\n",
        .BIOS_Manufacturer = L"\n",
        .BIOS_Name = L"\n",
        .COMPUTERSYSTEM_Name = L"\n",
        .COMPUTERSYSTEM_NumberOfProcessors = L"\n",
        .COMPUTERSYSTEM_SystemType = L"\n",
        .COMPUTERSYSTEM_TotalPhysicalMemory = L"\n",
        .COMPUTERSYSTEM_UserName = L"\n",
        .CPU_Manufacturer = L"\n",
        .CPU_MaxClockSpeed = L"\n",
        .CPU_Name = L"\n",
        .CPU_NumberOfCores = L"\n",
        .CPU_SocketDesignation = L"\n",
        .TIMEZONE_Caption = L"\n",
        .GPU_AdapterRAM = L"\n",
        .GPU_DriverVersion = L"\n",
        .GPU_Name = L"\n",
        .GPU_VideoModeDescription = L"\n"
    };

    parseArgs(argc, argv);

    // Set output mode to handle virtual terminal sequences
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hOut, &csbi);
    sysinfo.term_rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;

    if (hOut == INVALID_HANDLE_VALUE)
    {
        // return GetLastError();
        fwprintf(stderr, L"* WARNING: cound not get handle for terminal!\n");
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        // return GetLastError();
        fwprintf(stderr, L"* WARNING: cound not get current terminal mode!\n");
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        // return GetLastError();
        fwprintf(stderr, L"* WARNING: cound not set VT mode!\n");
    }

    if (getOSInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting OS info\n");

    if (getBASEBOARDInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting BASEBOARD info\n");

    if (getBIOSInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting BIOS info\n");

    if (getCOMPUTERSYSTEMInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting COMPUTERSYSTEM info\n");

    if (getCPUInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting CPU info\n");

    if (getTIMEZONEInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting TIMEZONE info\n");

    if (getGPUInfo(&sysinfo) == EXIT_FAILURE)
        fwprintf(stderr, L"* ERROR: getting GPU info\n");


	printOutput(&sysinfo);
	cleanup(&sysinfo);

    return 0;
}

/**
 * clean-up heap-memory allocated for the values in the SYSINFO-object.
 * 
 * _IN:
 *      _sysinfo: the object storing the gathered system-information
 */
void cleanup(SYSINFO *_sysinfo)
{
    // we check for the default-value of '\n'. if it is NOT present
    // means the variable is pointing to a location on the heap
    // and the space there has to be freed
    if (_sysinfo->OS_Caption[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).OS_Caption);
    if (_sysinfo->OS_LastBootUpTime[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).OS_LastBootUpTime);
	if (_sysinfo->OS_OSArchitecture[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).OS_OSArchitecture);
	if (_sysinfo->OS_Version[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).OS_Version);
    if (_sysinfo->BASEBOARD_Manufacturer[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).BASEBOARD_Manufacturer);
	if (_sysinfo->BASEBOARD_Product[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).BASEBOARD_Product);
    if (_sysinfo->BIOS_Manufacturer[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).BIOS_Manufacturer);
	if (_sysinfo->BIOS_Name[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).BIOS_Name);
    if (_sysinfo->COMPUTERSYSTEM_Name[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).COMPUTERSYSTEM_Name);
	if (_sysinfo->COMPUTERSYSTEM_NumberOfProcessors[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).COMPUTERSYSTEM_NumberOfProcessors);
	if (_sysinfo->COMPUTERSYSTEM_SystemType[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).COMPUTERSYSTEM_SystemType);
	if (_sysinfo->COMPUTERSYSTEM_TotalPhysicalMemory[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).COMPUTERSYSTEM_TotalPhysicalMemory);
	if (_sysinfo->COMPUTERSYSTEM_UserName[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).COMPUTERSYSTEM_UserName);
    if (_sysinfo->CPU_Manufacturer[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).CPU_Manufacturer);
	if (_sysinfo->CPU_MaxClockSpeed[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).CPU_MaxClockSpeed);
	if (_sysinfo->CPU_Name[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).CPU_Name);
	if (_sysinfo->CPU_NumberOfCores[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).CPU_NumberOfCores);
	if (_sysinfo->CPU_SocketDesignation[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).CPU_SocketDesignation);
    if (_sysinfo->TIMEZONE_Caption[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).TIMEZONE_Caption);
    if (_sysinfo->GPU_AdapterRAM[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).GPU_AdapterRAM);
    if (_sysinfo->GPU_DriverVersion[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).GPU_DriverVersion);
    if (_sysinfo->GPU_Name[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).GPU_Name);
    if (_sysinfo->GPU_VideoModeDescription[0] != L'\n')
        HeapFree(GetProcessHeap(), 0, (*_sysinfo).GPU_VideoModeDescription);
}

/**
 * parses the commandline-arguments.
 * 
 * _IN:
 *      _argc: the number of arguments
 *      _argv: the vector of arguments
 */
void parseArgs(int _argc, LPWSTR *_argv)
{
    if (_argc > 1)
    {
        if (_argv[1][0] == L'/')
        {
            if (_wcsicmp(_argv[1], L"/?") == 0)
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else
            {
                fwprintf(stderr, L"* ERROR: Unknown argument: %s\n", _argv[1]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fwprintf(stderr, L"* ERROR: Unknown argument: %s\n", _argv[1]);
            exit(EXIT_FAILURE);
        }
    }
}

/**
 * shows help-message and version.
 */
void printHelp()
{
    wprintf_s(L"winfetch v%hs\n", WINFETCH_VERSION);
    wprintf_s(L"\n");
    wprintf_s(L"Usage:\n");
    wprintf_s(L"\twinfetch.exe [/?]\n");
    wprintf_s(L"\n");
    wprintf_s(L"Arguments:\n");
    wprintf_s(L"\t/?    - Print help\n");
    wprintf_s(L"\n");
}

/**
 * writes the final output to the terminal.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 */
void printOutput(SYSINFO *_sysinfo)
{
    wprintf_s(L"\n");
	wprintf_s(L"\x1b[36m                                 111\x1b[0m %-10s  \x1b[1\x1b[96m%s\x1b[0m", L"", _sysinfo->COMPUTERSYSTEM_UserName);
	wprintf_s(L"\x1b[36m                     111111111111111\x1b[0m %-10s  %s", L"", L"------------------------------\n");
    wprintf_s(L"\x1b[36m       11111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", L"OS", _sysinfo->OS_Caption);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", L"", _sysinfo->OS_Version);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", L"", _sysinfo->OS_OSArchitecture);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s\n", L"", _wsetlocale(LC_ALL, NULL));
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", L"", _sysinfo->TIMEZONE_Caption);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", L"Mainboard", _sysinfo->BASEBOARD_Manufacturer);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", L"", _sysinfo->BASEBOARD_Product);
	wprintf_s(L"                                     \x1b[33m%10s:\x1b[0m %s", L"BIOS", _sysinfo->BIOS_Manufacturer);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", L"", _sysinfo->BIOS_Name);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %.2Lf GB\n", L"RAM", byteToGiga(wcstoull(_sysinfo->COMPUTERSYSTEM_TotalPhysicalMemory, (LPWSTR *)NULL, 10)));
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", L"CPU", _sysinfo->CPU_Name);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %10s  %s", L"", _sysinfo->CPU_SocketDesignation);
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m %10s  %d x %.2f Ghz\n", L"", (int)wcstol(_sysinfo->CPU_NumberOfCores, (LPWSTR *)NULL, 10), mhzToGhz(wcstol(_sysinfo->CPU_MaxClockSpeed, (LPWSTR *)NULL, 10)));
	wprintf_s(L"\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", L"GPU", _sysinfo->GPU_Name);
	wprintf_s(L"\x1b[36m       11111111  1111111111111111111\x1b[0m %10s  %s", L"", _sysinfo->GPU_DriverVersion);
	wprintf_s(L"\x1b[36m                     111111111111111\x1b[0m %10s  %.2Lf GB\n", L"", byteToGiga(wcstoull(_sysinfo->GPU_AdapterRAM, (LPWSTR *)NULL, 10)));
	wprintf_s(L"\x1b[36m                                 111\x1b[0m %10s  %s", L"", _sysinfo->GPU_VideoModeDescription);
	wprintf_s(L"                                     \x1b[33m%10s:\x1b[0m %s\n", L"Uptime", calculateUptime(_sysinfo));
    wprintf_s(L"\n");

    // keep printing empty lines to fill exactly the terminal window
    for (int i = 25; i < _sysinfo->term_rows; ++i)
        wprintf(L"\n");
}

/**
 * gathers information about the Operating System.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getOSInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;

    FILE *pStdout;
    pStdout = _wpopen(L"wmic os get Version, Caption, OSArchitecture,"
                        "LastBootUpTime /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[4][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).OS_Caption = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).OS_Caption, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).OS_LastBootUpTime = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).OS_LastBootUpTime, STRINGLENTH, wcstok_s(NULL, L"=", &context));
    
    (*_sysinfo).OS_OSArchitecture = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).OS_OSArchitecture, STRINGLENTH, wcstok_s(NULL, L"=", &context));
    
    (*_sysinfo).OS_Version = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).OS_Version, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers information about the Baseboard/Motherboard.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getBASEBOARDInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic BASEBOARD get Manufacturer, Product /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[2][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).BASEBOARD_Manufacturer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).BASEBOARD_Manufacturer, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).BASEBOARD_Product = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).BASEBOARD_Product, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers information about the BIOS.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getBIOSInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic BIOS get Manufacturer, Name /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[2][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).BIOS_Manufacturer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=" ,&context);
    wcscpy_s((*_sysinfo).BIOS_Manufacturer, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).BIOS_Name = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).BIOS_Name, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers general information about the System.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getCOMPUTERSYSTEMInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic COMPUTERSYSTEM get Name, NumberOfProcessors,"
                    "SystemType, TotalPhysicalMemory, UserName /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[6][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).COMPUTERSYSTEM_Name = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).COMPUTERSYSTEM_Name, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).COMPUTERSYSTEM_NumberOfProcessors = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).COMPUTERSYSTEM_NumberOfProcessors, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).COMPUTERSYSTEM_SystemType = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).COMPUTERSYSTEM_SystemType, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).COMPUTERSYSTEM_TotalPhysicalMemory = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).COMPUTERSYSTEM_TotalPhysicalMemory, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).COMPUTERSYSTEM_UserName = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).COMPUTERSYSTEM_UserName, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers information about the CPU.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getCPUInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic CPU get Manufacturer, MaxClockSpeed,"
                    "Name, NumberOfCores, SocketDesignation /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[7][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).CPU_Manufacturer = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).CPU_Manufacturer, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).CPU_MaxClockSpeed = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).CPU_MaxClockSpeed, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).CPU_Name = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).CPU_Name, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).CPU_NumberOfCores = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).CPU_NumberOfCores, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).CPU_SocketDesignation = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).CPU_SocketDesignation, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers information about the Timezone-Setting.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getTIMEZONEInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic TIMEZONE get Caption /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[1][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).TIMEZONE_Caption = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).TIMEZONE_Caption, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * gathers information about the Graphics-Card/GPU.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: 0 on success, 1 on error
 */
int getGPUInfo(SYSINFO *_sysinfo)
{
    if (!_sysinfo) return EXIT_FAILURE;
    
    FILE *pStdout;
    pStdout = _wpopen(L"wmic path win32_VideoController get AdapterRAM,"
                    "DriverVersion, Name, VideoModeDescription /value", L"rt");

    if (!pStdout) return EXIT_FAILURE;

    WCHAR buff[STRINGLENTH];
    WCHAR values[4][STRINGLENTH];
    LPWSTR context;
    int i = 0;

    while (fgetws(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlankW(buff, STRINGLENTH)) continue;
        _snwprintf_s(values[i++], STRINGLENTH, STRINGLENTH-1, L"%s", buff);
        if (feof(pStdout)) break;
    }

    _pclose(pStdout);

    i = 0;

    (*_sysinfo).GPU_AdapterRAM = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).GPU_AdapterRAM, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).GPU_DriverVersion = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).GPU_DriverVersion, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).GPU_Name = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).GPU_Name, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    (*_sysinfo).GPU_VideoModeDescription = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(WCHAR) * STRINGLENTH);
    wcstok_s(values[i++], L"=", &context);
    wcscpy_s((*_sysinfo).GPU_VideoModeDescription, STRINGLENTH, wcstok_s(NULL, L"=", &context));

    return EXIT_SUCCESS;
}

/**
 * calculatethe uptime since last reboot.
 * 
 * IMPORTANT: LastBootUpTime may NOT be the time the machine was last
 * powered on! This is due to the fact that Windows 10 uses a mechanism
 * known as 'FastBoot' which suspends to harddrive rather than shutting
 * down completely to speed up booting. This does not affect RE-BOOTING,
 * the system, which always triggers a FULL shutdown and reboot.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 * 
 * _RETURNS: a pointer to the created string on success, NULL on error
 */
LPWSTR calculateUptime(SYSINFO *_sysinfo)
{
    WCHAR tmp[5];

    struct tm tm_boot = { 0 };
    tm_boot.tm_isdst = -1;

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 0, 4);
    tm_boot.tm_year = (int)wcstol(tmp, (LPWSTR *)NULL, 10) - 1900;

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 4, 2);
    tm_boot.tm_mon = (int)wcstol(tmp, (LPWSTR *)NULL, 10) - 1;

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 6, 2);
    tm_boot.tm_mday = (int)wcstol(tmp, (LPWSTR *)NULL, 10);

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 8, 2);
    tm_boot.tm_hour = (int)wcstol(tmp, (LPWSTR *)NULL, 10);

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 10, 2);
    tm_boot.tm_min = (int)wcstol(tmp, (LPWSTR *)NULL, 10);

    subStringW(_sysinfo->OS_LastBootUpTime, tmp, 12, 2);
    tm_boot.tm_sec = (int)wcstol(tmp, (LPWSTR *)NULL, 10);


    time_t boot_t = _mktime64(&tm_boot);
    time_t curr_t = time(NULL);
    unsigned long long diff_t = (unsigned long)difftime(curr_t, boot_t);

    int years   = (int)(diff_t / 378432000);
    diff_t %= 378432000;
    int months  = (int)(diff_t / 31536000);
    diff_t %= 31536000;
    int days    = (int)(diff_t / 86400);
    diff_t %= 86400;
    int hours   = (int)(diff_t / 3600);
    diff_t %= 3600;
    int minutes = (int)(diff_t / 60);
    diff_t %= 60;
    int seconds = (int)diff_t;

    LPWSTR output = malloc(sizeof(WCHAR) * BUFSIZ);
    if (!output) return NULL;

    _snwprintf_s(output, BUFSIZ, BUFSIZ-1, L"%d:%d:%d:%d:%d:%d", years, months, days, hours, minutes, seconds);

    return output;
}