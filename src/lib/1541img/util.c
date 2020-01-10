#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "log.h"

#include "util.h"

SOLOCAL void *xmalloc(size_t size)
{
    void *m = malloc(size);
    if (!m)
    {
        logmsg(L_FATAL, "memory allocation failed.");
        abort();
    }
    return m;
}

SOLOCAL void *xrealloc(void *ptr, size_t size)
{
    void *m = realloc(ptr, size);
    if (!m)
    {
        logmsg(L_FATAL, "memory allocation failed.");
        abort();
    }
    return m;
}

SOLOCAL char *copystr(const char *src)
{
    if (!src) return 0;
    char *copy = xmalloc(strlen(src) +1);
    strcpy(copy, src);
    return copy;
}

SOLOCAL char *upperstr(const char *src)
{
    if (!src) return 0;
    char *upper = xmalloc(strlen(src) +1);
    const char *r = src;
    char *w = upper;
    while (*r)
    {
	*w++ = toupper(*r++);
    }
    *w = 0;
    return upper;
}

