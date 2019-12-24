#ifndef _TERMTOOLS_H
#define _TERMTOOLS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Function:  basename 
 * --------------------
 * returns the last element of a path.
 *
 * _IN:
 *      path: the full path
 *
 * RETURNS: a pointer to the start of the last element
 *           or NULL.
 */
const char *basename(const char *path)
{
    // if (!path) return NULL;
    int pathsep = '\\'; //sorry, Windows only...
    const char *tmp = strrchr(path, pathsep);
    if (tmp) return tmp + 1;
    else return tmp;
}



#endif // _TERMTOOLS_H