/*
 * pager - a simple terminal pager
 * 
 * Version: 0.5.0
 * Date: 2019-12-20
 * Author: Holger Dörner <holger.doerner@gmail.com>
 * 
 * This application is part of the 'TermTools'-project.
 * GitHub: https://GitHub.com/HolgerDoerner/TermTools
*/

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#define USE_LIBCMT
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "pdcurses.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifdef MOUSE_MOVED
    #undef MOUSE_MOVED
#endif // MOUSE_MOVED

#ifndef PDC_WIDE
    #define PDC_WIDE
#endif // PDC_WIDE

#ifndef PDC_FORCE_UTF8
    #define PDC_FORCE_UTF8
#endif // PDC_FORCE_UTF8

#include <curses.h>

#include "termtools.h"

#define TOP 0
#define BOTTOM 1
#define TRUE 1
#define FALSE 0

/* global vars */
int lineCount = 0;
char inBuf[BUFSIZ];
char **screenBuff;
int allocCount = 1;
int sbCount = 0;
FILE *pFile;
WINDOW *term;
short isStdinRedirected = FALSE;
char *fileName;
/* END (global vars) */

void scrollUp(const int);
void scrollDown(const int);
void gotoStartEnd(const int);
void updateStatusLine(void);
void cleanup(void);
void version(void);
void help(void);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
    
    if (argc >= 2)
    {
        if (_stricmp(argv[1], "/?") == 0)
        {
            help();
            exit(0);
        }
        else if (_stricmp(argv[1], "/v") == 0)
        {
            version();
            exit(0);
        }

        char *tmp = basename(argv[1]);
        if (tmp) fileName = tmp;
        else
        {
            size_t len = strlen(argv[1]) + 1;
            fileName = malloc(len);
            if (!fileName)
            {
                perror("* ERROR");
                return(EXIT_FAILURE);
            }
            snprintf(fileName, len, "%s", argv[1]);
        }

        pFile = fopen(argv[1], "r");
        if (pFile == NULL)
        {
            perror("* ERROR");
        }
    }
    else
    {
        char name[] = "pipe";
        fileName = malloc(strlen(name));
        if (!fileName)
        {
            perror("* ERROR");
            return(EXIT_FAILURE);
        }
        snprintf(fileName, strlen(name)+1, "%s", name);
        pFile = stdin;
        isStdinRedirected = TRUE;
    }

    screenBuff = (char **)malloc(sizeof(char *) * (BUFSIZ * allocCount));

    while (fgets(inBuf, BUFSIZ, pFile))
    {
        screenBuff[sbCount] = (char *)malloc(sizeof(char) * BUFSIZ);
        if (screenBuff[sbCount] == NULL)
        {
            perror("* ERROR");
            exit(1);
        }

        snprintf(screenBuff[sbCount++], BUFSIZ, "%s", inBuf);


        if (feof(pFile)) break;

        if ((sbCount > 0) && ((sbCount % BUFSIZ) == 0))
        {
            screenBuff = (char **)realloc(screenBuff, sizeof(char *) * (BUFSIZ * ++allocCount));

            if (screenBuff == NULL)
            {
                perror("* ERROR");
                exit(EXIT_FAILURE);
            }
        }
    }

    if (!isStdinRedirected && fclose(pFile))
    {
        perror("* ERROR");
        exit(EXIT_FAILURE);
    }
    else
    {
        freopen("CON", "r", stdin);
    }
    
    term = initscr();
    curs_set(0);
    raw();
    keypad(term, TRUE);
    scrollok(term, TRUE);
    noecho();
    nonl();

    for (; lineCount <= (LINES - 2); ++lineCount)
    {
        wscrl(term, 1);
        mvwaddstr(term, LINES-2, 0, screenBuff[lineCount]);
    }

    updateStatusLine();
    // reDrawScreen(); // need fix

    // main loop
    int isRunning = TRUE;
    while (isRunning)
    {
        switch (getch())
        {
            case VK_ESCAPE: case 'q': isRunning = FALSE; break;
            case KEY_DOWN: case VK_DOWN: case KEY_C2: case 'j':
                scrollDown(1); break;
            case KEY_UP: case VK_UP: case KEY_A2: case 'k':
                scrollUp(1); break;
            case KEY_NPAGE: case VK_NEXT: case 457: case 'h':
                scrollDown(LINES); break;
            case KEY_PPAGE: case VK_PRIOR: case 451: case 'l':
                scrollUp(LINES); break;
            case KEY_HOME: case VK_HOME: case 449: case 'b':
                gotoStartEnd(0); break;
            case KEY_END: case VK_END: case 455: case 'e':
                gotoStartEnd(1); break;
            // for now, just exit on resize
            case KEY_RESIZE: isRunning = FALSE; break;
            default: break;
        }
    }

    endwin();
    cleanup();
    return 0;
}

void scrollUp(const int range)
{
    lineCount -= LINES;

    for (int i = 0; i < range; ++i)
    {
        if (lineCount >= 0)
        {
            wscrl(term, -1);
            mvwaddstr(term, 0, 0, screenBuff[lineCount--]);
            wrefresh(term);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }

    lineCount += LINES;

    updateStatusLine();
}

void scrollDown(const int range)
{
    for (int i = 0; i < range; ++i)
    {
        if (lineCount < sbCount)
        {
            wscrl(term, 1);
            mvwaddstr(term, LINES-2, 0, screenBuff[lineCount++]);
            wrefresh(term);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }

    updateStatusLine();
}

void gotoStartEnd(const int direction)
{
    if ( ((lineCount == LINES-1) & (direction == TOP))
        || ((lineCount == sbCount) & (direction == BOTTOM)) )
    {
        beep();
        flash();
        return;
    }

    lineCount = direction ? (sbCount - LINES) : 0;
    for (int i = 0; i < (direction ? LINES : LINES-1); ++i)
    {
        wscrl(term, 1);
        mvwaddstr(term, LINES-2, 0, screenBuff[lineCount++]);
        wrefresh(term);
    }

    updateStatusLine();
}

void updateStatusLine()
{
    wmove(term, LINES-1, 0);
    wclrtoeol(term);

    char *status;
    size_t barSize = sizeof(chtype) * getmaxx(term);

    status = malloc(barSize);
    snprintf(status, barSize-1, "--- %s: %d of %d ---", fileName, lineCount, sbCount);
    mvwaddstr(term, LINES-1, 0, status);
    free(status);
}

void cleanup()
{
    for (int i = 0; i < (BUFSIZ * allocCount); ++i)
    {
        free(screenBuff[i]);
        screenBuff[i] = NULL;
    }

    free(screenBuff);
    free(fileName);
    screenBuff = NULL;
    fileName = NULL;
}

void version()
{
    printf("pager.exe - Version: 0.5.0 - Date: 2019-12-20\n");
}

void help()
{
    printf("Usage:\n");
    printf("    pager.exe <filename>\n");
    printf("        or\n");
    printf("    type <filename> | pager.exe\n");
    printf("\n");
    printf("Controls:\n");
    printf("    ARROW_UP, K    = scroll 1 line up\n");
    printf("    ARROW_DOWN, J  = scroll 1 line down\n");
    printf("    PG_UP, l       = scroll 1 page up\n");
    printf("    PG_DOWN, h     = scroll 1 page down\n");
    printf("    HOME           = jump to the beginning\n");
    printf("    END            = jump to the end\n");
    printf("    ESC, q         = exit\n");
    printf("\n");
}