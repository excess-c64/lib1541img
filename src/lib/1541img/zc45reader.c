#include <1541img/d64.h>
#include <1541img/sector.h>

#include "log.h"
#include <1541img/zc45reader.h>

static int nextbyte(const uint8_t *zcfile, size_t *pos, size_t zcfilelen)
{
    if (*pos == zcfilelen) return -1;
    return zcfile[(*pos)++];
}

static int decodeplain(uint8_t *data, const uint8_t *zcfile,
	size_t *pos, size_t zcfilelen)
{
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
	int byte = nextbyte(zcfile, pos, zcfilelen);
	if (byte < 0)
        {
            logmsg(L_ERROR, "zc45_read: unexpected end of file in verbatim "
                    "sector.");
            return -1;
        }
	data[i] = byte;
    }
    return 0;
}

static int decodefill(uint8_t *data, const uint8_t *zcfile,
	size_t *pos, size_t zcfilelen)
{
    int fill = nextbyte(zcfile, pos, zcfilelen);
    if (fill < 0)
    {
        logmsg(L_ERROR, "zc45_read: unexpected end of file in fill sector.");
        return -1;
    }
    for (int i = 0; i < SECTOR_SIZE; ++i)
    {
	data[i] = fill;
    }
    return 0;
}

static int decoderle(uint8_t *data, const uint8_t *zcfile,
	size_t *pos, size_t zcfilelen)
{
    int i = 0;
    int len = nextbyte(zcfile, pos, zcfilelen);
    if (len < 0)
    {
        logmsg(L_ERROR, "zc45_read: unexpected end of file trying to read "
                "length of RLE encoded sector.");
        return -1;
    }
    int repeat = nextbyte(zcfile, pos, zcfilelen);
    if (repeat < 0)
    {
        logmsg(L_ERROR, "zc45_read: unexpected end of file trying to read "
                "repeat marker of RLE encoded sector.");
        return -1;
    }
    while (i < SECTOR_SIZE)
    {
	int byte = nextbyte(zcfile, pos, zcfilelen);
	if (byte < 0)
        {
            logmsg(L_ERROR, "zc45_read: unexpected end of file in RLE encoded "
                    "sector.");
            return -1;
        }
	if (byte == repeat)
	{
	    int repcount = nextbyte(zcfile, pos, zcfilelen);
	    if (repcount < 0)
            {
                logmsg(L_ERROR, "zc45_read: unexpected end of file trying to "
                        "read repeat count of sequence.");
                return -1;
            }
	    int repbyte = nextbyte(zcfile, pos, zcfilelen);
	    if (repbyte < 0)
            {
                logmsg(L_ERROR, "zc45_read: unexpected end of file trying to "
                        "read fill value of sequence.");
                return -1;
            }
	    for (int j = 0; j < repcount; ++j)
	    {
		data[i++] = repbyte;
		if (i == SECTOR_SIZE && j+1 < repcount)
                {
                    logmsg(L_WARNING, "zc45_read: end of sector reached while "
                            "filling sequence in RLE encoded sector.");
                    break;
                }
	    }
	}
	else
	{
	    data[i++] = byte;
	}
    }
    return 0;
}
static int decodesector(D64 *d64, const uint8_t *zcfile,
	size_t *pos, size_t zcfilelen)
{
    int control = nextbyte(zcfile, pos, zcfilelen);
    if (control < 0)
    {
        logmsg(L_ERROR, "zc45_read: unexpected end of file trying to read "
                "next track number.");
        return -1;
    }
    uint8_t tracknum = control & 0x3f;
    int sectornum = nextbyte(zcfile, pos, zcfilelen);
    if (sectornum < 0)
    {
        logmsg(L_ERROR, "zc45_read: unexpected end of file trying to read "
                "next sector number.");
        return -1;
    }
    logsetsilent(1);
    Sector *sector = D64_sector(d64, tracknum, sectornum);
    logsetsilent(0);
    if (!sector)
    {
	while (sectornum == tracknum)
	{
	    int next = nextbyte(zcfile, pos, zcfilelen);
	    if (next < 0)
            {
                logfmt(L_INFO, "zc45_read: ignoring extra $%02hhx bytes at "
                        "end of file.", tracknum);
                return -2;
            }
	    if (next != sectornum) break;
	}
        logfmt(L_ERROR, "zc45_read: Invalid sector %hhu:%hhu found.",
                tracknum, sectornum);
	return -1;
    }
    uint8_t *data = Sector_content(sector);

    switch ((unsigned)control >> 6)
    {
	case 0:
            logfmt(L_DEBUG, "zc45_read: processing sector %hhu:%hhu (plain)",
                    tracknum, sectornum);
	    return decodeplain(data, zcfile, pos, zcfilelen);
	case 1:
            logfmt(L_DEBUG, "zc45_read: processing sector %hhu:%hhu (fill)",
                    tracknum, sectornum);
	    return decodefill(data, zcfile, pos, zcfilelen);
	case 2:
            logfmt(L_DEBUG, "zc45_read: processing sector %hhu:%hhu (rle)",
                    tracknum, sectornum);
	    return decoderle(data, zcfile, pos, zcfilelen);
	default:
            logfmt(L_ERROR, "zc45_read: invalid encoding for sector %hhu:%hhu.",
                    tracknum, sectornum);
	    return -1;
    }
}

int zc45_read(D64 *d64, const uint8_t *zcfile, size_t zcfilelen)
{
    if (zcfilelen < 5)
    {
        logfmt(L_ERROR, "zc45_read: input file too short.");
        return -1;
    }
    uint16_t loadaddr = zcfile[0] | (zcfile[1]<<8);
    size_t pos;
    switch (loadaddr)
    {
	case 0x400:
	    pos = 2;
	    break;
	case 0x3fe:
	    pos = 4;
	    break;
	default:
            logfmt(L_ERROR, "zc45_read: not a valid zipcode file.");
	    return -1;
    }
    int sectors = 0;
    while (pos < zcfilelen)
    {
	int rc;
	if ((rc = decodesector(d64, zcfile, &pos, zcfilelen)) < 0)
	{
	    if (rc == -2) break;
	    return -1;
	}
	++sectors;
    }
    return sectors;
}

