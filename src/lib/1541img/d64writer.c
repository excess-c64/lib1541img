#include <1541img/d64.h>
#include <1541img/track.h>
#include <1541img/sector.h>
#include "log.h"

#include <1541img/d64writer.h>

int writeD64(FILE *file, const D64 *d64)
{
    for (uint8_t tracknum = 1; tracknum <= D64_tracks(d64); ++tracknum)
    {
	const Track *track = D64_rtrack(d64, tracknum);
	for (uint8_t sectnum = 0; sectnum < Track_sectors(track); ++sectnum)
	{
	    const Sector *sector = Track_rsector(track, sectnum);
	    const uint8_t *content = Sector_rcontent(sector);
	    if (!fwrite(content, SECTOR_SIZE, 1, file))
            {
                logmsg(L_ERROR, "writeD64: unknown write error.");
                return -1;
            }
	}
    }
    logmsg(L_DEBUG, "writeD64: success.");
    return 0;
}

