#include <stdlib.h>
#include <string.h>

#include "util.h"

#include "filename.h"

#ifdef _WIN32
#define PATHSEP '\\'
#else
#define PATHSEP '/'
#endif

struct Filename
{
    char *full;
    char *dir;
    char *base;
    char *ext;
};

SOLOCAL Filename *Filename_create(void)
{
    Filename *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    return self;
}

SOLOCAL Filename *Filename_clone(const Filename *self)
{
    Filename *clone = Filename_create();
    if (self->full) clone->full = copystr(self->full);
    if (self->dir) clone->dir = copystr(self->dir);
    if (self->base) clone->base = copystr(self->base);
    if (self->ext) clone->ext = copystr(self->ext);
    return clone;
}

SOLOCAL const char *Filename_full(const Filename *self)
{
    return self->full;
}

SOLOCAL const char *Filename_dir(const Filename *self)
{
    return self->dir;
}

SOLOCAL const char *Filename_base(const Filename *self)
{
    return self->base;
}

SOLOCAL const char *Filename_ext(const Filename *self)
{
    return self->ext;
}

SOLOCAL void Filename_setFull(Filename *self, const char *full)
{
    free(self->full);
    free(self->dir);
    free(self->base);
    free(self->ext);
    memset(self, 0, sizeof *self);
    if (!full) return;
    self->full = copystr(full);
#ifdef _WIN32
    for (char *c = self->full; *c; ++c)
    {
        if (*c == '/') *c = PATHSEP;
    }
#endif
    const char *pathsep = strrchr(self->full, PATHSEP);
    const char *base;
    if (pathsep)
    {
	size_t dirlen = pathsep - self->full;
	self->dir = xmalloc(dirlen + 1);
	strncpy(self->dir, self->full, dirlen);
	self->dir[dirlen] = 0;
	base = pathsep+1;
    }
    else
    {
	base = self->full;
    }

    const char *extsep = *base ? strrchr(base, '.') : 0;
    if (extsep)
    {
	self->ext = copystr(extsep+1);
	size_t baselen = extsep - base;
	self->base = xmalloc(baselen + 1);
	strncpy(self->base, base, baselen);
	self->base[baselen] = 0;
    }
    else
    {
	self->base = *base ? copystr(base) : 0;
    }
}

static void Filename_updateFull(Filename *self)
{
    free(self->full);
    if (!self->base)
    {
	self->full = 0;
	return;
    }
    size_t fulllen = strlen(self->base);
    if (self->dir) fulllen += strlen(self->dir) + 1;
    if (self->ext) fulllen += strlen(self->ext) + 1;
    self->full = xmalloc(fulllen+1);
    char *wrptr = self->full;
    if (self->dir)
    {
	strcpy(wrptr, self->dir);
	wrptr += strlen(self->dir);
	*wrptr++ = PATHSEP;
    }
    strcpy(wrptr, self->base);
    wrptr += strlen(self->base);
    if (self->ext)
    {
	*wrptr++ = '.';
	strcpy(wrptr, self->ext);
    }
}

SOLOCAL void Filename_setDir(Filename *self, const char *dir)
{
    free(self->dir);
    self->dir = copystr(dir);
    Filename_updateFull(self);
}

SOLOCAL void Filename_setBase(Filename *self, const char *base)
{
    free(self->base);
    self->base = copystr(base);
    Filename_updateFull(self);
}

SOLOCAL void Filename_setExt(Filename *self, const char *ext)
{
    free(self->ext);
    self->ext = copystr(ext);
    Filename_updateFull(self);
}

SOLOCAL void Filename_destroy(Filename *self)
{
    if (!self) return;
    free(self->full);
    free(self->dir);
    free(self->base);
    free(self->ext);
    free(self);
}

