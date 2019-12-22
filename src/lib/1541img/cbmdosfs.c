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
    uint8_t bam[40][21];
};

static void createTrackBam(CbmdosFs *self, uint8_t *tbam, uint8_t trackno)
{
    uint8_t sectors = Track_sectors(D64_track(self->d64, trackno));
    tbam[0] = sectors;
    tbam[1] = 0xff;
    tbam[2] = 0xff;
    tbam[3] = 0xff;
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
        else
        {
            if (self->options.flags & CFF_DOLPHINDOSBAM)
            {
                createTrackBam(self, bam + 0x1c + 4*trackno, trackno);
            }
            if (self->options.flags & CFF_SPEEDDOSBAM)
            {
                createTrackBam(self, bam + 0x30 + 4*trackno, trackno);
            }
        }
    }
    memset(bam+0x90, 0xa0, 0x1a);
    bam[0xa5] = 0x32;
    bam[0xa6] = 0x41;
    uint8_t length;
    const char *name = CbmdosVfs_name(self->vfs, &length);
    memcpy(bam+0x90, name, length);
    const char *id = CbmdosVfs_id(self->vfs, &length);
    memcpy(bam+0xa2, id, length);
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
        uint8_t sectno, uint8_t interlv)
{
    uint8_t sectors = Track_sectors(D64_track(self->d64, trackno));
    uint8_t nextsect = (sectno + interlv) % sectors;
    uint8_t tmp = nextsect;
    while (self->bam[trackno-1][nextsect])
    {
        ++nextsect;
        if (nextsect == sectors) nextsect = 0;
        if (nextsect == tmp) break;
    }
    return self->bam[trackno-1][nextsect] ? 0xff : nextsect;
}

static uint8_t nextTrack(CbmdosFs *self, uint8_t trackno)
{
    if (trackno == 40) return 1;
    if (trackno == 35 && !(self->options.flags & CFF_40TRACK)) return 1;
    return trackno + 1;
}

static int findStartSector(CbmdosFs *self, uint8_t *trackno, uint8_t *sectno)
{
    uint8_t tn = 19;
    uint8_t stoptn = (self->options.flags & CFF_FILESONDIRTRACK) ? 19 : 18;
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
	tn = nextTrack(self, tn);
    } while (tn != stoptn);
    return -1;
}

static int findNextSector(CbmdosFs *self, uint8_t *trackno, uint8_t *sectno)
{
    uint8_t tn = *trackno;
    uint8_t sn = *sectno;
    if (tn == 18 && !(self->options.flags & CFF_FILESONDIRTRACK)) return -1;
    do
    {
	sn = freeSectorOnTrack(self, tn, sn, self->options.fileInterleave);
	if (sn != 0xff)
	{
	    *trackno = tn;
	    *sectno = sn;
	    return 0;
	}
	tn = nextTrack(self, tn);
	if (tn == 18 && !(self->options.flags & CFF_FILESONDIRTRACK)) tn = 19;
    } while (tn != *trackno);
    return -1;
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
                    self->options.dirInterleave);
            if (nextsect == 0xff)
            {
                if (!(self->options.flags & CFF_ALLOWLONGDIR)) goto fail;
                while (nextsect == 0xff)
                {
                    uint8_t nexttrack = nextTrack(self, trackno);
                    if (nexttrack == 18) goto fail;
                    trackno = nexttrack;
                    nextsect = freeSectorOnTrack(self, trackno, sectno,
                            self->options.dirInterleave);
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
        dirent[2] = CbmdosFile_type(file);
        if (CbmdosFile_locked(file)) dirent[2] |= 1<<6;
        if (CbmdosFile_closed(file)) dirent[2] |= 1<<7;
        dirent[3] = self->dir.entries[i].starttrack;
        dirent[4] = self->dir.entries[i].startsector;
        memset(dirent+5, 0xa0, 0x10);
        uint8_t namelen;
        const char *name = CbmdosFile_name(file, &namelen);
        memcpy(dirent+5, name, namelen);
        dirent[0x1e] = self->dir.entries[i].blocks & 0xff;
        dirent[0x1f] = self->dir.entries[i].blocks >> 8;
	++dirpos;
    }
    return 0;

fail:
    logmsg(L_ERROR, "CbmdosFs: no space left writing directory.");
    return -1;
}

static void scratchFile(CbmdosFs *self, unsigned pos)
{
    if (!self->dir.entries[pos].starttrack) return;
    deleteChain(self, self->dir.entries[pos].starttrack,
	    self->dir.entries[pos].startsector);
    self->dir.entries[pos].starttrack = 0;
    self->dir.entries[pos].startsector = 0;
    self->dir.entries[pos].blocks = 0;
}

static int updateFile(CbmdosFs *self, unsigned pos)
{
    uint8_t trackno;
    uint8_t sectno;
    uint16_t blocks = 0;

    const CbmdosFile *file = CbmdosVfs_rfile(self->vfs, pos);
    const FileData *fdat = CbmdosFile_rdata(file);
    size_t length = FileData_size(fdat);
    if (CbmdosFile_type(file) == CFT_DEL || !length)
    {
	uint16_t forcedBlocks = CbmdosFile_forcedBlocks(file);
	if (forcedBlocks != 0xffff)
	{
	    self->dir.entries[pos].blocks = forcedBlocks;
	}
	else
	{
	    self->dir.entries[pos].blocks = 0;
	}
	self->dir.entries[pos].starttrack = 0;
	self->dir.entries[pos].startsector = 0;
	return 0;
    }

    scratchFile(self, pos);

    if (findStartSector(self, &trackno, &sectno) < 0) goto fail;
    self->dir.entries[pos].starttrack = trackno;
    self->dir.entries[pos].startsector = sectno;
    const uint8_t *content = FileData_rcontent(fdat);
    while (length)
    {
	self->bam[trackno-1][sectno] = 1;
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
	    if (findNextSector(self, &trackno, &sectno) < 0)
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
	    self->dir.entries[ea->filepos].starttrack = 0;
	    self->dir.entries[ea->filepos].startsector = 0;
	    self->dir.entries[ea->filepos].blocks = 0;
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
		    || ea->fileEventArgs->what == CFE_FORCEDBLOCKSCHANGED)
	    {
		if (updateFile(self, ea->filepos) < 0)
		{
		    self->status |= CFS_DISKFULL;
		}
	    }
	    updateDir(self);
	    updateBam(self);
	    break;
    }
}

CbmdosFs *CbmdosFs_create(CbmdosFsOptions options)
{
    if (options.flags & CFF_FILESONDIRTRACK)
    {
	logmsg(L_WARNING, "CbmdosFs_fromImage: Ignoring CFF_FILESONDIRTRACK, "
		"flag ist invalid when creating empty FS.");
	options.flags &= ~CFF_FILESONDIRTRACK;
    }

    CbmdosFs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->d64 = D64_create(options.flags & CFF_40TRACK ?
            D64_40TRACK : D64_STANDARD);
    self->dir.capa = DIRCHUNK;
    self->dir.entries = xmalloc(DIRCHUNK * sizeof *self->dir.entries);
    self->vfs = CbmdosVfs_create();
    self->options = options;
    self->bam[17][0] = 1;
    updateBam(self);
    Event_register(CbmdosVfs_changedEvent(self->vfs), self, vfsChanged);
    return self;
}

CbmdosFs *CbmdosFs_fromImage(D64 *d64, CbmdosFsOptions options)
{
    if (options.flags & CFF_FILESONDIRTRACK)
    {
	logmsg(L_WARNING, "CbmdosFs_fromImage: Ignoring CFF_FILESONDIRTRACK, "
		"flag ist invalid when creating FS from an image.");
	options.flags &= ~CFF_FILESONDIRTRACK;
    }
    switch (D64_type(d64))
    {
	case D64_STANDARD:
	    if (options.flags & CFF_40TRACK)
	    {
		logmsg(L_WARNING, "CbmdosFs_fromImage: removing CFF_40TRACK "
			"flag, passed image has only 35 tracks.");
		options.flags &= ~CFF_40TRACK;
	    }
	    break;
	case D64_40TRACK:
	    if (!(options.flags & CFF_40TRACK))
	    {
		logmsg(L_WARNING, "CbmdosFs_fromImage: adding CFF_40TRACK "
			"flag, passed image has 40 tracks.");
		options.flags |= CFF_40TRACK;
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
    int rc = readCbmdosVfsInternal(self->vfs, self->d64, self->bam, &self->dir);
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
    Event_register(CbmdosVfs_changedEvent(self->vfs), self, vfsChanged);
    return self;
}

CbmdosFs *CbmdosFs_fromVfs(CbmdosVfs *vfs, CbmdosFsOptions options)
{
    CbmdosFs *self = xmalloc(sizeof *self);
    memset(self, 0, sizeof *self);
    self->dir.capa = DIRCHUNK;
    self->dir.size = CbmdosVfs_fileCount(vfs);
    while (self->dir.capa < self->dir.size)
    {
	self->dir.capa += DIRCHUNK;
    }
    self->dir.entries = xmalloc(DIRCHUNK * sizeof *self->dir.entries);
    memset(self->dir.entries, 0, sizeof *self);
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

CbmdosFsStatus CbmdosFs_status(const CbmdosFs *self)
{
    return self->status;
}

const CbmdosVfs *CbmdosFs_rvfs(const CbmdosFs *self)
{
    return self->vfs;
}

CbmdosVfs *CbmdosFs_vfs(CbmdosFs *self)
{
    return self->vfs;
}

const D64 *CbmdosFs_image(const CbmdosFs *self)
{
    return self->d64;
}

CbmdosFsOptions CbmdosFs_options(const CbmdosFs *self)
{
    return self->options;
}

void CbmdosFs_setOptions(CbmdosFs *self, CbmdosFsOptions options)
{
    self->options = options;
}

int CbmdosFs_rewrite(CbmdosFs *self)
{
    D64_destroy(self->d64);
    self->d64 = D64_create(self->options.flags & CFF_40TRACK ?
	    D64_40TRACK : D64_STANDARD);
    self->status = CFS_OK;
    memset(self->bam, 0, sizeof self->bam);
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

uint16_t CbmdosFs_freeBlocks(const CbmdosFs *self)
{
    uint16_t free = 664;
    if (self->options.flags & (CFF_DOLPHINDOSBAM | CFF_SPEEDDOSBAM))
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
	if (fileBlocks > free) return 0xffff;
	free -= fileBlocks;
    }
    return free;
}

void CbmdosFs_getFreeBlocksLine(const CbmdosFs *self, uint8_t *line)
{
    uint16_t rawFree = CbmdosFs_freeBlocks(self);
    int free = (rawFree == 0xffff) ? -1 : rawFree;
    char freestr[4];
    snprintf(freestr, 4, "%d", free);
    memset(line, 0xa0, 16);
    const char *blocksfree = " BLOCKS FREE.";
    uint8_t *w = line;
    const char *r = freestr;
    while (*r) *w++ = *r++;
    r = blocksfree;
    while (*r) *w++ = *r++;
}

void CbmdosFs_destroy(CbmdosFs *self)
{
    if (!self) return;
    free(self->dir.entries);
    CbmdosVfs_destroy(self->vfs);
    D64_destroy(self->d64);
    free(self);
}
