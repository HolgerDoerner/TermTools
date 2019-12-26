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
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "pdcurses.lib")

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <stdbool.h>

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

typedef struct SETTINGS
{
    WINDOW *term;
    bool showLineNum;
    long lineCount;
    long sbLines;
    long sbSize;
    unsigned char digitCount;
    char *fileName;
    char *filePath;
    char **screenBuff;
} SETTINGS;


void parseArgs(SETTINGS *, int, char **);
long fillScreenBuffer(SETTINGS *, FILE **);
void scrollUp(SETTINGS *, const int);
void scrollDown(SETTINGS *, const int);
void gotoStartEnd(SETTINGS *, const int);
void updateStatusLine(SETTINGS *);
void showInlineHelp(SETTINGS *);
void cleanup(SETTINGS *);
void version(void);
void help(void);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");

    SETTINGS settings = {
        .lineCount = 0,
        .term = NULL,
        .showLineNum = false,
        .digitCount = 1,
        .fileName = NULL,
        .filePath = NULL,
        .screenBuff = NULL,
        .sbLines = 0,
        .sbSize = (sizeof(char *) * BUFSIZ)
    };

    FILE *pFile = NULL;
    short isStdinRedirected = FALSE;
    
    if (argc >= 2)
    {
        parseArgs(&settings, argc, argv);

        char *tmp = basename(argv[1]);
        if (tmp) settings.fileName = tmp;
        else
        {
            size_t len = sizeof(chtype) * strlen(argv[1]);
            settings.fileName = malloc(len + 1);
            if (!settings.fileName)
            {
                perror("* ERROR");
                return(EXIT_FAILURE);
            }
            snprintf(settings.fileName, len, "%s", argv[1]);
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
        settings.fileName = malloc(len + 1);
        if (!settings.fileName)
        {
            perror("* ERROR");
            return(EXIT_FAILURE);
        }
        snprintf(settings.fileName, len, "%s", name);
        pFile = stdin;
        isStdinRedirected = TRUE;
    }

    fillScreenBuffer(&settings, &pFile);

    if (!isStdinRedirected && fclose(pFile))
    {
        perror("* ERROR");
        exit(EXIT_FAILURE);
    }
    else
    {
        freopen("CON", "r", stdin);
    }
    
    settings.term = initscr();
    start_color();
    raw();
    keypad(settings.term, TRUE);
    scrollok(settings.term, TRUE);
    noecho();
    nonl();
    curs_set(0);

    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_MAGENTA);

    settings.digitCount = countDigits(settings.sbLines);

    for (; settings.lineCount <= (LINES - 2); ++settings.lineCount)
    {
        wscrl(settings.term, 1);
        mvwaddnstr(settings.term, LINES-2, 0, settings.screenBuff[settings.lineCount] + (settings.showLineNum ? (7 - settings.digitCount) : 9), COLS);
    }

    updateStatusLine(&settings);
    wrefresh(settings.term);

    // main loop
    int isRunning = TRUE;
    while (isRunning)
    {
        switch (getch())
        {
            case VK_ESCAPE: case 'q': isRunning = FALSE; break;
            case KEY_ENTER: case VK_RETURN: case KEY_DOWN: case VK_DOWN: case 'j':
                scrollDown(&settings, 1); updateStatusLine(&settings); break;
            case KEY_UP: case VK_UP: case 'k':
                scrollUp(&settings, 1); updateStatusLine(&settings); break;
            case KEY_NPAGE: case VK_NEXT: case ' ':
                scrollDown(&settings, LINES); updateStatusLine(&settings); break;
            case KEY_PPAGE: case VK_PRIOR: case 'b':
                scrollUp(&settings, LINES); updateStatusLine(&settings); break;
            case KEY_HOME: case VK_HOME: case 449: case 'g':
                gotoStartEnd(&settings, TOP); updateStatusLine(&settings); break;
            case KEY_END: case VK_END: case 455: case 'G':
                gotoStartEnd(&settings, BOTTOM); updateStatusLine(&settings); break;
            case 'v':
                system(argv[1]); isRunning = FALSE; break;
            case '?':
                showInlineHelp(&settings); break;
            // for now, just exit on resize
            case KEY_RESIZE: isRunning = FALSE; break;
            default: break;
        }

        wrefresh(settings.term);
    }

    cleanup(&settings);
    return EXIT_SUCCESS;
}

void parseArgs(SETTINGS *settings, int argc, char **argv)
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
            settings->showLineNum = true;
        }
}

long fillScreenBuffer(SETTINGS *_settings, FILE **pFile)
{
    if (!_settings || !pFile) return -1;

    // long sbCount = 0;
    unsigned int allocCount = 1;
    char inBuf[BUFSIZ];

    (*_settings).screenBuff = malloc(_settings->sbSize);
    if (!(*_settings).screenBuff)
    {
        perror("* ERROR");
        exit(EXIT_FAILURE);
    }

    while (fgets(inBuf, BUFSIZ, *pFile))
    {
        (*_settings).screenBuff[_settings->sbLines] = malloc(sizeof(char) * BUFSIZ);
        if (!(*_settings).screenBuff[_settings->sbLines])
        {
            perror("* ERROR");
            exit(EXIT_FAILURE);
        }

        // storing the linenumber within the string
        // sneaky, sneaky...
        int ret = snprintf((*_settings).screenBuff[(*_settings).sbLines++], BUFSIZ, "%7ld: %s", _settings->sbLines, inBuf);

        if (feof(*pFile)) break;

        if ((_settings->sbLines > 0) && ((_settings->sbLines % BUFSIZ) == 0))
        {
            (*_settings).screenBuff = realloc((*_settings).screenBuff, ((*_settings).sbSize *= ++allocCount));
            if (!(*_settings).screenBuff)
            {
                perror("* ERROR");
                exit(EXIT_FAILURE);
            }
        }
    }

    return _settings->sbLines;
}

void scrollUp(SETTINGS *_settings, const int range)
{
    (*_settings).lineCount -= LINES;

    for (int i = 0; i < range; ++i)
    {
        if (_settings->lineCount >= 0)
        {
            wscrl(_settings->term, -1);
            mvwaddnstr(_settings->term, 0, 0, (*_settings).screenBuff[(*_settings).lineCount--] + (_settings->showLineNum ? (7 - _settings->digitCount) : 9), COLS);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }

    (*_settings).lineCount += LINES;
}

void scrollDown(SETTINGS *_settings, const int range)
{
    for (int i = 0; i < range; ++i)
    {
        if (_settings->lineCount < _settings->sbLines)
        {
            wscrl(_settings->term, 1);
            mvwaddnstr(_settings->term, LINES-2, 0, (*_settings).screenBuff[(*_settings).lineCount++] + (_settings->showLineNum ? (7 - _settings->digitCount) : 9), COLS);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }
}

void gotoStartEnd(SETTINGS *_settings, const int direction)
{
    if ( ((_settings->lineCount == LINES-1) & (direction == TOP))
        || ((_settings->lineCount == _settings->sbLines) & (direction == BOTTOM)) )
    {
        beep();
        flash();
        return;
    }

    (*_settings).lineCount = direction ? (_settings->sbLines - LINES) : 0;
    for (int i = 0; i < (direction ? LINES : LINES-1); ++i)
    {
        wscrl(_settings->term, 1);
        mvwaddnstr(_settings->term, LINES-2, 0, (*_settings).screenBuff[(*_settings).lineCount++] + (_settings->showLineNum ? (7 - _settings->digitCount) : 9), COLS);
    }
}

void updateStatusLine(SETTINGS *_settings)
{
    wmove(_settings->term, LINES-1, 0);
    wclrtoeol(_settings->term);

    char *status;
    size_t barSize = sizeof(chtype) * COLS;

    status = malloc(barSize);
    snprintf(status, barSize-1, " %s: %ld of %ld (%3ld%%) ", _settings->fileName, _settings->lineCount, _settings->sbLines, (100 * _settings->lineCount / _settings->sbLines));
    wattron(_settings->term, COLOR_PAIR(2));
    mvwaddnstr(_settings->term, LINES-1, 0, status, COLS);
    wattron(_settings->term, COLOR_PAIR(1));
    free(status);
}

void showInlineHelp(SETTINGS *_settings)
{
    wmove(_settings->term, LINES-1, 0);
    wclrtoeol(_settings->term);

    char *helpMessage;
    size_t barSize = sizeof(chtype) * COLS;

    helpMessage = malloc(barSize);
    snprintf(helpMessage, barSize-1, " ENTER, j: DOWN - k: LINE_UP - SPACE: PAGE_DOWN - b: PAGE_UP - g: TOP - G: END - q: EXIT ");
    wattron(_settings->term, COLOR_PAIR(3));
    mvwaddnstr(_settings->term, LINES-1, 0, helpMessage, COLS);
    wattron(_settings->term, COLOR_PAIR(1));
    free(helpMessage); 
}

void cleanup(SETTINGS *_settings)
{
    endwin();
    delwin(_settings->term);

    for (int i = 0; i < _settings->sbLines; ++i)
    {
        free((*_settings).screenBuff[i]);
        (*_settings).screenBuff[i] = NULL;
    }

    free((*_settings).screenBuff);
    free((*_settings).fileName);
    (*_settings).screenBuff = NULL;
    (*_settings).fileName = NULL;
}

void version()
{
    printf("pager.exe - Version: 0.5.0 - Date: 2019-12-20\n");
}

void help()
{
    printf("Usage:\n");
    printf("    pager.exe [/? | /V] <filename> [/N]\n");
    printf("        or\n");
    printf("    type <filename> | pager.exe [/N]\n");
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
    printf("Switches:\n");
    printf("    /?                      = show help\n");
    printf("    /V                      = show version\n");
    printf("    /N                      = show line numbers\n");
    printf("\n");
}