#include <string.h>

#include "util.h"
#include "log.h"
#include "dirdata.h"
#include <1541img/d64.h>
#include <1541img/track.h>
#include <1541img/sector.h>
#include <1541img/filedata.h>
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosvfs.h>

#include "cbmdosvfsreader.h"

static int parseTrackBam(uint8_t *bamdata, uint8_t sectors,
	const uint8_t *trackbam)
{
    uint8_t tmp[21] = {0};
    uint8_t sectfree = 0;
    for (uint8_t sectno = 0; sectno < sectors; ++sectno)
    {
	if (trackbam[1+sectno/8] & (1U << (7 - sectno%8)))
	{
	    ++sectfree;
	}
	else
	{
	    tmp[sectno] = 1;
	}
    }
    memcpy(bamdata, tmp, 21);
    if (sectfree != trackbam[0]) return -1;
    return 0;
}

int readCbmdosVfsInternal(CbmdosVfs *vfs, const D64 *d64,
	uint8_t (*bamdata)[21], DirData *dirdata)
{
    if (CbmdosVfs_fileCount(vfs))
    {
        logmsg(L_ERROR, "readCbmdosVfs: called with non-empty vfs.");
        return -1;
    }

    const uint8_t *bam = Sector_rcontent(D64_rsector(d64, 18, 0));
    CbmdosVfs_setDosver(vfs, bam[2]);
    uint8_t namelen = 16;
    while (namelen && bam[namelen + 0x8f] == 0xa0) --namelen;
    CbmdosVfs_setName(vfs, (const char *)bam+0x90, namelen);
    uint8_t idlen = 5;
    if (bam[0xa4] == 0xa0)
    {
        if (bam[0xa6] == 0x41)
        {
            idlen = 4;
            if (bam[0xa5] == 0x32) idlen = 2;
        }
    }
    CbmdosVfs_setId(vfs, (const char *)bam+0xa2, idlen);

    int rc = 0;
    if (bamdata)
    {
	for (uint8_t trackno = 1; trackno < D64_tracks(d64); ++trackno)
	{
	    uint8_t sectors = Track_sectors(D64_rtrack(d64, trackno));
	    if (trackno < 36)
	    {
		if (parseTrackBam(bamdata[trackno-1], sectors,
			    bam + 4*trackno) < 0)
		{
		    rc = -2;
		}
	    }
	    else
	    {
		int trc = parseTrackBam(bamdata[trackno-1], sectors,
			bam + 0x1c + 4*trackno);
		if (parseTrackBam(bamdata[trackno-1], sectors,
			bam + 0x30 + 4*trackno) < 0 && trc < 0)
		{
		    rc = -2;
		}
	    }
	}
    }

    const Sector *dirsect = D64_rsector(d64, 18, 1);

    while (dirsect)
    {
        const uint8_t *dirbytes = Sector_rcontent(dirsect);
        for (uint8_t dirpos = 0; dirpos < 8; ++dirpos)
        {
            const uint8_t *direntry = dirbytes + dirpos * 0x20;
            if (direntry[2])
            {
                CbmdosFileType type = direntry[2] & 0xf;
                if (type == CFT_REL) continue; // TODO: REL files
                int locked = !!(direntry[2] & (1<<6));
                int closed = !!(direntry[2] & (1<<7));
                CbmdosFile *file = CbmdosFile_create();
                if (CbmdosFile_setType(file, type) < 0)
                {
                    CbmdosFile_destroy(file);
                    continue;
                }
                CbmdosFile_setLocked(file, locked);
                CbmdosFile_setClosed(file, closed);
                uint8_t filenamelen = 16;
                while (filenamelen && direntry[filenamelen + 4] == 0xa0)
                    --filenamelen;
                CbmdosFile_setName(file, (const char *)direntry+5, filenamelen);
                logfmt(L_DEBUG, "readCbmdosVfs: found file \"%16s\" %s",
                        CbmdosFile_name(file, 0), CbmdosFileType_name(type));

		uint16_t blocks = (direntry[0x1f] << 8) | direntry[0x1e];
		if (dirdata)
		{
		    if (dirdata->size == dirdata->capa)
		    {
			dirdata->capa += DIRCHUNK;
			dirdata->entries = xrealloc(dirdata->entries,
				dirdata->capa * sizeof *dirdata->entries);
		    }
		    dirdata->entries[dirdata->size].starttrack = direntry[3];
		    dirdata->entries[dirdata->size].startsector = direntry[4];
		    dirdata->entries[dirdata->size].blocks = blocks;
		    ++dirdata->size;
		}

		if (type != CFT_DEL)
		{
		    FileData *data = CbmdosFile_data(file);
		    uint8_t track = direntry[3];
		    uint8_t sector = direntry[4];
		    while (track)
		    {
			const Sector *filesector = D64_rsector(
				d64, track, sector);
			if (!filesector)
			{
			    logmsg(L_ERROR,
				    "readCbmdosVfs: corrupt filesystem.");
			    CbmdosFile_destroy(file);
			    file = 0;
			    break;
			}
			const uint8_t *sectorbytes = Sector_rcontent(
				D64_rsector(d64, track, sector));
			track = sectorbytes[0];
			sector = sectorbytes[1];
			size_t appendsize = 254;
			if (!track) appendsize = sector-1;
			if (FileData_append(data, sectorbytes+2,
				    appendsize) < 0)
			{
			    logmsg(L_ERROR, "readCbmdosVfs: error appending "
				    "to file.");
			    CbmdosFile_destroy(file);
			    file = 0;
			    break;
			}
		    }
		}
                if (file)
                {
		    if (CbmdosFile_realBlocks(file) != blocks)
		    {
			CbmdosFile_setForcedBlocks(file, blocks);
		    }
                    CbmdosVfs_append(vfs, file);
                }
            }
        }
        if (dirbytes[0])
        {
            dirsect = D64_rsector(d64, dirbytes[0], dirbytes[1]);
            if (!dirsect)
            {
                logmsg(L_ERROR, "readCbmdosVfs: corrupt filesystem.");
		rc = -1;
            }
        }
        else dirsect = 0;
    }

    return rc;
}

int readCbmdosVfs(CbmdosVfs *vfs, const D64 *d64)
{
    return readCbmdosVfsInternal(vfs, d64, 0, 0);
}
