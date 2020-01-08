#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

FILE *winfopen(const char *path, const char *mode)
{
    LPWSTR wpath;
    LPWSTR wmode;

    FILE *file = 0;

    size_t pathlen = strlen(path) + 1;
    size_t modelen = strlen(mode) + 1;

    wpath = malloc(2 * pathlen);
    wmode = malloc(2 * modelen);

    if (!wpath || !wmode) goto done;

    if (!MultiByteToWideChar(CP_UTF8, 0, path, pathlen, wpath, pathlen))
    {
	goto done;
    }
    if (!MultiByteToWideChar(CP_UTF8, 0, mode, modelen, wmode, modelen))
    {
	goto done;
    }

    file = _wfopen(wpath, wmode);

done:
    free(wmode);
    free(wpath);
    return file;
}
