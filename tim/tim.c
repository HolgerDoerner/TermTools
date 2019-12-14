/*
 * TIM - Type IMproved
*/

#ifdef _MSC_VER
  #define _CRT_SECURE_NO_WARNINGS
  #define WIN32_LEAN_AND_MEAN
  #define USE_LIBCMT
  #pragma comment(lib, "user32.lib")
  #pragma comment(lib, "advapi32.lib")
  #pragma comment(lib, "pdcurses.lib")
#endif

#include <windows.h>

#ifdef MOUSE_MOVED
    #undef MOUSE_MOVED
#endif // MOUSE_MOVED

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#ifndef PDC_WIDE
    #define PDC_WIDE
#endif

#ifndef PDC_FORCE_UTF8
    #define PDC_FORCE_UTF8
#endif // PDC_FORCE_UTF8

#include <curses.h>

#define TOP 0
#define BOTTOM 1
#define TRUE 1
#define FALSE 0

/* global vars */
int lineCount = 0;
int caretPos = 1;
char inBuf[BUFSIZ];
char **screenBuff;
wchar_t **testBuff;
int allocCount = 1;
int sbCount = 0;
FILE *pFile;
WINDOW *term;
short isStdinRedirected = FALSE;
/* END (global vars) */

void scrollUp(const int);
void scrollDown(const int);
void scrollHomeEnd(const int);
void cleanup(void);
void help(void);

int main(int argc, char **argv)
{
    setlocale(LC_ALL, "UTF-8");
    
    if (argc >= 2)
    {
        if (strstr(argv[1], "/?"))
        {
            help();
            exit(0);
        }

        pFile = fopen(argv[1], "r");
        if (pFile == NULL)
        {
            fprintf(stderr, "* ERROR: Could not open %s", argv[1]);
        }
    }
    else
    {
        pFile = stdin;
        isStdinRedirected = TRUE;
    }

    screenBuff = (char **)malloc(sizeof(char *) * (BUFSIZ * allocCount));

    while (fgets(inBuf, BUFSIZ, pFile))
    {
        screenBuff[sbCount] = (char *)malloc(sizeof(char) * BUFSIZ);
        if (screenBuff[sbCount] == NULL)
        {
            fprintf(stderr, "* ERROR: Failed memory allocation!\n");
            exit(1);
        }

        snprintf(screenBuff[sbCount++], BUFSIZ, "%s", inBuf);


        if (feof(pFile)) break;

        if ((sbCount > 0) && ((sbCount % BUFSIZ) == 0))
        {
            screenBuff = (char **)realloc(screenBuff, sizeof(char *) * (BUFSIZ * ++allocCount));

            if (screenBuff == NULL)
            {
                fprintf(stderr, "* ERROR: Failed reallocation of ScreenBuffer!\n");
                exit(1);
            }
        }
    }

    if (!isStdinRedirected && fclose(pFile))
    {
        fprintf(stderr, "* ERROR: Closing %s failed!\n", argv[1]);
        exit(1);
    }
    else
    {
        freopen("CON", "r", stdin);
    }
    
    term = initscr();
    raw();
    keypad(term, TRUE);
    scrollok(term, TRUE);
    noecho();
    curs_set(0);

    for (; lineCount < (LINES - 1); ++lineCount)
    {
        waddstr(term, screenBuff[lineCount]);
    }

    // main loop
    while (1)
    {
        switch (getch())
        {
            case VK_ESCAPE: case 3: exit(0);
            case KEY_DOWN: case VK_DOWN: case KEY_C2: case 'j':
                scrollDown(1); break;
            case KEY_UP: case VK_UP: case KEY_A2: case 'k':
                scrollUp(1); break;
            case KEY_NPAGE: case VK_NEXT: case 457: case 'h':
                scrollDown(LINES); break;
            case KEY_PPAGE: case VK_PRIOR: case 451: case 'l':
                scrollUp(LINES); break;
            case KEY_HOME: case VK_HOME: case 449:
                scrollHomeEnd(0); break;
            case KEY_END: case VK_END: case 455:
                scrollHomeEnd(1); break;
            default: break;
        }
    }

    cleanup();
    endwin();
}

void scrollUp(const int range)
{
    for (int i = 0; i < range; ++i)
    {
        if (caretPos == BOTTOM)
        {
            lineCount -= LINES;
            caretPos = TOP;
        }

        if (lineCount >= 0)
        {
            wscrl(term, -1);
            move(0, 0);
            waddstr(term, screenBuff[lineCount--]);
            wrefresh(term);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }
}

void scrollDown(const int range)
{
    for (int i = 0; i < range; ++i)
    {
        if (caretPos == TOP)
        {
            lineCount += LINES;
            caretPos = BOTTOM;
            move(LINES-1, 0);
        }

        if (lineCount < sbCount)
        {
            waddstr(term, screenBuff[lineCount++]);
            wrefresh(term);
        }
        else
        {
            beep();
            flash();
            break;
        }
    }
}

void scrollHomeEnd(const int direction)
{
    if ( ((lineCount == LINES-1) & (direction == TOP))
        || ((lineCount == sbCount) & (direction == BOTTOM)) )
    {
        beep();
        flash();
    }

    move(0, 0);
    lineCount = direction ? (sbCount - LINES) : 0;
    for (int i = 0; i < (direction ? LINES : LINES-1); ++i)
    {
        waddstr(term, screenBuff[lineCount++]);
    }
    caretPos = 1;
    wrefresh(term);
}

void cleanup()
{
    for (int i = 0; i < (BUFSIZ * allocCount); ++i)
    {
        free(screenBuff[i]);
        screenBuff[i] = NULL;
    }

    free(screenBuff);
    screenBuff = NULL;
}

void help()
{
    _wprintf_p(L"Usage:\n");
    _wprintf_p(L"    TIM.EXE <filename>\n");
    _wprintf_p(L"        or\n");
    _wprintf_p(L"    type <filename> | TIM.EXE\n");
    _wprintf_p(L"\n");
    _wprintf_p(L"Controls:\n");
    _wprintf_p(L"    ARROW_UP, K    = scroll 1 line up\n");
    _wprintf_p(L"    ARROW_DOWN, J  = scroll 1 line down\n");
    _wprintf_p(L"    PG_UP, l       = scroll 1 page up\n");
    _wprintf_p(L"    PG_DOWN, h     = scroll 1 page down\n");
    _wprintf_p(L"    HOME           = jump to beginning of the input\n");
    _wprintf_p(L"    END            = jump to end of the input\n");
    _wprintf_p(L"    ESC, CTRL+C    = exit\n");
    _wprintf_p(L"\n");
}