#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include "dirdata.h"
#include "cbmdosvfsreader.h"
#include <1541img/d64.h>
#include <1541img/track.h>
#include <1541img/sector.h>
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosvfseventargs.h>
#include <1541img/filedata.h>
#include <1541img/event.h>

#include <1541img/cbmdosfs.h>

const CbmdosFsOptions CFO_DEFAULT = {
    .flags = CFF_COMPATIBLE,
    .dirInterleave = 3,
    .fileInterleave = 10
};

struct CbmdosFs
{
    D64 *d64;
    CbmdosVfs *vfs;
    DirData dir;
    CbmdosFsStatus status;
    CbmdosFsOptions options;
    uint8_t bam[42][21];
};

static void createTrackBam(CbmdosFs *self, uint8_t *tbam, uint8_t trackno)
{
    uint8_t sectors = Track_sectors(D64_track(self->d64, trackno));
    tbam[0] = sectors;
    tbam[1] = 0xff;
    tbam[2] = 0xff;
    tbam[3] = 0xff >> (24 - sectors);
    for (uint8_t sectno = 0; sectno < sectors; ++sectno)
    {
        if (self->bam[trackno-1][sectno])
        {
            --tbam[0];
            tbam[1+sectno/8] ^= (1U << (sectno%8));
        }
    }
    if (self->options.flags & CFF_ZEROFREE)
    {
	tbam[0] = 0;
    }
}

static void updateBam(CbmdosFs *self)
{
    uint8_t *bam = Sector_content(D64_sector(self->d64, 18, 0));
    memset(bam, 0, 256);
    bam[0] = 18;
    bam[1] = 1;
    bam[2] = CbmdosVfs_dosver(self->vfs);
    for (uint8_t trackno = 1; trackno <= D64_tracks(self->d64); ++trackno)
    {
        if (trackno < 36)
        {
            createTrackBam(self, bam + 4*trackno, trackno);
        }
        else if (trackno < 41)
        {
            if (self->options.flags & CFF_DOLPHINDOSBAM)
            {
                createTrackBam(self, bam + 0x1c + 4*trackno, trackno);
            }
            if (self->options.flags & CFF_SPEEDDOSBAM)
            {
                createTrackBam(self, bam + 0x30 + 4*trackno, trackno);
            }
            if (self->options.flags & CFF_PROLOGICDOSBAM)
            {
                createTrackBam(self, bam + 4*trackno, trackno);
            }
        }
    }
    uint8_t nameoffset = 0;
    if (self->options.flags & CFF_PROLOGICDOSBAM) nameoffset = 0x14;
    memset(bam+0x90+nameoffset, 0xa0, 0x1b);
    bam[0xa5+nameoffset] = 0x32;
    bam[0xa6+nameoffset] = CbmdosVfs_dosver(self->vfs);
    uint8_t length;
    const char *name = CbmdosVfs_name(self->vfs, &length);
    memcpy(bam+0x90+nameoffset, name, length);
    const char *id = CbmdosVfs_id(self->vfs, &length);
    memcpy(bam+0xa2+nameoffset, id, length);
}

static void deleteChain(CbmdosFs *self, uint8_t nexttrack, uint8_t nextsect)
{
    if (self->bam[nexttrack-1][nextsect] != 1) return;
    do
    {
        if (nexttrack == 18 && nextsect == 0)
        {
            logmsg(L_WARNING, "CbmdosFs: refusing to delete BAM at [18:0]");
            return;
        }
        uint8_t *dir = Sector_content(
                D64_sector(self->d64, nexttrack, nextsect));
        self->bam[nexttrack-1][nextsect] = 0;
        nexttrack = dir[0];
        nextsect = dir[1];
    } while (nexttrack && self->bam[nexttrack-1][nextsect] == 1);
}

static uint8_t freeSectorOnTrack(CbmdosFs *self, uint8_t trackno,
        uint8_t sectno, uint8_t interlv, int simpleInterlv)
{
    uint8_t sectors = Track_sectors(D64_track(self->d64, trackno));
    uint8_t nextsect = 0;
    if (interlv)
    {
	if (simpleInterlv)
	{
	    nextsect = (sectno + interlv) % sectors;
	}
	else
	{
	    nextsect = sectno + interlv;
	    if (nextsect >= sectors)
	    {
		nextsect -= sectors;
		if (nextsect) --nextsect;
	    }
	}
    }
    uint8_t tmp = nextsect;
    while (self->bam[trackno-1][nextsect])
    {
        ++nextsect;
        if (nextsect == sectors) nextsect = 0;
        if (nextsect == tmp) break;
    }
    return self->bam[trackno-1][nextsect] ? 0xff : nextsect;
}

static uint8_t nextTrack(const CbmdosFsOptions *opts, uint8_t trackno)
{
    if (opts->flags & CFF_TALLOC_TRACKLOAD)
    {
	if (trackno >= 40 && !(opts->flags & CFF_42TRACK))
	{
	    return 0;
	}
	if (trackno >= 35 &&
		!(opts->flags & (CFF_40TRACK | CFF_42TRACK)))
	{
	    return 0;
	}
	if (trackno == 42) return 0;
	if (trackno == 17)
	{
	    if ((opts->flags & CFF_FILESONDIRTRACK) &&
		    !(opts->flags & CFF_TALLOC_PREFDIRTRACK))
	    {
		return 18;
	    }
	    return 19;
	}
	if (trackno == 18
		&& (opts->flags & CFF_FILESONDIRTRACK)
		&& (opts->flags & CFF_TALLOC_PREFDIRTRACK))
	{
	    return 1;
	}
	return trackno + 1;
    }
    else if (opts->flags & CFF_TALLOC_SIMPLE)
    {
	if (trackno == 42)
	{
	    if ((opts->flags & CFF_FILESONDIRTRACK) &&
		    !(opts->flags & CFF_TALLOC_PREFDIRTRACK))
	    {
		return 18;
	    }
	    return 0;
	}
	if (trackno == 40 || (trackno == 35 && !(opts->flags &
			(CFF_40TRACK | CFF_42TRACK))))
	{
	    return 1;
	}
	if (trackno == 17)
	{
	    if (opts->flags & CFF_42TRACK)
	    {
		return 41;
	    }
	    if ((opts->flags & CFF_FILESONDIRTRACK) &&
		    !(opts->flags & CFF_TALLOC_PREFDIRTRACK))
	    {
		return 18;
	    }
	    return 0;
	}
	if (trackno == 18)
	{
	    if ((opts->flags & CFF_FILESONDIRTRACK)
		    && (opts->flags & CFF_TALLOC_PREFDIRTRACK))
	    {
		return 19;
	    }
	    return 0;
	}
	return trackno + 1;
    }
    else
    {
	if (trackno >= 40 && !(opts->flags & CFF_42TRACK))
	{
	    return 0;
	}
	if (trackno >= 35 &&
		!(opts->flags & (CFF_40TRACK | CFF_42TRACK)))
	{
	    return 0;
	}
	if (trackno == 42) return 0;
	if (trackno == 1) return 0;
	if (trackno == 18)
	{
	    if ((opts->flags & CFF_FILESONDIRTRACK)
		    && (opts->flags & CFF_TALLOC_PREFDIRTRACK))
	    {
		return 17;
	    }
	    return 0;
	}
	if (trackno < 18) return trackno - 1;
	return trackno + 1;
    }
}

static uint8_t startTrack(const CbmdosFsOptions *opts, uint8_t trackno)
{
    if (!trackno)
    {
	if ((opts->flags & CFF_FILESONDIRTRACK)
		&& (opts->flags & CFF_TALLOC_PREFDIRTRACK))
	{
	    return 18;
	}
	if (opts->flags & CFF_TALLOC_TRACKLOAD)
	{
	    return 1;
	}
	if (opts->flags & CFF_TALLOC_SIMPLE)
	{
	    return 19;
	}
	return 17;
    }
    if (opts->flags & (CFF_TALLOC_TRACKLOAD | CFF_TALLOC_SIMPLE))
    {
	return nextTrack(opts, trackno);
    }
    if (trackno >= 40 && !(opts->flags & CFF_42TRACK))
    {
	return 0;
    }
    if (trackno >= 35)
    {
	if (!(opts->flags & (CFF_40TRACK | CFF_42TRACK)))
	{
	    return 0;
	}
	return trackno + 1;
    }
    int diff = 18 - trackno;
    if (!diff) return 17;
    if (diff > 0) return 18 + diff;
    return 18 + diff - 1;
}

static int findStartSector(CbmdosFs *self,
        uint8_t *trackno, uint8_t *sectno, const CbmdosFsOptions *opts)
{
    uint8_t tn = startTrack(opts, 0);
    do
    {
	for (uint8_t sn = 0;
		sn < Track_sectors(D64_rtrack(self->d64, tn)); ++sn)
	{
	    if (!self->bam[tn-1][sn])
	    {
		*trackno = tn;
		*sectno = sn;
		return 0;
	    }
	}
    } while ((tn = startTrack(opts, tn)));
    return -1;
}

static int findNextSector(CbmdosFs *self,
        uint8_t *trackno, uint8_t *sectno, const CbmdosFsOptions *opts)
{
    uint8_t tn = *trackno;
    uint8_t sn = freeSectorOnTrack(self, tn, *sectno, opts->fileInterleave,
            (opts->flags & CFF_SIMPLEINTERLEAVE));
    do
    {
	if (sn != 0xff)
	{
	    *trackno = tn;
	    *sectno = sn;
	    return 0;
	}
	if ((tn = nextTrack(opts, tn)))
	{
	    if (opts->flags & CFF_TALLOC_CHAININTERLV)
	    {
		sn = freeSectorOnTrack(self, tn, sn, opts->fileInterleave,
                        (opts->flags & CFF_SIMPLEINTERLEAVE));
	    }
	    else
	    {
		sn = freeSectorOnTrack(self, tn, sn, 0, 0);
	    }
	}
    } while (tn);
    return findStartSector(self, trackno, sectno, opts);
}

static int updateDir(CbmdosFs *self)
{
    uint8_t trackno = 18;
    uint8_t sectno = 1;
    deleteChain(self, trackno, sectno);
    uint8_t *dir = Sector_content(D64_sector(self->d64, trackno, sectno));
    memset(dir, 0, 256);
    dir[1] = 0xff;
    self->bam[17][1] = 1;
    uint8_t dirpos = 0;
    for (unsigned i = 0; i < self->dir.size; ++i)
    {
        if (dirpos == 8)
        {
            uint8_t nextsect = freeSectorOnTrack(self, trackno, sectno,
                    self->options.dirInterleave,
                    (self->options.flags & CFF_SIMPLEINTERLEAVE));
            if (nextsect == 0xff)
            {
                if (!(self->options.flags & CFF_ALLOWLONGDIR)) goto fail;
                while (nextsect == 0xff)
                {
                    uint8_t nexttrack = nextTrack(&(self->options), trackno);
                    if (nexttrack == 18) goto fail;
                    trackno = nexttrack;
                    nextsect = freeSectorOnTrack(self, trackno, sectno,
                            self->options.dirInterleave,
                            (self->options.flags & CFF_SIMPLEINTERLEAVE));
                }
            }
            sectno = nextsect;
            dir[0] = trackno;
            dir[1] = sectno;
            dir = Sector_content(D64_sector(self->d64, trackno, sectno));
	    self->bam[trackno-1][sectno] = 1;
            memset(dir, 0, 256);
            dir[1] = 0xff;
            dirpos = 0;
        }
        const CbmdosFile *file = CbmdosVfs_rfile(self->vfs, i);
        uint8_t *dirent = dir + 0x20*dirpos;
	int invalidType = CbmdosFile_invalidType(file);
	if (invalidType < 0) dirent[2] = CbmdosFile_type(file);
	else dirent[2] = (uint8_t)invalidType & 0xf;
        if (CbmdosFile_locked(file)) dirent[2] |= 1<<6;
        if (CbmdosFile_closed(file)) dirent[2] |= 1<<7;
        dirent[3] = self->dir.entries[i].starttrack;
        dirent[4] = self->dir.entries[i].startsector;
        memset(dirent+5, 0xa0, 0x10);
        uint8_t namelen;
        const char *name = CbmdosFile_name(file, &namelen);
        memcpy(dirent+5, name, namelen);
        if (CbmdosFile_type(file) == CFT_REL)
        {
            dirent[0x15] = self->dir.entries[i].sidetrack;
            dirent[0x16] = self->dir.entries[i].sidesector;
            dirent[0x17] = CbmdosFile_recordLength(file);
        }
        dirent[0x1e] = self->dir.entries[i].blocks & 0xff;
        dirent[0x1f] = self->dir.entries[i].blocks >> 8;
	++dirpos;
    }
    self->status &= ~CFS_DIRFULL;
    return 0;

fail:
    logmsg(L_ERROR, "CbmdosFs: no space left writing directory.");
    return -1;
}

static void scratchFile(CbmdosFs *self, unsigned pos)
{
    if (self->dir.entries[pos].sidetrack)
    {
        deleteChain(self, self->dir.entries[pos].sidetrack,
                self->dir.entries[pos].sidesector);
    }
    if (self->dir.entries[pos].starttrack)
    {
        deleteChain(self, self->dir.entries[pos].starttrack,
                self->dir.entries[pos].startsector);
    }
    memset(self->dir.entries + pos, 0, sizeof *self->dir.entries);
}

static int updateFile(CbmdosFs *self, unsigned pos)
{
    uint8_t trackno;
    uint8_t sectno;
    uint16_t blocks = 0;

    const CbmdosFile *file = CbmdosVfs_rfile(self->vfs, pos);
    const FileData *fdat = CbmdosFile_rdata(file);
    size_t length = FileData_size(fdat);

    scratchFile(self, pos);

    int invalidType = CbmdosFile_invalidType(file);
    CbmdosFileType type = CbmdosFile_type(file);
    if ((invalidType < 0 && type == CFT_DEL) || !length)
    {
	uint16_t forcedBlocks = CbmdosFile_forcedBlocks(file);
	if (forcedBlocks != 0xffff)
	{
	    self->dir.entries[pos].blocks = forcedBlocks;
	}
	return 0;
    }

    CbmdosFsOptOverrides overrides = CbmdosFile_optOverrides(file);
    CbmdosFsOptions opts = self->options;
    CbmdosFsOptions_applyOverrides(&opts, &overrides);

    uint8_t tracks[720];
    uint8_t sectors[720];
    uint16_t sidesectlinkno = 0;

    if (findStartSector(self, &trackno, &sectno, &opts) < 0) goto fail;
    self->dir.entries[pos].starttrack = trackno;
    self->dir.entries[pos].startsector = sectno;
    const uint8_t *content = FileData_rcontent(fdat);
    while (length)
    {
	self->bam[trackno-1][sectno] = 1;
        if (type == CFT_REL)
        {
            tracks[sidesectlinkno] = trackno;
            sectors[sidesectlinkno] = sectno;
            if (++sidesectlinkno > 720)
            {
                scratchFile(self, pos);
                goto fail;
            }
        }
	uint8_t *block = Sector_content(D64_sector(self->d64, trackno, sectno));
	block[0] = 0;
	if (length <= 254)
	{
	    block[1] = length+1;
	    memcpy(block+2, content, length);
	    if (length < 254)
	    {
		memset(block+2+length, 0, 254-length);
	    }
	    length = 0;
	}
	else
	{
	    if (findNextSector(self, &trackno, &sectno, &opts) < 0)
	    {
		scratchFile(self, pos);
		goto fail;
	    }
	    block[0] = trackno;
	    block[1] = sectno;
	    memcpy(block+2, content, 254);
	    length -= 254;
	    content += 254;
	}
	++blocks;
    }
    uint16_t forcedBlocks = CbmdosFile_forcedBlocks(file);
    if (forcedBlocks != 0xffff) blocks = forcedBlocks;
    self->dir.entries[pos].blocks = blocks;

    if (type == CFT_REL)
    {
        uint8_t sidesectnum = sidesectlinkno / 120
            + !!(sidesectlinkno % 120);
        uint8_t *sidesects[6] = { 0 };
        uint8_t sidetracks[6] = { 0 };
        uint8_t sidesectors[6] = { 0 };
        uint8_t recordlen = CbmdosFile_recordLength(file);

        if (findStartSector(self, &trackno, &sectno, &opts) < 0)
        {
            scratchFile(self, pos);
            goto fail;
        }
        self->bam[trackno-1][sectno] = 1;
        sidetracks[0] = trackno;
        sidesectors[0] = sectno;
        self->dir.entries[pos].sidetrack = trackno;
        self->dir.entries[pos].sidesector = sectno;
        sidesects[0] = Sector_content(D64_sector(self->d64, trackno, sectno));
        sidesects[0][0] = 0;
        sidesects[0][1] = 0;
        sidesects[0][2] = 0;
        sidesects[0][3] = recordlen;
        for (uint8_t i = 1; i < sidesectnum; ++i)
        {
            if (findNextSector(self, &trackno, &sectno, &opts) < 0)
            {
                scratchFile(self, pos);
                goto fail;
            }
            sidesects[i-1][0] = trackno;
            sidesects[i-1][1] = sectno;
            self->bam[trackno-1][sectno] = 1;
            sidetracks[i] = trackno;
            sidesectors[i] = sectno;
            sidesects[i] = Sector_content(D64_sector(
                        self->d64, trackno, sectno));
            sidesects[i][0] = 0;
            sidesects[i][1] = 0;
            sidesects[i][2] = i;
            sidesects[i][3] = recordlen;
        }
        for (uint8_t i = 0; i < sidesectnum; ++i)
        {
            for (uint8_t j = 0; j < 6; ++j)
            {
                sidesects[i][4+2*j] = sidetracks[j];
                sidesects[i][5+2*j] = sidesectors[j];
            }
            for (int l = 0; l < 120; ++l)
            {
                sidesects[i][16+2*l] = tracks[l+120*i];
                sidesects[i][17+2*l] = sectors[l+120*i];
            }
        }
    }

    return 0;

fail:
    logmsg(L_ERROR, "CbmdosFs: no space left writing file.");
    return -1;
}

static void vfsChanged(void *receiver, int id, const void *sender,
	const void *args)
{
    (void)id;
    (void)sender;

    CbmdosFs *self = receiver;
    if (self->status & CFS_BROKEN) return;
    const CbmdosVfsEventArgs *ea = args;
    switch (ea->what)
    {
	DirEntry tmpEntry;

	case CVE_DOSVERCHANGED:
	case CVE_NAMECHANGED:
	case CVE_IDCHANGED:
	    updateBam(self);
	    break;

	case CVE_FILEADDED:
	    if (ea->filepos > self->dir.size)
	    {
		logmsg(L_ERROR, "CbmdosFs: inconsistent state, file added at "
			"invalid position.");
		self->status |= CFS_BROKEN;
		break;
	    }
	    if (self->dir.size == self->dir.capa)
	    {
		self->dir.capa += DIRCHUNK;
		self->dir.entries = xrealloc(self->dir.entries,
			self->dir.capa * sizeof *self->dir.entries);
	    }
	    if (ea->filepos < self->dir.size)
	    {
		memmove(self->dir.entries + ea->filepos + 1,
			self->dir.entries + ea->filepos,
			(self->dir.size - ea->filepos)
			* sizeof *self->dir.entries);
	    }
	    ++self->dir.size;
            memset(self->dir.entries + ea->filepos, 0,
                    sizeof *self->dir.entries);
	    if (updateDir(self) < 0)
	    {
		self->status |= CFS_DIRFULL;
		updateBam(self);
		break;
	    }
	    if (updateFile(self, ea->filepos) < 0)
	    {
		self->status |= CFS_DISKFULL;
	    }
	    updateDir(self);
	    updateBam(self);
	    break;

	case CVE_FILEDELETED:
	    if (ea->filepos >= self->dir.size)
	    {
		logmsg(L_ERROR, "CbmdosFs: inconsistent state, non-existent "
			"file removed.");
		self->status |= CFS_BROKEN;
		break;
	    }
	    scratchFile(self, ea->filepos);
	    --self->dir.size;
	    if (ea->filepos < self->dir.size)
	    {
		memmove(self->dir.entries + ea->filepos,
			self->dir.entries + ea->filepos + 1,
			(self->dir.size - ea->filepos)
			* sizeof *self->dir.entries);
	    }
	    if (self->status & CFS_DISKFULL)
	    {
		self->status &= ~CFS_DISKFULL;
		for (unsigned pos = 0; pos < self->dir.size; ++pos)
		{
		    if (updateFile(self, pos) < 0)
		    {
			self->status |= CFS_DISKFULL;
		    }
		}
	    }
	    updateDir(self);
	    updateBam(self);
	    break;

	case CVE_FILEMOVED:
	    if (ea->filepos >= self->dir.size
		    || ea->targetpos >= self->dir.size)
	    {
		logmsg(L_ERROR, "CbmdosFs: inconsistent state, file moved to "
			"or from invalid position.");
		self->status |= CFS_BROKEN;
		break;
	    }
	    memcpy(&tmpEntry, self->dir.entries + ea->filepos,
		    sizeof tmpEntry);
	    if (ea->targetpos > ea->filepos)
	    {
		memmove(self->dir.entries + ea->filepos,
			self->dir.entries + ea->filepos + 1,
			(ea->targetpos - ea->filepos)
			* sizeof *self->dir.entries);
	    }
	    else
	    {
		memmove(self->dir.entries + ea->targetpos + 1,
			self->dir.entries + ea->targetpos,
			(ea->filepos - ea->targetpos)
			* sizeof *self->dir.entries);
	    }
	    memcpy(self->dir.entries + ea->targetpos, &tmpEntry,
		    sizeof *self->dir.entries);
	    updateDir(self);
	    updateBam(self);
	    break;

	case CVE_FILECHANGED:
	    if (ea->fileEventArgs->what == CFE_DATACHANGED
                    || ea->fileEventArgs->what == CFE_TYPECHANGED
                    || ea->fileEventArgs->what == CFE_RECORDLENGTHCHANGED
		    || ea->fileEventArgs->what == CFE_FORCEDBLOCKSCHANGED)
	    {
		if (updateFile(self, ea->filepos) < 0)
		{
		    self->status |= CFS_DISKFULL;
		}
		if (self->status & CFS_DISKFULL)
		{
		    self->status &= ~CFS_DISKFULL;
		    for (unsigned pos = 0; pos < self->dir.size; ++pos)
		    {
			if (updateFile(self, pos) < 0)
			{
			    self->status |= CFS_DISKFULL;
			}
		    }
		}
	    }
	    updateDir(self);
	    updateBam(self);
	    break;
    }
}

static int validateOptions(CbmdosFsOptions options)
{
    if ((options.flags & (CFF_SPEEDDOSBAM | CFF_DOLPHINDOSBAM))
	    && (options.flags & CFF_PROLOGICDOSBAM))
    {
	logmsg(L_ERROR, "Cannot combine Prologic DOS extended BAM with any "
		"other extended BAM formats.");
	return -1;
    }
    if ((options.flags & CFF_TALLOC_TRACKLOAD)
	    && (options.flags & CFF_TALLOC_SIMPLE))
    {
	logmsg(L_ERROR, "Cannot combine `trackload' track allocation strategy "
		"with `simple' track allocation strategy.");
	return -1;
    }
    if (options.dirInterleave < 1 || options.dirInterleave > 20
            || options.fileInterleave < 1 || options.fileInterleave > 20)
    {
        logmsg(L_ERROR, "Cannot set interleave values outside the range from "
                "1 to 20.");
        return -1;
    }
    return 0;
}

SOEXPORT CbmdosFs *CbmdosFs_create(CbmdosFsOptions options)
{
    if (validateOptions(options) < 0) return 0;
    CbmdosFs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    D64Type d64Type = D64_STANDARD;
    if (options.flags & CFF_42TRACK) d64Type = D64_42TRACK;
    else if (options.flags & CFF_40TRACK) d64Type = D64_40TRACK;
    self->d64 = D64_create(d64Type);
    self->dir.capa = DIRCHUNK;
    self->dir.entries = xmalloc(DIRCHUNK * sizeof *self->dir.entries);
    self->vfs = CbmdosVfs_create();
    if (options.flags & CFF_PROLOGICDOSBAM)
    {
	CbmdosVfs_setDosver(self->vfs, 0x50);
    }
    self->options = options;
    self->bam[17][0] = 1;
    updateDir(self);
    updateBam(self);
    Event_register(CbmdosVfs_changedEvent(self->vfs), self, vfsChanged);
    return self;
}

SOEXPORT CbmdosFs *CbmdosFs_fromImage(D64 *d64, CbmdosFsOptions options)
{
    if (validateOptions(options) < 0) return 0;
    switch (D64_type(d64))
    {
	case D64_STANDARD:
	    if (options.flags & (CFF_40TRACK | CFF_42TRACK))
	    {
		logmsg(L_ERROR, "CbmdosFs_fromImage: trying to read a 40- or "
			"42-tracks filesystem from an image that only has "
			"35 tracks.");
		return 0;
	    }
	    break;
	case D64_40TRACK:
	    if (options.flags & CFF_42TRACK)
	    {
		logmsg(L_ERROR, "CbmdosFs_fromImage: trying to read a "
			"42-tracks filesystem from an image that only has "
			"40 tracks.");
		return 0;
	    }
	    if (!(options.flags & CFF_40TRACK))
	    {
		logmsg(L_WARNING, "CbmdosFs_fromImage: trying to read a "
			"35-tracks filesystem from an image that has 40 "
			"tracks, this will fail if the filesystem spans "
			"all 40 tracks.");
	    }
	    break;
	case D64_42TRACK:
	    if (!(options.flags & CFF_42TRACK))
	    {
		logmsg(L_WARNING, "CbmdosFs_fromImage: trying to read a 35- "
			"or 40-tracks filesystem from an image that has 42 "
			"tracks, this will fail if the filesystem spans "
			"all 42 tracks.");
	    }
	    break;
    }

    CbmdosFs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->d64 = d64;
    self->dir.capa = DIRCHUNK;
    self->dir.entries = xmalloc(DIRCHUNK * sizeof *self->dir.entries);
    self->vfs = CbmdosVfs_create();
    self->options = options;
    int rc = readCbmdosVfsInternal(self->vfs, self->d64,
	    &self->options, self->bam, &self->dir);
    switch (rc)
    {
	case -1:
	    self->d64 = 0;
	    CbmdosFs_destroy(self);
	    return 0;
	case -2:
	    self->status |= CFS_INVALIDBAM;
	    break;
    }
    if (self->options.flags & CFF_RECOVER)
    {
	self->options.flags &= ~CFF_RECOVER;
	self->status |= CFS_BROKEN;
    }
    Event_register(CbmdosVfs_changedEvent(self->vfs), self, vfsChanged);
    return self;
}

SOEXPORT CbmdosFs *CbmdosFs_fromVfs(CbmdosVfs *vfs, CbmdosFsOptions options)
{
    if (validateOptions(options) < 0) return 0;
    CbmdosFs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->dir.capa = DIRCHUNK;
    self->dir.size = CbmdosVfs_fileCount(vfs);
    while (self->dir.capa < self->dir.size)
    {
	self->dir.capa += DIRCHUNK;
    }
    self->dir.entries = xmalloc(self->dir.capa * sizeof *self->dir.entries);
    if ((options.flags & CFF_PROLOGICDOSBAM)
	    && CbmdosVfs_dosver(vfs) == 0x41)
    {
	CbmdosVfs_setDosver(vfs, 0x50);
    }
    self->vfs = vfs;
    self->options = options;
    if (CbmdosFs_rewrite(self) < 0)
    {
	self->vfs = 0;
	CbmdosFs_destroy(self);
	return 0;
    }
    Event_register(CbmdosVfs_changedEvent(self->vfs), self, vfsChanged);
    return self;
}

SOEXPORT CbmdosFsStatus CbmdosFs_status(const CbmdosFs *self)
{
    return self->status;
}

SOEXPORT const CbmdosVfs *CbmdosFs_rvfs(const CbmdosFs *self)
{
    return self->vfs;
}

SOEXPORT CbmdosVfs *CbmdosFs_vfs(CbmdosFs *self)
{
    return self->vfs;
}

SOEXPORT const D64 *CbmdosFs_image(const CbmdosFs *self)
{
    return self->d64;
}

SOEXPORT CbmdosFsOptions CbmdosFs_options(const CbmdosFs *self)
{
    return self->options;
}

SOEXPORT int CbmdosFs_setOptions(CbmdosFs *self, CbmdosFsOptions options)
{
    int needRewrite = CbmdosFs_optionsWillRewrite(self, options);
    if (needRewrite < 0) return -1;
    CbmdosFsFlags changedFlags = self->options.flags ^ options.flags;
    self->options = options;
    if (needRewrite)
    {
        CbmdosFs_rewrite(self);
        return 1;
    }
    else if (changedFlags & (CFF_SPEEDDOSBAM|CFF_DOLPHINDOSBAM
                |CFF_PROLOGICDOSBAM|CFF_ZEROFREE))
    {
        updateBam(self);
        return 1;
    }
    return 0;
}

SOEXPORT int CbmdosFs_optionsWillRewrite(
	const CbmdosFs *self, CbmdosFsOptions options)
{
    if (validateOptions(options) < 0) return -1;
    CbmdosFsFlags changedFlags = self->options.flags ^ options.flags;
    if (changedFlags & (
		CFF_FILESONDIRTRACK|CFF_ALLOWLONGDIR|CFF_42TRACK|
		CFF_SIMPLEINTERLEAVE|CFF_TALLOC_TRACKLOAD|CFF_TALLOC_SIMPLE|
		CFF_TALLOC_PREFDIRTRACK|CFF_TALLOC_CHAININTERLV))
    {
	return 1;
    }
    if (!(self->options.flags & CFF_42TRACK) && (changedFlags & CFF_40TRACK))
    {
        return 1;
    }
    if (options.dirInterleave != self->options.dirInterleave) return 1;
    if (options.fileInterleave != self->options.fileInterleave) return 1;
    return 0;
}

SOEXPORT int CbmdosFs_rewrite(CbmdosFs *self)
{
    D64_destroy(self->d64);
    D64Type d64Type = D64_STANDARD;
    if (self->options.flags & CFF_42TRACK) d64Type = D64_42TRACK;
    else if (self->options.flags & CFF_40TRACK) d64Type = D64_40TRACK;
    self->d64 = D64_create(d64Type);
    self->status = CFS_OK;
    memset(self->dir.entries, 0, self->dir.size * sizeof *self->dir.entries);
    memset(self->bam, 0, sizeof self->bam);
    self->bam[17][0] = 1;
    if (updateDir(self) < 0)
    {
	self->status |= CFS_DIRFULL;
	return -1;
    }
    for (unsigned pos = 0; pos < self->dir.size; ++pos)
    {
	if (updateFile(self, pos) < 0)
	{
	    self->status |= CFS_DISKFULL;
	    return -1;
	}
    }
    updateDir(self);
    updateBam(self);
    return 0;
}

SOEXPORT uint16_t CbmdosFs_freeBlocks(const CbmdosFs *self)
{
    uint16_t free = 664;
    if (self->options.flags & CFF_42TRACK)
    {
	free += 119;
    }
    else if (self->options.flags & CFF_40TRACK)
    {
	free += 85;
    }
    unsigned filecount = CbmdosVfs_fileCount(self->vfs);
    if (filecount > 144)
    {
	if (!(self->options.flags & CFF_ALLOWLONGDIR)) return 0xffff;
	unsigned extrafiles = filecount - 144;
	unsigned dirblocks = extrafiles / 8;
	if (extrafiles % 8) ++dirblocks;
	if (dirblocks > free) return 0xffff;
	free -= dirblocks;
    }
    else if (self->options.flags & CFF_FILESONDIRTRACK)
    {
	unsigned freefiles = 144 - filecount;
	unsigned freedirblocks = freefiles / 8;
	free += freedirblocks;
    }
    for (unsigned n = 0; n < filecount; ++n)
    {
	const CbmdosFile *file = CbmdosVfs_rfile(self->vfs, n);
	uint16_t fileBlocks = CbmdosFile_realBlocks(file);
        if (CbmdosFile_type(file) == CFT_REL)
        {
            uint8_t sidesects = fileBlocks / 120 + !!(fileBlocks % 120);
            fileBlocks += sidesects;
        }
	if (fileBlocks > free) return 0xffff;
	free -= fileBlocks;
    }
    return free;
}

SOEXPORT void CbmdosFs_getFreeBlocksLine(const CbmdosFs *self, uint8_t *line)
{
    uint16_t rawFree = CbmdosFs_freeBlocks(self);
    int free = (rawFree == 0xffff) ? -1 : rawFree;
    if (free > 0 && (self->options.flags & CFF_ZEROFREE)) free = 0;
    char freestr[4];
    snprintf(freestr, sizeof freestr, "%d", free);
    memset(line, 0xa0, 16);
    const char *blocksfree = " BLOCKS FREE.";
    uint8_t *w = line;
    const char *r = freestr;
    while (*r) *w++ = *r++;
    r = blocksfree;
    while (*r) *w++ = *r++;
}

SOEXPORT void CbmdosFs_destroy(CbmdosFs *self)
{
    if (!self) return;
    free(self->dir.entries);
    CbmdosVfs_destroy(self->vfs);
    D64_destroy(self->d64);
    free(self);
}
