#include <stdlib.h>

#include "util.h"
#include "log.h"
#include <1541img/event.h>
#include <1541img/filedata.h>

#include <1541img/cbmdosinode.h>

struct CbmdosInode
{
    FileData *data;
    Event *changedEvent;
    CbmdosFsOptOverrides overrides;
};

static void fileDataHandler(void *receiver, int id, const void *sender,
        const void *args)
{
    (void)id;
    (void)args;
    (void)sender;

    CbmdosInodeEventArgs ea = { CIE_DATACHANGED };
    CbmdosInode *self = receiver;
    Event_raise(self->changedEvent, &ea);
}

SOEXPORT CbmdosInode *CbmdosInode_create(void)
{
    CbmdosInode *self = xmalloc(sizeof *self);
    self->data = FileData_create();
    self->changedEvent = Event_create(0, self);
    self->overrides = CFOO_NONE;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    return self;
}

SOEXPORT CbmdosInode *CbmdosInode_clone(const CbmdosInode *other)
{
    CbmdosInode *self = xmalloc(sizeof *self);
    self->data = FileData_clone(other->data);
    self->changedEvent = Event_create(0, self);
    self->overrides = other->overrides;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    return self;
}

SOEXPORT const FileData *CbmdosInode_rdata(const CbmdosInode *self)
{
    return self->data;
}

SOEXPORT FileData *CbmdosInode_data(CbmdosInode *self)
{
    return self->data;
}

SOEXPORT void CbmdosInode_setData(CbmdosInode *self, FileData *data)
{
    Event_unregister(FileData_changedEvent(self->data), self, fileDataHandler);
    FileData_destroy(self->data);
    self->data = data;
    Event_register(FileData_changedEvent(self->data), self, fileDataHandler);
    CbmdosInodeEventArgs ea = { CIE_DATACHANGED };
    Event_raise(self->changedEvent, &ea);
}

SOEXPORT uint16_t CbmdosInode_blocks(const CbmdosInode *self)
{
    if (!self->data)
    {
	return 0;
    }
    size_t size = FileData_size(self->data);
    uint16_t blocks = size / 254;
    if (size % 254) ++blocks;
    return blocks;
}

SOEXPORT CbmdosFsOptOverrides CbmdosInode_optOverrides(
        const CbmdosInode *self)
{
    return self->overrides;
}

SOEXPORT void CbmdosInode_setOptOverrides(
        CbmdosInode *self, CbmdosFsOptOverrides overrides)
{
    if (overrides.mask != self->overrides.mask
	    || overrides.flags != self->overrides.flags)
    {
	self->overrides = overrides;
	CbmdosInodeEventArgs args = { CIE_OPTOVERRIDESCHANGED };
	Event_raise(self->changedEvent, &args);
    }
}

SOEXPORT Event *CbmdosInode_changedEvent(CbmdosInode *self)
{
    return self->changedEvent;
}

SOEXPORT void CbmdosInode_destroy(CbmdosInode *self)
{
    if (!self) return;
    Event_destroy(self->changedEvent);
    FileData_destroy(self->data);
    free(self);
}

