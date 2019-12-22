/*
 * counter
 * 
 * Version: 0.1.0
 * Date: 2019-12-21
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
*/

#define _CRT_SECURE_NO_WARNINGS
#define USE_LIBCMT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CLINES 0
#define CWORDS 1

short parseArgs(int, char **, char **);
size_t count(const char *, short);
void usage(void);

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        usage();
        exit(0);
    }

    size_t ret;
    char *fileName;

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

    printf("%zd\n", ret);
    
    return 0;
}

short parseArgs(int argc, char **argv, char **fileName)
{
    int switchCount = 0;
    short retCode = 1;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '/')
        {
            if (_stricmp(argv[i], "/?") == 0)
            {
                retCode = 0;
                break;
            }
            else if (_stricmp(argv[i], "/w") == 0)
            {
                ++switchCount;
                retCode = 2;
            }
            else
            {
                printf("Unkown parameter '%s'\n", argv[i]);
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

size_t count(const char *fileName, short mode)
{
    FILE *pFile;
    pFile = fopen(fileName, "r");
    if (!pFile)
    {
        perror("* ERROR");
        exit(1);
    }

    size_t counter = 0;
    char delimeter[] = " \t\n";
    char *tok;

    char buffer[BUFSIZ];
    while (fgets(buffer, BUFSIZ, pFile))
    {
        if (mode == CWORDS)
        {
            tok = strtok(buffer, delimeter);

            while (tok)
            {
                ++counter;
                tok = strtok(NULL, delimeter);
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
    printf("counter - Counts Lines or Words\n");
    printf("\n");
    printf("Usage:\n");
    printf("\tcounter.exe [/W] <filename>\n");
    printf("\n");
    printf("Parameter:\n");
    printf("\t/W            = counts words instead of lines\n");
    printf("\t<filename>    = path/name of the file to read\n");
}