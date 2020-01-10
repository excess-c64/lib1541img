#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include <1541img/event.h>

#include <1541img/filedata.h>

#define FD_CHUNKSIZE 1024

struct FileData
{
    size_t size;
    size_t capacity;
    Event *changedEvent;
    uint8_t *content;
};

SOEXPORT FileData *FileData_create(void)
{
    FileData *self = xmalloc(sizeof *self);
    self->content = xmalloc(FD_CHUNKSIZE);
    self->size = 0;
    self->capacity = FD_CHUNKSIZE;
    self->changedEvent = Event_create(0, self);
    return self;
}

SOEXPORT FileData *FileData_clone(const FileData *self)
{
    FileData *cloned = xmalloc(sizeof *cloned);
    cloned->size = self->size;
    cloned->capacity = self->capacity;
    cloned->content = xmalloc(self->capacity);
    cloned->changedEvent = Event_create(0, cloned);
    memcpy(cloned->content, self->content, self->size);
    return cloned;
}

SOEXPORT size_t FileData_size(const FileData *self)
{
    return self->size;
}

SOEXPORT const uint8_t *FileData_rcontent(const FileData *self)
{
    return self->content;
}

SOEXPORT int FileData_append(FileData *self, const uint8_t *data, size_t size)
{
    if (self->size + size < size || self->size + size > FILEDATA_MAXSIZE)
    {
        logmsg(L_ERROR, "FileData_append: maximum size exceeded.");
        return -1;
    }
    while (self->size + size > self->capacity)
    {
        self->capacity += FD_CHUNKSIZE;
        self->content = xrealloc(self->content, self->capacity);
    }
    memcpy(self->content + self->size, data, size);
    self->size += size;
    Event_raise(self->changedEvent, 0);
    return 0;
}

SOEXPORT int FileData_appendByte(FileData *self, uint8_t byte)
{
    if (self->size == FILEDATA_MAXSIZE)
    {
        logmsg(L_ERROR, "FileData_appendByte: maximum size exceeded.");
        return -1;
    }
    if (self->size == self->capacity)
    {
        self->capacity += FD_CHUNKSIZE;
        self->content = xrealloc(self->content, self->capacity);
    }
    self->content[self->size++] = byte;
    Event_raise(self->changedEvent, 0);
    return 0;
}

SOEXPORT int FileData_appendBytes(FileData *self, uint8_t byte, size_t count)
{
    if (self->size + count < count || self->size + count > FILEDATA_MAXSIZE)
    {
        logmsg(L_ERROR, "FileData_appendBytes: maximum size exceeded.");
        return -1;
    }
    while (self->size + count > self->capacity)
    {
        self->capacity += FD_CHUNKSIZE;
        self->content = xrealloc(self->content, self->capacity);
    }
    memset(self->content + self->size, byte, count);
    self->size += count;
    Event_raise(self->changedEvent, 0);
    return 0;
}

SOEXPORT int FileData_setByte(FileData *self, uint8_t byte, size_t pos)
{
    if (pos >= self->size)
    {
	logmsg(L_ERROR, "FileData_setByte: invalid position.");
	return -1;
    }
    self->content[pos] = byte;
    return 0;
}

SOEXPORT Event *FileData_changedEvent(FileData *self)
{
    return self->changedEvent;
}

SOEXPORT void FileData_destroy(FileData *self)
{
    if (!self) return;
    Event_destroy(self->changedEvent);
    free(self->content);
    free(self);
}
