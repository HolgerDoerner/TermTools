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

#include "termtools.h"

// #pragma comment(lib, "user32.lib")

#define BUFFERSIZE 70
#define STRINGLENTH 128

typedef struct SYSINFO {
	char *OS_Caption;
    char *OS_NumberOfUsers;
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

int getOSInfo(SYSINFO *);
int getBASEBOARDInfo(SYSINFO *);
int getBIOSInfo(SYSINFO *);
int getCOMPUTERSYSTEMInfo(SYSINFO *);
int getCPUInfo(SYSINFO *);
int getTIMEZONEInfo(SYSINFO *);
int getGPUInfo(SYSINFO *);

int main(int argc, char **argv)
{
    SYSINFO sysinfo;

    if (getOSInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting OS info\n");
        exit(EXIT_FAILURE);
    }

    if (getBASEBOARDInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting BASEBOARD info\n");
        exit(EXIT_FAILURE);
    }

    if (getBIOSInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting BIOS info\n");
        exit(EXIT_FAILURE);
    }

    if (getCOMPUTERSYSTEMInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting COMPUTERSYSTEM info\n");
        exit(EXIT_FAILURE);
    }

    if (getCPUInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting CPU info\n");
        exit(EXIT_FAILURE);
    }

    if (getTIMEZONEInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting TIMEZONE info\n");
        exit(EXIT_FAILURE);
    }

    if (getGPUInfo(&sysinfo) == EXIT_FAILURE)
    {
        fprintf(stderr, "* ERROR: getting GPU info\n");
        exit(EXIT_FAILURE);
    }
    
	printf("                                111 %-10s  %s", "", sysinfo.COMPUTERSYSTEM_UserName);
	printf("                    111111111111111 %-10s  %s", "", "------------------------------\n");
    printf("      11111111  1111111111111111111 %10s: %s", "OS", sysinfo.OS_Caption);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.OS_Version);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.OS_OSArchitecture);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.OS_MUILanguages);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.TIMEZONE_Caption);
	printf("11111111111111  1111111111111111111 %10s: %s", "Mainboard", sysinfo.BASEBOARD_Manufacturer);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.BASEBOARD_Product);
	printf("                                    %10s: %s", "BIOS", sysinfo.BIOS_Manufacturer);
	printf("11111111111111  1111111111111111111 %-10s  %s", "", sysinfo.BIOS_Name);
	printf("11111111111111  1111111111111111111 %10s: %s", "RAM", sysinfo.COMPUTERSYSTEM_TotalPhysicalMemory);
	printf("11111111111111  1111111111111111111 %10s: %s", "CPU", sysinfo.CPU_Name);
	printf("11111111111111  1111111111111111111 %10s  %s", "", sysinfo.CPU_SocketDesignation);
	printf("11111111111111  1111111111111111111 %10s  # %s", "", sysinfo.CPU_NumberOfCores);
	printf("11111111111111  1111111111111111111 %10s: %s", "GPU", sysinfo.GPU_Name);
	printf("      11111111  1111111111111111111 %10s  %s", "", sysinfo.GPU_DriverVersion);
	printf("                    111111111111111 %10s  %s", "", sysinfo.GPU_AdapterRAM);
	printf("                                111 %10s  %s", "", sysinfo.GPU_VideoModeDescription);

	// printf("%-40s: %s", "OS_ServicePackMajorVersion", sysinfo.OS_ServicePackMajorVersion);
	// printf("%-40s: %s", "OS_ServicePackMinorVersion", sysinfo.OS_ServicePackMinorVersion);
	// printf("%-40s: %s", "OS_WindowsDirectory", sysinfo.OS_WindowsDirectory);
	// printf("%-40s: %s", "OS_NumberOfUsers", sysinfo.OS_NumberOfUsers);
	// printf("%-40s: %s", "COMPUTERSYSTEM_Name", sysinfo.COMPUTERSYSTEM_Name);
	// printf("%-40s: %s", "COMPUTERSYSTEM_NumberOfProcessors", sysinfo.COMPUTERSYSTEM_NumberOfProcessors);
	// printf("%-40s: %s", "COMPUTERSYSTEM_SystemType", sysinfo.COMPUTERSYSTEM_SystemType);
	// printf("%-40s: %s", "CPU_Manufacturer", sysinfo.CPU_Manufacturer);
	// printf("%-40s: %s", "CPU_MaxClockSpeed", sysinfo.CPU_MaxClockSpeed);
	// printf("%-40s: %s", "CPU_NumberOfEnabledCore", sysinfo.CPU_NumberOfEnabledCore);
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
    FILE *pStdin;
    pStdin = _popen("wmic os get Version, Caption, OSArchitecture, NumberOfUsers,"
                            "ServicepackMajorVersion, ServicepackMinorVersion,"
                            "WindowsDirectory, MUILanguages /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[8][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

    i = 0;

    (*_sysinfo).OS_Caption = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_Caption, strtok(NULL, "="));

    (*_sysinfo).OS_MUILanguages = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_MUILanguages, strtok(NULL, "="));

    (*_sysinfo).OS_NumberOfUsers = malloc(sizeof(char) * STRINGLENTH);
    strtok(values[i++], "=");
    strcpy((*_sysinfo).OS_NumberOfUsers, strtok(NULL, "="));
    
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
    FILE *pStdin;
    pStdin = _popen("wmic BASEBOARD get Manufacturer, Product /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[2][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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
    FILE *pStdin;
    pStdin = _popen("wmic BIOS get Manufacturer, Name /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[2][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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
    FILE *pStdin;
    pStdin = _popen("wmic COMPUTERSYSTEM get Name, NumberOfProcessors,"
                    "SystemType, TotalPhysicalMemory, UserName /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[6][STRINGLENTH];
    int i = 0;


    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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
    FILE *pStdin;
    pStdin = _popen("wmic CPU get Manufacturer, MaxClockSpeed,"
                    "Name, NumberOfCores, NumberOfEnabledCore,"
                    "SocketDesignation /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[7][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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
    FILE *pStdin;
    pStdin = _popen("wmic TIMEZONE get Caption /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[1][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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
    FILE *pStdin;
    pStdin = _popen("wmic path win32_VideoController get AdapterRAM,"
                    "DriverVersion, Name, VideoModeDescription /value", "rt");

    if (!pStdin) return EXIT_FAILURE;

    char buff[STRINGLENTH];
    char values[4][STRINGLENTH];
    int i = 0;

    while (fgets(buff, STRINGLENTH-1, pStdin))
    {
        if (isLineBlank(buff)) continue;
        snprintf(values[i++], STRINGLENTH-1, "%s", buff);
        if (feof(pStdin)) break;
    }

    fclose(pStdin);

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