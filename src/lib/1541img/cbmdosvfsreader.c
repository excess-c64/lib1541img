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
#include <1541img/cbmdosfs.h>

#include "cbmdosvfsreader.h"

static int parseTrackBam(uint8_t *bamdata, uint8_t sectors,
	const uint8_t *trackbam)
{
    uint8_t tmp[21] = {0};
    uint8_t sectfree = 0;
    for (uint8_t sectno = 0; sectno < sectors; ++sectno)
    {
	if (trackbam[1+sectno/8] & (1U << (sectno%8)))
	{
	    ++sectfree;
	}
	else
	{
	    tmp[sectno] = 1;
	}
    }
    if (bamdata) memcpy(bamdata, tmp, 21);
    if (sectfree != trackbam[0]) return -1;
    return 0;
}

SOLOCAL int readCbmdosVfsInternal(CbmdosVfs *vfs, const D64 *d64,
	const CbmdosFsOptions *options,
	uint8_t (*bamdata)[21], DirData *dirdata)
{
    if (CbmdosVfs_fileCount(vfs))
    {
        logmsg(L_ERROR, "readCbmdosVfs: called with non-empty vfs.");
        return -1;
    }

    const uint8_t *bam = Sector_rcontent(D64_rsector(d64, 18, 0));
    CbmdosVfs_setDosver(vfs, bam[2]);
    uint8_t nameoffset = 0;
    uint8_t defaultver = 0x41;
    if (options->flags & CFF_PROLOGICDOSBAM)
    {
	nameoffset = 0x14;
	defaultver = 0x50;
    }
    uint8_t namelen = 16;
    while (namelen && bam[namelen + 0x8f + nameoffset] == 0xa0) --namelen;
    CbmdosVfs_setName(vfs, (const char *)bam+0x90+nameoffset, namelen);
    uint8_t idlen = 5;
    if (bam[0xa6+nameoffset] == defaultver)
    {
	--idlen;
	if (bam[0xa5+nameoffset] == 0x32)
	{
	    --idlen;
	    if (bam[0xa4+nameoffset] == 0xa0) --idlen;
	}
    }
    CbmdosVfs_setId(vfs, (const char *)bam+0xa2+nameoffset, idlen);

    int rc = 0;
    if (bamdata)
    {
	for (uint8_t trackno = 1; trackno <= D64_tracks(d64); ++trackno)
	{
	    uint8_t sectors = Track_sectors(D64_rtrack(d64, trackno));
	    if (trackno < 36)
	    {
		if (parseTrackBam(bamdata[trackno-1], sectors,
			    bam + 4*trackno) < 0)
		{
		    if (!(options->flags & CFF_ZEROFREE)) rc = -2;
		}
	    }
	    else if ((options->flags & (CFF_40TRACK | CFF_42TRACK))
		    && (trackno < 41))
	    {
		if (options->flags & CFF_DOLPHINDOSBAM)
		{
		    if (parseTrackBam(bamdata[trackno-1], sectors,
				bam + 0x1c + 4*trackno) < 0)
		    {
			if (!(options->flags & CFF_ZEROFREE)) rc = -2;
		    }
		}
		if (options->flags & CFF_SPEEDDOSBAM)
		{
		    if (parseTrackBam(bamdata[trackno-1], sectors,
				bam + 0x30 + 4*trackno) < 0)
		    {
			if (!(options->flags & CFF_ZEROFREE)) rc = -2;
		    }
		}
		if (options->flags & CFF_PROLOGICDOSBAM)
		{
		    if (parseTrackBam(bamdata[trackno-1], sectors,
				bam + 4*trackno) < 0)
		    {
			if (!(options->flags & CFF_ZEROFREE)) rc = -2;
		    }
		}
	    }
	}
    }

    uint8_t maxtrack = 35;
    if (options->flags & CFF_40TRACK) maxtrack = 40;
    if (options->flags & CFF_42TRACK) maxtrack = 42;

    const Sector *dirsect = D64_rsector(d64, 18, 1);

    uint8_t rdmap[42][21] = { { 0 } };
    rdmap[17][0] = 1;
    rdmap[17][1] = 1;

    while (dirsect)
    {
        const uint8_t *dirbytes = Sector_rcontent(dirsect);
        for (uint8_t dirpos = 0; dirpos < 8; ++dirpos)
        {
            const uint8_t *direntry = dirbytes + dirpos * 0x20;
            if (direntry[2])
            {
                CbmdosFileType type = direntry[2] & 0xf;
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
                if (type == CFT_REL)
                {
                    if (CbmdosFile_setRecordLength(file, direntry[0x17]) < 0)
                    {
                        CbmdosFile_destroy(file);
                        continue;
                    }
                }
                uint8_t filenamelen = 16;
                while (filenamelen && direntry[filenamelen + 4] == 0xa0)
                    --filenamelen;
                CbmdosFile_setName(file, (const char *)direntry+5, filenamelen);
                logfmt(L_DEBUG, "readCbmdosVfs: found file \"%s\" %s",
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
                    if (type == CFT_DEL)
                    {
		        dirdata->entries[dirdata->size].starttrack = 0;
		        dirdata->entries[dirdata->size].startsector = 0;
                    }
                    else
                    {
		        dirdata->entries[dirdata->size].starttrack
                            = direntry[3];
		        dirdata->entries[dirdata->size].startsector
                            = direntry[4];
                    }
		    dirdata->entries[dirdata->size].blocks = blocks;
                    if (type == CFT_REL)
                    {
                        dirdata->entries[dirdata->size].sidetrack
                            = direntry[0x15];
                        dirdata->entries[dirdata->size].sidesector
                            = direntry[0x16];
                    }
                    else
                    {
                        dirdata->entries[dirdata->size].sidetrack = 0;
                        dirdata->entries[dirdata->size].sidesector = 0;
                    }
		    ++dirdata->size;
		}

		if (type != CFT_DEL)
		{
		    FileData *data = CbmdosFile_data(file);
		    uint8_t track = direntry[3];
		    uint8_t sector = direntry[4];
                    int doingsidesects = 0;
		    while (track)
		    {
			if (track > maxtrack)
			{
			    logfmt(L_ERROR, "readCbmdosVfs: invalid track "
				    "%d found.", track);
			    CbmdosFile_destroy(file);
			    file = 0;
			    if (options->flags & CFF_RECOVER) goto nextfile;
			    dirsect = 0;
			    rc = -1;
			    break;
			}
			if (!(options->flags & CFF_FILESONDIRTRACK)
				&& track == 18)
			{
			    logmsg(L_ERROR, "readCbmdosVfs: unexpectedly "
				    "found file on directory track.");
			    CbmdosFile_destroy(file);
			    file = 0;
			    if (options->flags & CFF_RECOVER) goto nextfile;
			    dirsect = 0;
			    rc = -1;
			    break;
			}
			const Sector *filesector = D64_rsector(
				d64, track, sector);
			if (!filesector)
			{
			    logmsg(L_ERROR,
				    "readCbmdosVfs: corrupt filesystem.");
			    CbmdosFile_destroy(file);
			    file = 0;
			    if (options->flags & CFF_RECOVER) goto nextfile;
                            dirsect = 0;
			    rc = -1;
			    break;
			}
                        if (rdmap[track-1][sector])
                        {
                            logmsg(L_ERROR,
                                    "readCbmdosVfs: corrupt filesystem.");
                            CbmdosFile_destroy(file);
                            file = 0;
			    if (options->flags & CFF_RECOVER) goto nextfile;
                            dirsect = 0;
			    rc = -1;
                            break;
                        }
                        rdmap[track-1][sector] = 1;
			if (track > 40 && bamdata)
			{
			    bamdata[track-1][sector] = 1;
			}
			const uint8_t *sectorbytes = Sector_rcontent(
				D64_rsector(d64, track, sector));
			track = sectorbytes[0];
			sector = sectorbytes[1];
                        if (!doingsidesects)
                        {
                            size_t appendsize = 254;
                            if (!track) appendsize = sector-1;
                            if (FileData_append(data, sectorbytes+2,
                                        appendsize) < 0)
                            {
                                logmsg(L_ERROR, "readCbmdosVfs: error "
                                        "appending to file.");
                                CbmdosFile_destroy(file);
                                file = 0;
                                dirsect = 0;
                                rc = -1;
                                break;
                            }
                            if (!track && type == CFT_REL)
                            {
                                doingsidesects = 1;
                                track = direntry[0x15];
                                sector = direntry[0x16];
                            }
                        }
		    }
		}
nextfile:	if (file)
                {
		    if (CbmdosFile_realBlocks(file) != blocks)
		    {
			CbmdosFile_setForcedBlocks(file, blocks);
		    }
                    CbmdosVfs_append(vfs, file);
                }
		else if (dirdata)
		{
		    --dirdata->size;
		}
            }
        }
        if (dirsect && dirbytes[0])
        {
            dirsect = D64_rsector(d64, dirbytes[0], dirbytes[1]);
            if (!dirsect)
            {
                logmsg(L_ERROR, "readCbmdosVfs: corrupt filesystem.");
		if (options->flags & CFF_RECOVER) goto done;
		rc = -1;
            }
	    else if (rdmap[dirbytes[0]-1][dirbytes[1]])
            {
                logmsg(L_ERROR, "readCbmdosVfs: corrupt filesystem.");
		if (!(options->flags & CFF_RECOVER))
		{
		    rc = -1;
		    dirsect = 0;
		}
            }
	    else if (!(options->flags & CFF_ALLOWLONGDIR) && dirbytes[0] != 18)
	    {
		logmsg(L_ERROR, "readCbmdosVfs: unexpectedly found long "
			"directory.");
		if (options->flags & CFF_RECOVER) goto done;
		rc = -1;
		dirsect = 0;
	    }
	    else if (dirbytes[0] > maxtrack)
	    {
		logfmt(L_ERROR, "readCbmdosVfs: invalid track %d found.",
			dirbytes[0]);
		if (options->flags & CFF_RECOVER) goto done;
		rc = -1;
		dirsect = 0;
	    }
	    if (rc == 0)
	    {
		rdmap[dirbytes[0]-1][dirbytes[1]] = 1;
		if (dirbytes[0] > 40 && bamdata)
		{
		    bamdata[dirbytes[0]-1][dirbytes[1]] = 1;
		}
	    }
        }
        else dirsect = 0;
    }

done:
    if (rc == 0 && bamdata)
    {
	for (uint8_t track = 0; track < maxtrack; ++track)
	    for (uint8_t sect = 0; sect < 21; ++sect)
		if (bamdata[track][sect] != rdmap[track][sect])
		    return -2;
    }

    return rc;
}

SOEXPORT int readCbmdosVfs(CbmdosVfs *vfs, const D64 *d64,
	const CbmdosFsOptions *options)
{
    CbmdosFsOptions probeopts = CFO_DEFAULT;
    if (!options)
    {
	if (probeCbmdosFsOptions(&probeopts, d64) < 0) return -1;
	options = &probeopts;
    }
    return readCbmdosVfsInternal(vfs, d64, options, 0, 0);
}

SOEXPORT int probeCbmdosFsOptions(CbmdosFsOptions *options, const D64 *d64)
{
    CbmdosFsOptions probeopts = CFO_DEFAULT;
    if (options->flags & CFF_RECOVER)
    {
	probeopts.flags |= CFF_RECOVER;
    }

    uint8_t bamprobedolphin[5][21] = {{ 0 }};
    uint8_t bamprobespeed[5][21] = {{ 0 }};

    const uint8_t *bam = Sector_rcontent(D64_rsector(d64, 18, 0));
    if (D64_tracks(d64) > 35)
    {
	if (bam[2] == 0x50)
	{
	    probeopts.flags |= CFF_PROLOGICDOSBAM;
	    for (uint8_t trackno = 36; trackno < 41; ++trackno)
	    {
		uint8_t sectors = Track_sectors(D64_rtrack(d64, trackno));
		parseTrackBam(bamprobespeed[trackno-36],
			sectors, bam + 4*trackno);
	    }
	}
	else
	{
	    int dolphinok = 1;
	    int speedok = 1;
	    for (uint8_t trackno = 36; trackno < 41; ++trackno)
	    {
		uint8_t sectors = Track_sectors(D64_rtrack(d64, trackno));
		if (parseTrackBam(bamprobedolphin[trackno-36],
			    sectors, bam + 0x1c + 4*trackno) < 0)
		{
		    dolphinok = 0;
		}
		if (parseTrackBam(bamprobespeed[trackno-36],
			    sectors, bam + 0x30 + 4*trackno) < 0)
		{
		    speedok = 0;
		}
	    }
	    if (dolphinok)
	    {
		probeopts.flags |= CFF_DOLPHINDOSBAM;
	    }
	    if (speedok)
	    {
		probeopts.flags |= CFF_SPEEDDOSBAM;
	    }
	}
    }

    const Sector *dirsect = D64_rsector(d64, 18, 1);

    uint8_t rdmap[42][21] = { { 0 } };
    rdmap[17][0] = 1;
    rdmap[17][1] = 1;

    while (dirsect)
    {
        const uint8_t *dirbytes = Sector_rcontent(dirsect);
        for (uint8_t dirpos = 0; dirpos < 8; ++dirpos)
        {
            const uint8_t *direntry = dirbytes + dirpos * 0x20;
            if (direntry[2])
            {
                CbmdosFileType type = direntry[2] & 0xf;
		if (type < CFT_DEL || type > CFT_REL)
		{
		    logmsg(L_ERROR, "probeCbmdosFsOptions: invalid file type "
			    "found.");
		    if (probeopts.flags & CFF_RECOVER) goto nextfile;
		    return -1;
		}
		if (type != CFT_DEL)
		{
		    uint8_t track = direntry[3];
		    uint8_t sector = direntry[4];
                    int doingsidesects = 0;
		    while (track)
		    {
			if (track > 40)
			{
			    probeopts.flags &= ~CFF_40TRACK;
			    probeopts.flags |= CFF_42TRACK;
			}
			else if (track > 35 && !(probeopts.flags & CFF_42TRACK))
			{
			    probeopts.flags |= CFF_40TRACK;
			}
			else if (track == 18)
			{
			    probeopts.flags |= CFF_FILESONDIRTRACK;
			}
			const Sector *filesector = D64_rsector(
				d64, track, sector);
			if (!filesector)
			{
			    logfmt(L_ERROR, "probeCbmdosFsOptions: "
				    "non-existent sector %d/%d found.",
				    track, sector);
			    if (probeopts.flags & CFF_RECOVER) goto nextfile;
			    return -1;
			}
                        if (rdmap[track-1][sector])
                        {
			    logmsg(L_ERROR, "probeCbmdosFsOptions: "
				    "corrupt filesystem, sector used twice.");
			    if (probeopts.flags & CFF_RECOVER) goto nextfile;
			    return -1;
                        }
                        rdmap[track-1][sector] = 1;
			const uint8_t *sectorbytes = Sector_rcontent(
				D64_rsector(d64, track, sector));
			track = sectorbytes[0];
			sector = sectorbytes[1];
                        if (!track && !doingsidesects && type == CFT_REL)
                        {
                            doingsidesects = 1;
                            track = direntry[0x15];
                            sector = direntry[0x16];
                        }
		    }
		}
nextfile:
		;
            }
        }
        if (dirbytes[0])
        {
            dirsect = D64_rsector(d64, dirbytes[0], dirbytes[1]);
            if (!dirsect)
            {
                logfmt(L_ERROR, "probeCbmdosFsOptions: Non-existent sector "
			"%d/%d found.", dirbytes[0], dirbytes[1]);
		if (probeopts.flags & CFF_RECOVER) goto done;
		return -1;
            }
	    if (rdmap[dirbytes[0]-1][dirbytes[1]])
            {
                logmsg(L_ERROR, "probeCbmdosFsOptions: corrupt filesystem, "
			"sector used twice.");
                if (!(probeopts.flags & CFF_RECOVER)) return -1;
            }
	    if (dirbytes[0] > 40)
	    {
		probeopts.flags &= ~CFF_40TRACK;
		probeopts.flags |= CFF_42TRACK;
	    }
	    else if (dirbytes[0] > 35 && !(probeopts.flags & CFF_42TRACK))
	    {
		probeopts.flags |= CFF_40TRACK;
	    }
	    else if (dirbytes[0] != 18)
	    {
		probeopts.flags |= CFF_ALLOWLONGDIR;
	    }
	    rdmap[dirbytes[0]-1][dirbytes[1]] = 1;
        }
        else dirsect = 0;
    }

    if ((probeopts.flags & CFF_DOLPHINDOSBAM)
	    && memcmp(rdmap+35, bamprobedolphin, sizeof bamprobedolphin))
    {
	probeopts.flags &= ~CFF_DOLPHINDOSBAM;
    }
    if ((probeopts.flags & CFF_SPEEDDOSBAM)
	    && memcmp(rdmap+35, bamprobespeed, sizeof bamprobespeed))
    {
	probeopts.flags &= ~CFF_SPEEDDOSBAM;
    }
    if ((probeopts.flags & CFF_PROLOGICDOSBAM)
	    && memcmp(rdmap+35, bamprobespeed, sizeof bamprobespeed))
    {
	probeopts.flags &= ~CFF_PROLOGICDOSBAM;
    }

    int hasfreeblocks = 0;
    for (uint8_t trackno = 1;
	    trackno < (D64_type(d64) == D64_STANDARD ? 36 : 41); ++trackno)
    {
	uint8_t sectors = Track_sectors(D64_rtrack(d64, trackno));
	for (uint8_t sectno = 0; sectno < sectors; ++sectno)
	{
	    if (!rdmap[trackno-1][sectno])
	    {
		hasfreeblocks = 1;
		goto checkzerofree;
	    }
	}
    }

checkzerofree:
    if (hasfreeblocks)
    {
	int zerofree = 1;
	for (uint8_t trackno = 1; trackno < 36; ++trackno)
	{
	    if (bam[4*trackno])
	    {
		zerofree = 0;
		break;
	    }
	}

	if (D64_type(d64) == D64_STANDARD)
	{
	    if (zerofree) probeopts.flags |= CFF_ZEROFREE;
	}
	else if (zerofree)
	{
	    if (probeopts.flags & CFF_PROLOGICDOSBAM)
	    {
		for (uint8_t trackno = 36; trackno < 41; ++trackno)
		{
		    if (bam[4*trackno])
		    {
			zerofree = 0;
			break;
		    }
		}
		if (zerofree) probeopts.flags |= CFF_ZEROFREE;
	    }
	    else
	    {
		int havedolphin = 0;
		int havespeed = 0;
		if (!memcmp(rdmap+35, bamprobedolphin,
			sizeof bamprobedolphin))
		{
		    havedolphin = 1;
		    for (uint8_t trackno = 36; trackno < 41; ++trackno)
		    {
			if (bam[4*trackno + 0x1c])
			{
			    zerofree = 0;
			    break;
			}
		    }
		}
		if (!memcmp(rdmap+35, bamprobespeed,
			    sizeof bamprobespeed))
		{
		    havespeed = 1;
		    for (uint8_t trackno = 36; trackno < 41; ++trackno)
		    {
			if (bam[4*trackno + 0x30])
			{
			    zerofree = 0;
			    break;
			}
		    }
		}
		if (zerofree)
		{
		    probeopts.flags |= CFF_ZEROFREE;
		    if (havedolphin) probeopts.flags |= CFF_DOLPHINDOSBAM;
		    if (havespeed) probeopts.flags |= CFF_SPEEDDOSBAM;
		}
	    }
	}
    }

done:
    memcpy(options, &probeopts, sizeof probeopts);
    return 0;
}

