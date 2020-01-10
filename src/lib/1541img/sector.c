#include <stdlib.h>
#include <string.h>

#include "util.h"
#include <1541img/sector.h>

struct Sector
{
    uint8_t tracknum;
    uint8_t sectornum;
    uint8_t content[SECTOR_SIZE];
};

SOEXPORT Sector *Sector_create(void)
{
    Sector *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    return self;
}

SOEXPORT Sector *Sector_createAt(uint8_t tracknum, uint8_t sectornum)
{
    Sector *self = Sector_create();
    Sector_setTrackNum(self, tracknum);
    Sector_setSectorNum(self, sectornum);
    return self;
}

SOEXPORT const uint8_t *Sector_rcontent(const Sector *self)
{
    return self->content;
}

SOEXPORT uint8_t *Sector_content(Sector *self)
{
    return self->content;
}

SOEXPORT uint8_t Sector_trackNum(const Sector *self)
{
    return self->tracknum;
}

SOEXPORT uint8_t Sector_sectorNum(const Sector *self)
{
    return self->sectornum;
}

SOEXPORT void Sector_setTrackNum(Sector *self, uint8_t tracknum)
{
    self->tracknum = tracknum;
}

SOEXPORT void Sector_setSectorNum(Sector *self, uint8_t sectornum)
{
    self->sectornum = sectornum;
}

SOEXPORT void Sector_destroy(Sector *self)
{
    free(self);
}

