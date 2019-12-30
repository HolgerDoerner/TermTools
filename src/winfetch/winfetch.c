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
 * winfetch - displays system information on the terminal.
 * 
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
 */


#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "winfetch_version.h"
#include "termtools.h"

// #pragma comment(lib, "user32.lib")

#define BUFFERSIZE 70
#define STRINGLENTH 128

typedef struct SYSINFO {
    int term_rows;
	char *OS_Caption;
    char *OS_LastBootUpTime;
	char *OS_OSArchitecture;
	char *OS_ServicePackMajorVersion;
	char *OS_ServicePackMinorVersion;
	char *OS_Version;
	char *OS_WindowsDirectory;
	char *OS_MUILanguages;
    char *BASEBOARD_Manufacturer;
	char *BASEBOARD_Product;
    char *BIOS_Manufacturer;
	char *BIOS_Name;
    char *COMPUTERSYSTEM_Name;
	char *COMPUTERSYSTEM_NumberOfProcessors;
	char *COMPUTERSYSTEM_SystemType;
	char *COMPUTERSYSTEM_TotalPhysicalMemory;
	char *COMPUTERSYSTEM_UserName;
    char *CPU_Manufacturer;
	char *CPU_MaxClockSpeed;
	char *CPU_Name;
	char *CPU_NumberOfCores;
	char *CPU_NumberOfEnabledCore;
	char *CPU_SocketDesignation;
    char *TIMEZONE_Caption;
    char *GPU_AdapterRAM;
    char *GPU_DriverVersion;
    char *GPU_Name;
    char *GPU_VideoModeDescription;
} SYSINFO;

void parseArgs(int, char**);
void printHelp(void);
void printOutput(SYSINFO *);
int getOSInfo(SYSINFO *);
int getBASEBOARDInfo(SYSINFO *);
int getBIOSInfo(SYSINFO *);
int getCOMPUTERSYSTEMInfo(SYSINFO *);
int getCPUInfo(SYSINFO *);
int getTIMEZONEInfo(SYSINFO *);
int getGPUInfo(SYSINFO *);
char *calculateUptime(SYSINFO *);

int main(int argc, char **argv)
{
    SYSINFO sysinfo = {
        .term_rows = 0,
        .OS_Caption = "\n",
        .OS_LastBootUpTime = "\n",
        .OS_OSArchitecture = "\n",
        .OS_ServicePackMajorVersion = "\n",
        .OS_ServicePackMinorVersion = "\n",
        .OS_Version = "\n",
        .OS_WindowsDirectory = "\n",
        .OS_MUILanguages = "\n",
        .BASEBOARD_Manufacturer = "\n",
        .BASEBOARD_Product = "\n",
        .BIOS_Manufacturer = "\n",
        .BIOS_Name = "\n",
        .COMPUTERSYSTEM_Name = "\n",
        .COMPUTERSYSTEM_NumberOfProcessors = "\n",
        .COMPUTERSYSTEM_SystemType = "\n",
        .COMPUTERSYSTEM_TotalPhysicalMemory = "\n",
        .COMPUTERSYSTEM_UserName = "\n",
        .CPU_Manufacturer = "\n",
        .CPU_MaxClockSpeed = "\n",
        .CPU_Name = "\n",
        .CPU_NumberOfCores = "\n",
        .CPU_NumberOfEnabledCore = "\n",
        .CPU_SocketDesignation = "\n",
        .TIMEZONE_Caption = "\n",
        .GPU_AdapterRAM = "\n",
        .GPU_DriverVersion = "\n",
        .GPU_Name = "\n",
        .GPU_VideoModeDescription = "\n"
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
        fprintf(stderr, "* WARNING: cound not get handle for terminal!\n");
    }

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
    {
        // return GetLastError();
        fprintf(stderr, "* WARNING: cound not get current terminal mode!\n");
    }

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode))
    {
        // return GetLastError();
        fprintf(stderr, "* WARNING: cound not set VT mode!\n");
    }

    if (getOSInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting OS info\n");

    if (getBASEBOARDInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting BASEBOARD info\n");

    if (getBIOSInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting BIOS info\n");

    if (getCOMPUTERSYSTEMInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting COMPUTERSYSTEM info\n");

    if (getCPUInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting CPU info\n");

    if (getTIMEZONEInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting TIMEZONE info\n");

    if (getGPUInfo(&sysinfo) == EXIT_FAILURE)
        fprintf(stderr, "* ERROR: getting GPU info\n");


	printOutput(&sysinfo);

    return 0;
}

/*
 * parses the commandline-arguments.
 * 
 * _IN:
 *      _argc: the number of arguments
 *      _argv: the vector of arguments
 */
void parseArgs(int _argc, char **_argv)
{
    if (_argc > 1)
    {
        if (_argv[1][0] == '/')
        {
            if (_stricmp(_argv[1], "/?") == 0)
            {
                printHelp();
                exit(EXIT_SUCCESS);
            }
            else
            {
                fprintf(stderr, "* ERROR: Unknown argument: %s\n", _argv[1]);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            fprintf(stderr, "* ERROR: Unknown argument: %s\n", _argv[1]);
            exit(EXIT_FAILURE);
        }
    }
}

/*
 * shows help-message and version.
 */
void printHelp()
{
    printf("winfetch v%s\n", WINFETCH_VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("\twinfetch.exe [/?]\n");
    printf("\n");
    printf("Arguments:\n");
    printf("\t/?    - Print help\n");
    printf("\n");
}

/*
 * writes the final output to the terminal.
 * 
 * _IN:
 *      _sysinfo: stores the gathered informations
 */
void printOutput(SYSINFO *_sysinfo)
{
    printf("\n");
	printf("\x1b[36m                                 111\x1b[0m %-10s  \x1b[1\x1b[96m%s\x1b[0m", "", _sysinfo->COMPUTERSYSTEM_UserName);
	printf("\x1b[36m                     111111111111111\x1b[0m %-10s  %s", "", "------------------------------\n");
    printf("\x1b[36m       11111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", "OS", _sysinfo->OS_Caption);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->OS_Version);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->OS_OSArchitecture);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->OS_MUILanguages);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->TIMEZONE_Caption);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", "Mainboard", _sysinfo->BASEBOARD_Manufacturer);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->BASEBOARD_Product);
	printf("                                     \x1b[33m%10s:\x1b[0m %s", "BIOS", _sysinfo->BIOS_Manufacturer);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %-10s  %s", "", _sysinfo->BIOS_Name);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %.2Lf GB\n", "RAM", byteToGiga(strtoull(_sysinfo->COMPUTERSYSTEM_TotalPhysicalMemory, (char **)NULL, 10)));
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", "CPU", _sysinfo->CPU_Name);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %10s  %s", "", _sysinfo->CPU_SocketDesignation);
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m %10s  %d x %.2f Ghz\n", "", (int)strtol(_sysinfo->CPU_NumberOfCores, (char **)NULL, 10), mhzToGhz(strtol(_sysinfo->CPU_MaxClockSpeed, (char **)NULL, 10)));
	printf("\x1b[36m 11111111111111  1111111111111111111\x1b[0m \x1b[33m%10s:\x1b[0m %s", "GPU", _sysinfo->GPU_Name);
	printf("\x1b[36m       11111111  1111111111111111111\x1b[0m %10s  %s", "", _sysinfo->GPU_DriverVersion);
	printf("\x1b[36m                     111111111111111\x1b[0m %10s  %.2Lf GB\n", "", byteToGiga(strtoull(_sysinfo->GPU_AdapterRAM, (char **)NULL, 10)));
	printf("\x1b[36m                                 111\x1b[0m %10s  %s", "", _sysinfo->GPU_VideoModeDescription);
	printf("                                     \x1b[33m%10s:\x1b[0m %s\n", "Uptime", calculateUptime(_sysinfo));
    printf("\n");
	printf(" \x1b[7m TERMTOOLS: %s - WINFETCH: %s \x1b[0m\n", TT_VERSION, WINFETCH_VERSION);

    // keep printing empty lines to fill exactly the terminal window
    for (int i = 26; i < _sysinfo->term_rows; ++i)
        printf("\n");
}

/*
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
    pStdout = _popen("wmic os get Version, Caption, OSArchitecture,"
                            "ServicepackMajorVersion, ServicepackMinorVersion,"
                            "WindowsDirectory, MUILanguages, LastBootUpTime /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[10][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).OS_Caption = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_Caption, strtok(NULL, "="));

    (*_sysinfo).OS_LastBootUpTime = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_LastBootUpTime, strtok(NULL, "="));

    (*_sysinfo).OS_MUILanguages = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_MUILanguages, strtok(NULL, "="));
    
    (*_sysinfo).OS_OSArchitecture = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_OSArchitecture, strtok(NULL, "="));
    
    (*_sysinfo).OS_ServicePackMajorVersion = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_ServicePackMajorVersion, strtok(NULL, "="));
    
    (*_sysinfo).OS_ServicePackMinorVersion = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_ServicePackMinorVersion, strtok(NULL, "="));
    
    (*_sysinfo).OS_Version = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_Version, strtok(NULL, "="));
    
    (*_sysinfo).OS_WindowsDirectory = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_WindowsDirectory, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic BASEBOARD get Manufacturer, Product /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[2][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).BASEBOARD_Manufacturer = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).BASEBOARD_Manufacturer, strtok(NULL, "="));

    (*_sysinfo).BASEBOARD_Product = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).BASEBOARD_Product, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic BIOS get Manufacturer, Name /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[2][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).BIOS_Manufacturer = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).BIOS_Manufacturer, strtok(NULL, "="));

    (*_sysinfo).BIOS_Name = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).BIOS_Name, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic COMPUTERSYSTEM get Name, NumberOfProcessors,"
                    "SystemType, TotalPhysicalMemory, UserName /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[6][STRINGLENTH];
    int i = 0;


    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).COMPUTERSYSTEM_Name = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).COMPUTERSYSTEM_Name, strtok(NULL, "="));

    (*_sysinfo).COMPUTERSYSTEM_NumberOfProcessors = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).COMPUTERSYSTEM_NumberOfProcessors, strtok(NULL, "="));

    (*_sysinfo).COMPUTERSYSTEM_SystemType = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).COMPUTERSYSTEM_SystemType, strtok(NULL, "="));

    (*_sysinfo).COMPUTERSYSTEM_TotalPhysicalMemory = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).COMPUTERSYSTEM_TotalPhysicalMemory, strtok(NULL, "="));

    (*_sysinfo).COMPUTERSYSTEM_UserName = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).COMPUTERSYSTEM_UserName, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic CPU get Manufacturer, MaxClockSpeed,"
                    "Name, NumberOfCores, NumberOfEnabledCore,"
                    "SocketDesignation /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[7][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).CPU_Manufacturer = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_Manufacturer, strtok(NULL, "="));

    (*_sysinfo).CPU_MaxClockSpeed = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_MaxClockSpeed, strtok(NULL, "="));

    (*_sysinfo).CPU_Name = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_Name, strtok(NULL, "="));

    (*_sysinfo).CPU_NumberOfCores = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_NumberOfCores, strtok(NULL, "="));

    (*_sysinfo).CPU_NumberOfEnabledCore = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_NumberOfEnabledCore, strtok(NULL, "="));

    (*_sysinfo).CPU_SocketDesignation = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).CPU_SocketDesignation, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic TIMEZONE get Caption /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[1][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).TIMEZONE_Caption = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).TIMEZONE_Caption, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
    pStdout = _popen("wmic path win32_VideoController get AdapterRAM,"
                    "DriverVersion, Name, VideoModeDescription /value", "rt");

    if (!pStdout) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[4][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdout))
    {
        if (isStringBlank(buff, STRINGLENTH)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdout)) break;
    }

    fclose(pStdout);

    i = 0;

    (*_sysinfo).GPU_AdapterRAM = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).GPU_AdapterRAM, strtok(NULL, "="));

    (*_sysinfo).GPU_DriverVersion = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).GPU_DriverVersion, strtok(NULL, "="));

    (*_sysinfo).GPU_Name = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).GPU_Name, strtok(NULL, "="));

    (*_sysinfo).GPU_VideoModeDescription = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).GPU_VideoModeDescription, strtok(NULL, "="));

    return EXIT_SUCCESS;
}

/*
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
char *calculateUptime(SYSINFO *_sysinfo)
{
    char tmp[5];

    struct tm tm_boot = {0};
    tm_boot.tm_isdst = -1;

    subString(_sysinfo->OS_LastBootUpTime, tmp, 0, 4);
    tm_boot.tm_year = (int)strtol(tmp, (char **)NULL, 10) - 1900;

    subString(_sysinfo->OS_LastBootUpTime, tmp, 4, 2);
    tm_boot.tm_mon = (int)strtol(tmp, (char **)NULL, 10) - 1;

    subString(_sysinfo->OS_LastBootUpTime, tmp, 6, 2);
    tm_boot.tm_mday = (int)strtol(tmp, (char **)NULL, 10);

    subString(_sysinfo->OS_LastBootUpTime, tmp, 8, 2);
    tm_boot.tm_hour = (int)strtol(tmp, (char **)NULL, 10);

    subString(_sysinfo->OS_LastBootUpTime, tmp, 10, 2);
    tm_boot.tm_min = (int)strtol(tmp, (char **)NULL, 10);

    subString(_sysinfo->OS_LastBootUpTime, tmp, 12, 2);
    tm_boot.tm_sec = (int)strtol(tmp, (char **)NULL, 10);


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

    char *output = malloc(sizeof(char) * BUFSIZ);
    if (!output) return NULL;

    snprintf(output, BUFSIZ, "%d:%d:%d:%d:%d:%d", years, months, days, hours, minutes, seconds);

    return output;
}