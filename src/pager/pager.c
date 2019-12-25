/*
 * -----------------------------------------------------------------------
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
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "pdcurses.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <math.h>

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
char **screenBuff = NULL;
int allocCount = 1;
int sbCount = 0;
FILE *pFile = NULL;
WINDOW *term = NULL;
short isStdinRedirected = FALSE;
unsigned short showLineNum = FALSE;
int digitCount = 1;
char *fileName = NULL;
/* END (global vars) */

void scrollUp(const int);
void scrollDown(const int);
void gotoStartEnd(const int);
void updateStatusLine(void);
void countDigits(int);
void showHelp(void);
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
        else if (argc == 3 && _stricmp(argv[2], "/n") == 0)
        {
            showLineNum = TRUE;
        }

        char *tmp = basename(argv[1]);
        if (tmp) fileName = tmp;
        else
        {
            size_t len = sizeof(chtype) * strlen(argv[1]);
            fileName = malloc(len + 1);
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
        size_t len = sizeof(chtype) * strlen(name);
        fileName = malloc(len + 1);
        if (!fileName)
        {
            perror("* ERROR");
            return(EXIT_FAILURE);
        }
        snprintf(fileName, len, "%s", name);
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

        // storing the linenumber within the string
        // sneaky, sneaky...
        snprintf(screenBuff[sbCount++], BUFSIZ, "%5d: %s", sbCount, inBuf);

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
    start_color();
    raw();
    keypad(term, TRUE);
    scrollok(term, TRUE);
    noecho();
    nonl();
    curs_set(0);

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_MAGENTA);

    countDigits(sbCount);

    for (; lineCount <= (LINES - 2); ++lineCount)
    {
        wscrl(term, 1);
        mvwaddnstr(term, LINES-2, 0, screenBuff[lineCount] + (showLineNum ? (5-digitCount) : 7), COLS);
    }

    updateStatusLine();
    wrefresh(term);

    // main loop
    int isRunning = TRUE;
    while (isRunning)
    {
        switch (getch())
        {
            case VK_ESCAPE: case 'q': isRunning = FALSE; break;
            case KEY_ENTER: case VK_RETURN: case KEY_DOWN: case VK_DOWN: case 'j':
                scrollDown(1); break;
            case KEY_UP: case VK_UP: case 'k':
                scrollUp(1); break;
            case KEY_NPAGE: case VK_NEXT: case ' ':
                scrollDown(LINES); break;
            case KEY_PPAGE: case VK_PRIOR: case 'b':
                scrollUp(LINES); break;
            case KEY_HOME: case VK_HOME: case 449: case 'g':
                gotoStartEnd(TOP); break;
            case KEY_END: case VK_END: case 455: case 'G':
                gotoStartEnd(BOTTOM); break;
            case 'v':
                system(fileName); isRunning = FALSE; break;
            case '?':
                showHelp(); break;
            // for now, just exit on resize
            case KEY_RESIZE: isRunning = FALSE; break;
            default: break;
        }

        wrefresh(term);
    }

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
            mvwaddnstr(term, 0, 0, screenBuff[lineCount--] + (showLineNum ? (5-digitCount) : 7), COLS);
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
            mvwaddnstr(term, LINES-2, 0, screenBuff[lineCount++] + (showLineNum ? (5-digitCount) : 7), COLS);
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
        mvwaddnstr(term, LINES-2, 0, screenBuff[lineCount++] + (showLineNum ? (5-digitCount) : 7), COLS);
    }

    updateStatusLine();
}

void updateStatusLine()
{
    wmove(term, LINES-1, 0);
    wclrtoeol(term);

    char *status;
    size_t barSize = sizeof(chtype) * COLS;

    status = malloc(barSize);
    snprintf(status, barSize-1, " %s: %d of %d (%d%%) ", fileName, lineCount, sbCount, (100 * lineCount / sbCount));
    wattron(term, COLOR_PAIR(2));
    mvwaddnstr(term, LINES-1, 0, status, COLS);
    wattron(term, COLOR_PAIR(1));
    free(status);
}

void countDigits(int number)
{
    if (number < 10)
    {
        return;
    }
    else
    {
        while ((number /= 10)) ++digitCount;
    }
    
}

void showHelp()
{
    wmove(term, LINES-1, 0);
    wclrtoeol(term);

    char *help;
    size_t barSize = sizeof(chtype) * COLS;

    help = malloc(barSize);
    snprintf(help, barSize-1, " ENTER, j: DOWN - k: LINE_UP - SPACE: PAGE_DOWN - b: PAGE_UP - g: TOP - G: END - q: EXIT ");
    wattron(term, COLOR_PAIR(3));
    mvwaddnstr(term, LINES-1, 0, help, COLS);
    wattron(term, COLOR_PAIR(1));
    free(help); 
}

void cleanup()
{
    endwin();
    delwin(term);

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
    printf("    j, ENTER, ARROW_DOWN    = scroll 1 line down\n");
    printf("    k, ARROW_UP             = scroll 1 line up\n");
    printf("    SPACE, PG_DOWN          = scroll 1 page down\n");
    printf("    b, PG_UP                = scroll 1 page up\n");
    printf("    g, HOME                 = jump to the beginning\n");
    printf("    G, END                  = jump to the end\n");
    printf("    v                       = show file in editor (exit pager)\n");
    printf("    ?                       = help\n");
    printf("    q                       = exit\n");
    printf("\n");
}