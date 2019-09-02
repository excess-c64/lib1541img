#include <1541img/d64.h>
#include <1541img/track.h>
#include <1541img/sector.h>

#include "log.h"
#include <1541img/zc45writer.h>

enum method
{
    M_PLAIN = 0,
    M_FILL,
    M_RLE
};

static const size_t filesizes[] =
{
    168*258 + 4,
    168*258 + 2,
    172*258 + 2,
    175*258 + 2,
    85*258 + 2
};

static const uint8_t trackrange[] = { 1, 9, 17, 26, 36, 41 };

static uint8_t interleave(uint8_t trackno)
{
    if (trackno < 18) return 11;
    if (trackno < 25) return 10;
    return 9;
}

static int writebyte(uint8_t *zcfile, size_t *wpos, size_t zcfilelen,
        uint8_t byte)
{
    if (*wpos == zcfilelen) return -1;
    zcfile[(*wpos)++] = byte;
    return 0;
}

static uint8_t nextsectno(uint8_t interlv, uint8_t *sectorflags,
        uint8_t numsectors, uint8_t sectno)
{
    uint8_t pos = (sectno + interlv) % numsectors;
    uint8_t nextsect = pos;
    while (sectorflags[nextsect])
    {
        ++nextsect;
        if (nextsect == numsectors) nextsect = 0;
        if (nextsect == pos) return 0xff;
    }
    sectorflags[nextsect] = 1;
    return nextsect;
}

static enum method getMethod(const uint8_t *data,
        uint8_t *rlelen, uint8_t *repcode)
{
    uint8_t rlesave = 0;
    uint8_t last = *data;
    uint8_t repcnt = 0;
    uint8_t used[256] = { 0 };
    ++used[last];

    for (int i = 1; i < 256; ++i)
    {
        if (data[i] == last)
        {
            if (++repcnt > 2) ++rlesave;
        }
        else
        {
            repcnt = 0;
            last = data[i];
            ++used[last];
        }
    }
    if (repcnt == 255) return M_FILL;
    if (rlesave > 2)
    {
        if (rlelen)
        {
            *rlelen = 256 - rlesave;
        }
        if (repcode)
        {
            for (uint8_t rc = 0;;++rc)
            {
                if (!used[rc])
                {
                    *repcode = rc;
                    break;
                }
            }
        }
        return M_RLE;
    }
    return M_PLAIN;
}

static int writePlain(uint8_t *zcfile, size_t zcfilelen, size_t *wpos,
        const uint8_t *data)
{
    for (int i = 0; i < 256; ++i)
    {
        if (writebyte(zcfile, wpos, zcfilelen, data[i]) < 0) return -1;
    }
    return 0;
}

static int writeRle(uint8_t *zcfile, size_t zcfilelen, size_t *wpos,
        const uint8_t *data, uint8_t rlelen, uint8_t repcode)
{
    if (writebyte(zcfile, wpos, zcfilelen, rlelen) < 0) return -1;
    if (writebyte(zcfile, wpos, zcfilelen, repcode) < 0) return -1;
    for (int i = 0; i < 256; ++i)
    {
        uint8_t b = data[i];
        uint8_t repcnt = 0;
        for (int j = i+1; j < 256; ++j)
        {
            if (data[j] == b) ++repcnt;
            else break;
        }
        if (repcnt > 2)
        {
            if (writebyte(zcfile, wpos, zcfilelen, repcode) < 0) return -1;
            if (writebyte(zcfile, wpos, zcfilelen, repcnt+1) < 0) return -1;
            i += repcnt;
        }
        if (writebyte(zcfile, wpos, zcfilelen, b) < 0) return -1;
    }
    return 0;
}

static int writeSector(uint8_t *zcfile, size_t zcfilelen, size_t *wpos,
        uint8_t trackno, const Track *track, uint8_t sectno)
{
    const uint8_t *data = Sector_rcontent(Track_rsector(track, sectno));
    uint8_t rlelen;
    uint8_t repcode;
    enum method met = getMethod(data, &rlelen, &repcode);
    if (writebyte(zcfile, wpos, zcfilelen, trackno | (met<<6) ) < 0)
    {
        return -1;
    }
    if (writebyte(zcfile, wpos, zcfilelen, sectno) < 0)
    {
        return -1;
    }
    switch (met)
    {
        case M_PLAIN:
            logfmt(L_DEBUG, "zc45_write: processing sector %hhu:%hhu (plain)",
                    trackno, sectno);
            return writePlain(zcfile, zcfilelen, wpos, data);
        case M_FILL:
            logfmt(L_DEBUG, "zc45_write: processing sector %hhu:%hhu (fill)",
                    trackno, sectno);
            return writebyte(zcfile, wpos, zcfilelen, *data);
        case M_RLE:
            logfmt(L_DEBUG, "zc45_write: processing sector %hhu:%hhu (rle)",
                    trackno, sectno);
            return writeRle(zcfile, zcfilelen, wpos, data, rlelen, repcode);
    }
    return -1; // unreachable
}

static int writeTrack(uint8_t *zcfile, size_t zcfilelen, size_t *wpos,
        uint8_t trackno, const D64 *d64)
{
    uint8_t sectorflags[21] = {1,0};
    const Track *track = D64_rtrack(d64, trackno);
    uint8_t numsectors = Track_sectors(track);
    uint8_t sectno = 0;
    uint8_t interlv = interleave(trackno);

    do
    {
        if (writeSector(zcfile, zcfilelen, wpos, trackno, track, sectno) < 0)
        {
            return -1;
        }
    } while ((sectno = nextsectno(interlv, sectorflags,
                    numsectors, sectno)) != 0xff);
    return 0;
}

size_t zc45_write(uint8_t *zcfile, size_t zcfilelen, int zcfileno,
        const D64 *d64)
{
    if (zcfileno < 1 || zcfileno > 5)
    {
        logmsg(L_ERROR, "zc45_write: invalid fileno.");
        return 0;
    }
    if (zcfileno == 5 && D64_type(d64) != D64_40TRACK)
    {
        logmsg(L_ERROR, "zc45_write: fileno 5 invalid for 35-track disk.");
        return 0;
    }
    if (zcfilelen < filesizes[zcfileno-1])
    {
        logmsg(L_WARNING, "zc45_write: file size might be too small to hold "
                "the resulting zipcode.");
    }

    size_t wpos = 0;

    if (zcfileno == 1)
    {
        const uint8_t *bam = Sector_rcontent(D64_rsector(d64, 18, 0));
        if (writebyte(zcfile, &wpos, zcfilelen, 0xfe) < 0) goto fail;
        if (writebyte(zcfile, &wpos, zcfilelen, 0x03) < 0) goto fail;
        if (writebyte(zcfile, &wpos, zcfilelen, bam[0xa2]) < 0) goto fail;
        if (writebyte(zcfile, &wpos, zcfilelen, bam[0xa3]) < 0) goto fail;
    }
    else
    {
        if (writebyte(zcfile, &wpos, zcfilelen, 0x00) < 0) goto fail;
        if (writebyte(zcfile, &wpos, zcfilelen, 0x04) < 0) goto fail;
    }

    for (uint8_t trackno = trackrange[zcfileno-1];
            trackno < trackrange[zcfileno]; ++trackno)
    {
        if (writeTrack(zcfile, zcfilelen, &wpos, trackno, d64) < 0) goto fail;
    }
    return wpos;

fail:
    logmsg(L_ERROR, "zc45_write: not enough space.");
    return 0;
}
