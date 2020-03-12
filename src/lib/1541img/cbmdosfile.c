#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include <1541img/event.h>
#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>
#include <1541img/hostfilewriter.h>
#include <1541img/petscii.h>

#include <1541img/cbmdosfile.h>

static const char *exts[] =
{
    "DEL",
    "SEQ",
    "PRG",
    "USR",
    "REL"
};

SOEXPORT const char *CbmdosFileType_name(CbmdosFileType type)
{
    if (type < CFT_DEL || type > CFT_REL) return 0;
    return exts[type];
}

struct CbmdosFile
{
    CbmdosFileType type;
    int invalidType;
    int locked;
    int closed;
    int autoMapToLc;
    char *name;
    FileData *data;
    Event *changedEvent;
    uint8_t nameLength;
    uint8_t recordLength;
    uint16_t forcedBlocks;
    CbmdosFsOptOverrides overrides;
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

SOEXPORT CbmdosFile *CbmdosFile_create(void)
{
    CbmdosFile *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->data = FileData_create();
    self->changedEvent = Event_create(0, self);
    self->type = CFT_PRG;
    self->invalidType = -1;
    self->closed = 1;
    self->forcedBlocks = 0xffff;
    self->recordLength = 254;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    return self;
}

SOEXPORT CbmdosFile *CbmdosFile_clone(const CbmdosFile *other)
{
    CbmdosFile *self = xmalloc(sizeof *self);
    self->type = other->type;
    self->invalidType = other->invalidType;
    self->locked = other->locked;
    self->closed = other->closed;
    self->autoMapToLc = 0;
    self->name = 0;
    self->data = FileData_clone(other->data);
    self->changedEvent = Event_create(0, self);
    self->nameLength = 0;
    self->recordLength = other->recordLength;
    self->forcedBlocks = other->forcedBlocks;
    self->overrides = other->overrides;
    uint8_t len;
    const char *name = CbmdosFile_name(other, &len);
    CbmdosFile_setName(self, name, len);
    self->autoMapToLc = other->autoMapToLc;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    return self;
}

SOEXPORT CbmdosFileType CbmdosFile_type(const CbmdosFile *self)
{
    return self->type;
}

SOEXPORT int CbmdosFile_invalidType(const CbmdosFile *self)
{
    return self->invalidType;
}

SOEXPORT int CbmdosFile_setType(CbmdosFile *self, CbmdosFileType type)
{
    int rc = 0;
    int invalidType = -1;
    type &= 0xf;
    if (type < CFT_DEL || type > CFT_REL)
    {
        logmsg(L_WARNING, "CbmdosFile_setType: invalid type.");
	invalidType = type;
        type = CFT_DEL;
	rc = -1;
    }
    if (type == self->type && invalidType == self->invalidType) return rc;
    if ((self->invalidType < 0 && self->type == CFT_DEL) ||
	    (invalidType < 0 && type == CFT_DEL))
    {
	CbmdosFile_setData(self, FileData_create());
    }
    self->type = type;
    self->invalidType = invalidType;
    CbmdosFileEventArgs args = { CFE_TYPECHANGED };
    Event_raise(self->changedEvent, &args);
    return rc;
}

SOEXPORT const char *CbmdosFile_name(const CbmdosFile *self, uint8_t *length)
{
    if (length)
    {
        *length = self->nameLength;
    }
    return self->name;
}

SOEXPORT void CbmdosFile_setName(
	CbmdosFile *self, const char *name, uint8_t length)
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
    if (self->autoMapToLc)
    {
	petscii_mapUpperGfxToLower(self->name, self->nameLength);
    }
    CbmdosFileEventArgs args = { CFE_NAMECHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT void CbmdosFile_mapUpperGfxToLower(CbmdosFile *self)
{
    petscii_mapUpperGfxToLower(self->name, self->nameLength);
    CbmdosFileEventArgs args = { CFE_NAMECHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT const FileData *CbmdosFile_rdata(const CbmdosFile *self)
{
    return self->data;
}

SOEXPORT FileData *CbmdosFile_data(CbmdosFile *self)
{
    return self->data;
}

SOEXPORT void CbmdosFile_setData(CbmdosFile *self, FileData *data)
{
    Event_unregister(FileData_changedEvent(self->data), self, fileDataHandler);
    FileData_destroy(self->data);
    self->data = data;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    CbmdosFileEventArgs ea = { CFE_DATACHANGED };
    Event_raise(self->changedEvent, &ea);
}

SOEXPORT int CbmdosFile_exportRaw(const CbmdosFile *self, FILE *file)
{
    return writeHostFile(self->data, file);
}

SOEXPORT int CbmdosFile_exportPC64(const CbmdosFile *self, FILE *file)
{
    uint8_t header[26] = "C64File";
    memcpy(header+8, self->name, self->nameLength);
    if (self->type == CFT_REL) header[25] = self->recordLength;
    if (fwrite(header, sizeof header, 1, file) != 1) return -1;
    return writeHostFile(self->data, file);
}

SOEXPORT int CbmdosFile_import(CbmdosFile *self, FILE *file)
{
    FileData *data = readHostFile(file);
    if (!data) return -1;
    const uint8_t *content = FileData_rcontent(data);
    if (FileData_size(data) > 26 && !content[24]
	    && !memcmp(content, "C64File", sizeof "C64File"))
    {
	FileData *raw = FileData_create();
	if (!raw)
	{
	    FileData_destroy(data);
	    return -1;
	}
	if (FileData_append(raw, content+26, FileData_size(data)-26) < 0)
	{
	    FileData_destroy(raw);
	    FileData_destroy(data);
	    return -1;
	}
	CbmdosFile_setData(self, raw);
	CbmdosFile_setName(self, (const char *)content+8,
		strlen((const char *)content+8));
	CbmdosFile_setRecordLength(self, content[25] ? content[25] : 254);
	FileData_destroy(data);
    }
    else
    {
	CbmdosFile_setData(self, data);
    }
    return 0;
}

SOEXPORT uint8_t CbmdosFile_recordLength(const CbmdosFile *self)
{
    return self->recordLength;
}

SOEXPORT int CbmdosFile_setRecordLength(CbmdosFile *self, uint8_t recordLength)
{
    if (recordLength > 254)
    {
        logmsg(L_WARNING, "CbmdosFile_setRecordLength: invalid length, "
                "max is 254.");
        return -1;
    }
    if (recordLength < 1)
    {
        logmsg(L_WARNING, "CbmdosFile_setRecordLength: invalid length, "
                "min is 1.");
        return -1;
    }
    self->recordLength = recordLength;
    CbmdosFileEventArgs ea = { CFE_RECORDLENGTHCHANGED };
    Event_raise(self->changedEvent, &ea);
    return 0;
}

SOEXPORT uint16_t CbmdosFile_realBlocks(const CbmdosFile *self)
{
    if ((self->invalidType < 0 && self->type == CFT_DEL) || !self->data)
    {
	return 0;
    }
    size_t size = FileData_size(self->data);
    uint16_t blocks = size / 254;
    if (size % 254) ++blocks;
    return blocks;
}

SOEXPORT uint16_t CbmdosFile_blocks(const CbmdosFile *self)
{
    if (self->forcedBlocks != 0xffff) return self->forcedBlocks;
    return CbmdosFile_realBlocks(self);
}

SOEXPORT uint16_t CbmdosFile_forcedBlocks(const CbmdosFile *self)
{
    return self->forcedBlocks;
}

SOEXPORT void CbmdosFile_setForcedBlocks(
	CbmdosFile *self, uint16_t forcedBlocks)
{
    self->forcedBlocks = forcedBlocks;
    CbmdosFileEventArgs args = { CFE_FORCEDBLOCKSCHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT CbmdosFsOptOverrides CbmdosFile_optOverrides(
        const CbmdosFile *self)
{
    return self->overrides;
}

SOEXPORT void CbmdosFile_setOptOverrides(
        CbmdosFile *self, CbmdosFsOptOverrides overrides)
{
    if (overrides.mask != self->overrides.mask
	    || overrides.flags != self->overrides.flags)
    {
	self->overrides = overrides;
	CbmdosFileEventArgs args = { CFE_OPTOVERRIDESCHANGED };
	Event_raise(self->changedEvent, &args);
    }
}

SOEXPORT int CbmdosFile_locked(const CbmdosFile *self)
{
    return self->locked;
}

SOEXPORT void CbmdosFile_setLocked(CbmdosFile *self, int locked)
{
    self->locked = !!locked;
    CbmdosFileEventArgs args = { CFE_LOCKEDCHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT int CbmdosFile_closed(const CbmdosFile *self)
{
    return self->closed;
}

SOEXPORT void CbmdosFile_setClosed(CbmdosFile *self, int closed)
{
    self->closed = !!closed;
    CbmdosFileEventArgs args = { CFE_CLOSEDCHANGED };
    Event_raise(self->changedEvent, &args);
}

SOEXPORT int CbmdosFile_autoMapToLc(const CbmdosFile *self)
{
    return self->autoMapToLc;
}

SOEXPORT void CbmdosFile_setAutoMapToLc(CbmdosFile *self, int autoMapToLc)
{
    self->autoMapToLc = !!autoMapToLc;
}

SOEXPORT void CbmdosFile_getDirLine(const CbmdosFile *self, uint8_t *line)
{
    int blocklen = sprintf((char *)line, "%u", CbmdosFile_blocks(self));
    memset(line + blocklen, 0xa0, 28 - blocklen);
    memcpy(line + 6, self->name, self->nameLength);
    if (self->invalidType < 0)
    {
	memcpy(line + 24, CbmdosFileType_name(self->type), 3);
    }
    else
    {
	memcpy(line + 24, "?  ", 3);
    }
    line[5] = 0x22;
    uint8_t endidx = 6;
    int qclosed = 0;
    while (line[endidx] != 0xa0)
    {
	if (line[endidx] == 0x22) qclosed = 1;
	++endidx;
    }
    if (!qclosed) line[endidx] = 0x22;
    if (!self->closed) line[23] = 0x2a;
    if (self->locked) line[27] = 0x3c;
}

SOEXPORT Event *CbmdosFile_changedEvent(CbmdosFile *self)
{
    return self->changedEvent;
}

SOEXPORT void CbmdosFile_destroy(CbmdosFile *self)
{
    if (!self) return;
    free(self->name);
    Event_destroy(self->changedEvent);
    FileData_destroy(self->data);
    free(self);
}

