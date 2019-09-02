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

CbmdosVfs *CbmdosVfs_create(void)
{
    CbmdosVfs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->files = xmalloc(DIRCHUNKSIZE * sizeof *self->files);
    self->fileCapa = DIRCHUNKSIZE;
    self->dosver = 0x41;
    self->changedEvent = Event_create(0, self);
    return self;
}

uint8_t CbmdosVfs_dosver(const CbmdosVfs *self)
{
    return self->dosver;
}

void CbmdosVfs_setDosver(CbmdosVfs *self, uint8_t dosver)
{
    self->dosver = dosver;
    CbmdosVfsEventArgs args = { .what = CVE_DOSVERCHANGED };
    Event_raise(self->changedEvent, &args);
}

const char *CbmdosVfs_name(const CbmdosVfs *self, uint8_t *length)
{
    if (length)
    {
        *length = self->nameLength;
    }
    return self->name;
}

void CbmdosVfs_setName(CbmdosVfs *self, const char *name, uint8_t length)
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

const char *CbmdosVfs_id(const CbmdosVfs *self, uint8_t *length)
{
    if (length)
    {
        *length = self->idLength;
    }
    return self->id;
}

void CbmdosVfs_setId(CbmdosVfs *self, const char *id, uint8_t length)
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

unsigned CbmdosVfs_fileCount(const CbmdosVfs *self)
{
    return self->fileCount;
}

const CbmdosFile *CbmdosVfs_rfile(const CbmdosVfs *self, unsigned pos)
{
    if (pos >= self->fileCount) return 0;
    return self->files[pos];
}

CbmdosFile *CbmdosVfs_file(CbmdosVfs *self, unsigned pos)
{
    if (pos >= self->fileCount) return 0;
    return self->files[pos];
}

int CbmdosVfs_delete(CbmdosVfs *self, const CbmdosFile *file)
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

int CbmdosVfs_deleteAt(CbmdosVfs *self, unsigned pos)
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

int CbmdosVfs_append(CbmdosVfs *self, CbmdosFile *file)
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

int CbmdosVfs_insert(CbmdosVfs *self, CbmdosFile *file, unsigned pos)
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

Event *CbmdosVfs_changedEvent(CbmdosVfs *self)
{
    return self->changedEvent;
}

void CbmdosVfs_destroy(CbmdosVfs *self)
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

