#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include <1541img/event.h>
#include <1541img/cbmdosfile.h>

#include <1541img/cbmdosvfs.h>

#define DIRCHUNKSIZE 144

struct CbmdosVfs
{
    char *name;
    char *id;
    CbmdosFile **files;
    Event *changedEvent;
    unsigned fileCount;
    unsigned fileCapa;
    uint8_t nameLength;
    uint8_t idLength;
    uint8_t dosver;
};

static void fileHandler(void *receiver, int id, const void *sender,
        const void *args)
{
    (void)id;

    CbmdosVfs *self = receiver;
    unsigned pos;
    for (pos = 0; pos < self->fileCount; ++pos)
    {
        if (self->files[pos] == sender) break;
    }
    CbmdosVfsEventArgs ea = {
        .what = CVE_FILECHANGED,
        .fileEventArgs = args,
        .filepos = pos
    };
    Event_raise(self->changedEvent, &ea);
}

SOEXPORT CbmdosVfs *CbmdosVfs_create(void)
{
    CbmdosVfs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->files = xmalloc(DIRCHUNKSIZE * sizeof *self->files);
    self->fileCapa = DIRCHUNKSIZE;
    self->dosver = 0x41;
    self->changedEvent = Event_create(0, self);
    return self;
}

SOEXPORT uint8_t CbmdosVfs_dosver(const CbmdosVfs *self)
{
    return self->dosver;
}

SOEXPORT void CbmdosVfs_setDosver(CbmdosVfs *self, uint8_t dosver)
{
    self->dosver = dosver;
    CbmdosVfsEventArgs args = { .what = CVE_DOSVERCHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT const char *CbmdosVfs_name(const CbmdosVfs *self, uint8_t *length)
{
    if (length)
    {
        *length = self->nameLength;
    }
    return self->name;
}

SOEXPORT void CbmdosVfs_setName(
	CbmdosVfs *self, const char *name, uint8_t length)
{
    if (length > 16)
    {
        logmsg(L_WARNING, "CbmdosVfs_setName: truncating long name.");
        length = 16;
    }
    free(self->name);
    if (name)
    {
        self->name = xmalloc(length+1);
        memcpy(self->name, name, length);
        self->name[length] = 0;
    }
    else
    {
        self->name = 0;
        if (length)
        {
            logmsg(L_WARNING, "CbmdosVfs_setName: length > 0 given for empty "
                    "name.");
            length = 0;
        }
    }
    self->nameLength = length;
    CbmdosVfsEventArgs args = { .what = CVE_NAMECHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT const char *CbmdosVfs_id(const CbmdosVfs *self, uint8_t *length)
{
    if (length)
    {
        *length = self->idLength;
    }
    return self->id;
}

SOEXPORT void CbmdosVfs_setId(CbmdosVfs *self, const char *id, uint8_t length)
{
    if (length > 5)
    {
        logmsg(L_WARNING, "CbmdosVfs_setId: truncating long id.");
        length = 5;
    }
    free(self->id);
    if (id)
    {
        self->id = xmalloc(length+1);
        memcpy(self->id, id, length);
        self->id[length] = 0;
    }
    else
    {
        self->id = 0;
        if (length)
        {
            logmsg(L_WARNING, "CbmdosVfs_setId: length > 0 given for empty "
                    "id.");
            length = 0;
        }
    }
    self->idLength = length;
    CbmdosVfsEventArgs args = { .what = CVE_IDCHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT unsigned CbmdosVfs_fileCount(const CbmdosVfs *self)
{
    return self->fileCount;
}

SOEXPORT const CbmdosFile *CbmdosVfs_rfile(const CbmdosVfs *self, unsigned pos)
{
    if (pos >= self->fileCount) return 0;
    return self->files[pos];
}

SOEXPORT CbmdosFile *CbmdosVfs_file(CbmdosVfs *self, unsigned pos)
{
    if (pos >= self->fileCount) return 0;
    return self->files[pos];
}

SOEXPORT int CbmdosVfs_delete(CbmdosVfs *self, const CbmdosFile *file)
{
    unsigned pos;
    for (pos = 0; pos < self->fileCount; ++pos)
    {
        if (self->files[pos] == file) break;
    }
    if (pos == self->fileCount)
    {
        logmsg(L_WARNING, "CbmdosVfs_delete: file not found.");
        return -1;
    }
    return CbmdosVfs_deleteAt(self, pos);
}

SOEXPORT int CbmdosVfs_deleteAt(CbmdosVfs *self, unsigned pos)
{
    if (pos >= self->fileCount)
    {
        logmsg(L_WARNING, "CbmdosVfs_deleteAt: file not found.");
        return -1;
    }
    CbmdosFile_destroy(self->files[pos]);
    if (pos < --self->fileCount)
    {
        memmove(self->files + pos, self->files + pos + 1,
                (self->fileCount - pos) * sizeof *self->files);
    }
    CbmdosVfsEventArgs args = {
        .what = CVE_FILEDELETED,
        .filepos = pos
    };
    Event_raise(self->changedEvent, &args);
    return 0;
}

static int ensureSpace(CbmdosVfs *self)
{
    if (self->fileCount == self->fileCapa)
    {
        unsigned newCapa = self->fileCapa + DIRCHUNKSIZE;
        if (newCapa <= self->fileCapa)
        {
            logmsg(L_ERROR, "CbmdosVfs: directory overflow.");
            return -1;
        }
        CbmdosFile **newFiles = realloc(self->files,
                newCapa * sizeof *newFiles);
        if (!newFiles)
        {
            logmsg(L_ERROR, "CbmdosVfs: memory allocation failed.");
            return -1;
        }
        self->files = newFiles;
        self->fileCapa = newCapa;
    }
    return 0;
}

SOEXPORT int CbmdosVfs_append(CbmdosVfs *self, CbmdosFile *file)
{
    if (ensureSpace(self) < 0) return -1;
    self->files[self->fileCount++] = file;
    Event_register(CbmdosFile_changedEvent(file), self, fileHandler);
    CbmdosVfsEventArgs args = {
        .what = CVE_FILEADDED,
        .filepos = self->fileCount - 1
    };
    Event_raise(self->changedEvent, &args);
    return 0;
}

SOEXPORT int CbmdosVfs_insert(CbmdosVfs *self, CbmdosFile *file, unsigned pos)
{
    if (pos >= self->fileCount) return CbmdosVfs_append(self, file);
    if (ensureSpace(self) < 0) return -1;
    memmove(self->files + pos + 1, self->files + pos,
            (self->fileCount++ - pos) * sizeof *self->files);
    self->files[pos] = file;
    Event_register(CbmdosFile_changedEvent(file), self, fileHandler);
    CbmdosVfsEventArgs args = {
        .what = CVE_FILEADDED,
        .filepos = pos
    };
    Event_raise(self->changedEvent, &args);
    return 0;
}

SOEXPORT int CbmdosVfs_move(CbmdosVfs *self, unsigned to, unsigned from)
{
    if (from >= self->fileCount) return -1;
    if (to >= self->fileCount) to = self->fileCount - 1;
    if (to == from) return 0;
    CbmdosFile *tmp = self->files[from];
    if (to > from)
    {
	memmove(self->files + from, self->files + from + 1,
		(to - from) * sizeof *self->files);
    }
    else
    {
	memmove(self->files + to + 1, self->files + to,
		(from - to) * sizeof *self->files);
    }
    self->files[to] = tmp;
    CbmdosVfsEventArgs args = {
	.what = CVE_FILEMOVED,
	.filepos = from,
	.targetpos = to
    };
    Event_raise(self->changedEvent, &args);
    return 0;
}

SOEXPORT void CbmdosVfs_getDirHeader(const CbmdosVfs *self, uint8_t *line)
{
    memset(line, 0xa0, 24);
    line[0] = 0x22;
    line[17] = 0x22;
    line[22] = 0x32;
    line[23] = self->dosver;
    memcpy(line+1, self->name, self->nameLength);
    memcpy(line+19, self->id, self->idLength);
}

SOEXPORT Event *CbmdosVfs_changedEvent(CbmdosVfs *self)
{
    return self->changedEvent;
}

SOEXPORT void CbmdosVfs_destroy(CbmdosVfs *self)
{
    if (!self) return;
    for (unsigned pos = 0; pos < self->fileCount; ++pos)
    {
        CbmdosFile_destroy(self->files[pos]);
    }
    Event_destroy(self->changedEvent);
    free(self->files);
    free(self->name);
    free(self->id);
    free(self);
}

