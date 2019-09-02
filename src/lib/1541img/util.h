#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *copystr(const char *src);
char *upperstr(const char *src);

#endif
