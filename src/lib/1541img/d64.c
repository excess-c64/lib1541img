#include <stdlib.h>

#include "util.h"
#include "log.h"
#include <1541img/track.h>
#include <1541img/sector.h>

#include <1541img/d64.h>

struct D64
{
    D64Type type;
    Track *track[];
};

static const uint8_t tracks[] = {35, 40};

D64 *D64_create(D64Type type)
{
    if (type < 0 || type > 1)
    {
        logmsg(L_ERROR, "D64: invalid type argument.");
        return 0;
    }
    D64 *self = xmalloc(sizeof *self + tracks[type] * sizeof *self->track);
    for (uint8_t tracknum = 0; tracknum < tracks[type]; ++tracknum)
    {
	self->track[tracknum] = Track_create(tracknum+1);
    }
    self->type = type;
    return self;
}

D64Type D64_type(const D64 *self)
{
    return self->type;
}

uint8_t D64_tracks(const D64 *self)
{
    return tracks[D64_type(self)];
}

const Track *D64_rtrack(const D64 *self, uint8_t tracknum)
{
    if (!tracknum || tracknum > tracks[D64_type(self)])
    {
        logfmt(L_WARNING, "D64_rtrack: invalid track number %hhu requested.",
                tracknum);
        return 0;
    }
    return self->track[tracknum-1];
}

Track *D64_track(D64 *self, uint8_t tracknum)
{
    if (!tracknum || tracknum > tracks[D64_type(self)])
    {
        logfmt(L_WARNING, "D64_track: invalid track number %hhu requested.",
                tracknum);
        return 0;
    }
    return self->track[tracknum-1];
}

const Sector *D64_rsector(const D64 *self, uint8_t tracknum, uint8_t sectornum)
{
    const Track *track = D64_rtrack(self, tracknum);
    if (!track) return 0;
    return Track_rsector(track, sectornum);
}

Sector *D64_sector(D64 *self, uint8_t tracknum, uint8_t sectornum)
{
    Track *track = D64_track(self, tracknum);
    if (!track) return 0;
    return Track_sector(track, sectornum);
}

void D64_destroy(D64 *self)
{
    if (!self) return;
    for (uint8_t tracknum = 0; tracknum < tracks[D64_type(self)]; ++tracknum)
    {
	Track_destroy(self->track[tracknum]);
    }
    free(self);
}

