#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include <stdio.h>

void *xmalloc(size_t size);
void *xrealloc(void *ptr, size_t size);
char *copystr(const char *src);
char *upperstr(const char *src);

#ifdef _WIN32
FILE *winfopen(const char *path, const char *mode);
#define fopen_internal winfopen
#else
#define fopen_internal fopen
#endif

#endif
