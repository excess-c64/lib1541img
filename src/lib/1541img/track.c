#include <stdlib.h>

#include "util.h"
#include "log.h"

#include <1541img/track.h>
#include <1541img/sector.h>

struct Track
{
    uint8_t tracknum;
    uint8_t sectors;
    Sector *sector[];
};

static uint8_t getsectors(uint8_t tracknum)
{
    if (!tracknum || tracknum > 40) return 0;
    if (tracknum < 18) return 21;
    if (tracknum < 25) return 19;
    if (tracknum < 31) return 18;
    return 17;
}

Track *Track_create(uint8_t tracknum)
{
    uint8_t sectors = getsectors(tracknum);
    if (!sectors)
    {
        logfmt(L_ERROR, "Track: invalid track number %hhu.", tracknum);
        return 0;
    }
    Track *self = xmalloc(sizeof *self + sectors * sizeof *self->sector);
    for (uint8_t sectornum = 0; sectornum < sectors; ++sectornum)
    {
	self->sector[sectornum] = Sector_createAt(tracknum, sectornum);
    }
    self->tracknum = tracknum;
    self->sectors = sectors;
    return self;
}

const Sector *Track_rsector(const Track *self, uint8_t sectornum)
{
    if (sectornum >= self->sectors)
    {
        logfmt(L_WARNING, "Track_rsector: invalid sector number %hhu "
		"requested.", sectornum);
        return 0;
    }
    return self->sector[sectornum];
}

Sector *Track_sector(Track *self, uint8_t sectornum)
{
    if (sectornum >= self->sectors)
    {
        logfmt(L_WARNING, "Track_sector: invalid sector number %hhu requested.",
                sectornum);
        return 0;
    }
    return self->sector[sectornum];
}

uint8_t Track_trackNum(const Track *self)
{
    return self->tracknum;
}

uint8_t Track_sectors(const Track *self)
{
    return self->sectors;
}

void Track_destroy(Track *self)
{
    if (!self) return;
    for (uint8_t sectornum = 0; sectornum < self->sectors; ++sectornum)
    {
	Sector_destroy(self->sector[sectornum]);
    }
    free(self);
}

