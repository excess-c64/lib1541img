#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include <1541img/filedata.h>
#include <1541img/event.h>

#include <1541img/cbmdosfile.h>

static const char *exts[] =
{
    "DEL",
    "SEQ",
    "PRG",
    "USR",
    "REL"
};

const char *CbmdosFileType_name(CbmdosFileType type)
{
    if (type < CFT_DEL || type > CFT_REL) return 0;
    return exts[type];
}

struct CbmdosFile
{
    CbmdosFileType type;
    int locked;
    int closed;
    char *name;
    FileData *data;
    Event *changedEvent;
    uint8_t nameLength;
    uint8_t recordLength;
    uint16_t forcedBlocks;
};

static void fileDataHandler(void *receiver, int id, const void *sender,
        const void *args)
{
    (void)id;
    (void)args;
    (void)sender;

    CbmdosFileEventArgs ea = { CFE_DATACHANGED };
    CbmdosFile *self = receiver;
    Event_raise(self->changedEvent, &ea);
}

CbmdosFile *CbmdosFile_create(void)
{
    CbmdosFile *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->data = FileData_create();
    self->changedEvent = Event_create(0, self);
    self->type = CFT_PRG;
    self->closed = 1;
    self->forcedBlocks = 0xffff;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    return self;
}

CbmdosFileType CbmdosFile_type(const CbmdosFile *self)
{
    return self->type;
}

int CbmdosFile_setType(CbmdosFile *self, CbmdosFileType type)
{
    if (type < CFT_DEL || type > CFT_REL)
    {
        logmsg(L_WARNING, "CbmdosFile_setType: invalid type.");
        return -1;
    }
    if (type == self->type) return 0;
    if (type == CFT_DEL)
    {
	CbmdosFile_setData(self, FileData_create());
    }
    self->type = type;
    CbmdosFileEventArgs args = { CFE_TYPECHANGED };
    Event_raise(self->changedEvent, &args);
    return 0;
}

const char *CbmdosFile_name(const CbmdosFile *self, uint8_t *length)
{
    if (length)
    {
        *length = self->nameLength;
    }
    return self->name;
}

void CbmdosFile_setName(CbmdosFile *self, const char *name, uint8_t length)
{
    if (length > 16)
    {
        logmsg(L_WARNING, "CbmdosFile_setName: truncating long name.");
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
            logmsg(L_WARNING, "CbmdosFile_setName: length > 0 given for empty "
                    "name.");
            length = 0;
        }
    }
    self->nameLength = length;
    CbmdosFileEventArgs args = { CFE_NAMECHANGED };
    Event_raise(self->changedEvent, &args);
}

const FileData *CbmdosFile_rdata(const CbmdosFile *self)
{
    return self->data;
}

FileData *CbmdosFile_data(CbmdosFile *self)
{
    return self->data;
}

void CbmdosFile_setData(CbmdosFile *self, FileData *data)
{
    Event_unregister(FileData_changedEvent(self->data), self, fileDataHandler);
    FileData_destroy(self->data);
    self->data = data;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    CbmdosFileEventArgs ea = { CFE_DATACHANGED };
    Event_raise(self->changedEvent, &ea);
}

uint8_t CbmdosFile_recordLength(const CbmdosFile *self)
{
    return self->recordLength;
}

int CbmdosFile_setRecordLength(CbmdosFile *self, uint8_t recordLength)
{
    if (recordLength > 254)
    {
        logmsg(L_WARNING, "CbmdosFile_setRecordLength: invalid length, "
                "max is 254.");
        return -1;
    }
    self->recordLength = recordLength;
    return 0;
}

uint16_t CbmdosFile_realBlocks(const CbmdosFile *self)
{
    if (self->type == CFT_DEL || !self->data) return 0;
    size_t size = FileData_size(self->data);
    uint16_t blocks = size / 254;
    if (size % 254) ++blocks;
    return blocks;
}

uint16_t CbmdosFile_blocks(const CbmdosFile *self)
{
    if (self->forcedBlocks != 0xffff) return self->forcedBlocks;
    return CbmdosFile_realBlocks(self);
}

uint16_t CbmdosFile_forcedBlocks(const CbmdosFile *self)
{
    return self->forcedBlocks;
}

void CbmdosFile_setForcedBlocks(CbmdosFile *self, uint16_t forcedBlocks)
{
    self->forcedBlocks = forcedBlocks;
    CbmdosFileEventArgs args = { CFE_FORCEDBLOCKSCHANGED };
    Event_raise(self->changedEvent, &args);
}

int CbmdosFile_locked(const CbmdosFile *self)
{
    return self->locked;
}

void CbmdosFile_setLocked(CbmdosFile *self, int locked)
{
    self->locked = locked;
    CbmdosFileEventArgs args = { CFE_LOCKEDCHANGED };
    Event_raise(self->changedEvent, &args);
}

int CbmdosFile_closed(const CbmdosFile *self)
{
    return self->closed;
}

void CbmdosFile_setClosed(CbmdosFile *self, int closed)
{
    self->closed = closed;
    CbmdosFileEventArgs args = { CFE_CLOSEDCHANGED };
    Event_raise(self->changedEvent, &args);
}

void CbmdosFile_getDirLine(const CbmdosFile *self, uint8_t *line)
{
    int blocklen = sprintf((char *)line, "%u", CbmdosFile_blocks(self));
    memset(line + blocklen, 0xa0, 24 - blocklen);
    memcpy(line + 6, self->name, self->nameLength);
    memcpy(line + 24, CbmdosFileType_name(self->type), 3);
    line[5] = 0x22;
    line[22] = 0x22;
}

Event *CbmdosFile_changedEvent(CbmdosFile *self)
{
    return self->changedEvent;
}

void CbmdosFile_destroy(CbmdosFile *self)
{
    if (!self) return;
    free(self->name);
    Event_destroy(self->changedEvent);
    FileData_destroy(self->data);
}

