#include <stdlib.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/filedata.h>

#include <1541img/lynx.h>

static const uint8_t lynxheader[] =
{
    0x01, 0x08, 0x5B, 0x08, 0x0A, 0x00, 0x97, 0x35,
    0x33, 0x32, 0x38, 0x30, 0x2C, 0x30, 0x3A, 0x97,
    0x35, 0x33, 0x32, 0x38, 0x31, 0x2C, 0x30, 0x3A,
    0x97, 0x36, 0x34, 0x36, 0x2C, 0xC2, 0x28, 0x31,
    0x36, 0x32, 0x29, 0x3A, 0x99, 0x22, 0x93, 0x11,
    0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x22,
    0x3A, 0x99, 0x22, 0x20, 0x20, 0x20, 0x20, 0x20,
    0x55, 0x53, 0x45, 0x20, 0x4C, 0x59, 0x4E, 0x58,
    0x20, 0x54, 0x4F, 0x20, 0x44, 0x49, 0x53, 0x53,
    0x4F, 0x4C, 0x56, 0x45, 0x20, 0x54, 0x48, 0x49,
    0x53, 0x20, 0x46, 0x49, 0x4C, 0x45, 0x22, 0x3A,
    0x89, 0x31, 0x30, 0x00, 0x00, 0x00, 0x0d, 0x20,
    0x20, 0x20, 0x20, 0x2a, 0x4c, 0x59, 0x4e, 0x58,
    0x20, 0x41, 0x52, 0x43, 0x48, 0x49, 0x56, 0x45,
    0x20, 0x42, 0x59, 0x20, 0x45, 0x58, 0x43, 0x45,
    0x53, 0x53, 0x21, 0x0d, 0x20
};

static const uint8_t lynxfiletype[] = { 0x00, 0x53, 0x50, 0x55, 0x52 };
static const uint8_t lynxfakess[] = { 1, 11, 2, 12, 3, 13 };

typedef struct LynxDirEntry
{
    CbmdosFile *file;
    uint16_t blocks;
    size_t size;
} LynxDirEntry;

static int formatPetsciiNum(uint8_t *buf, size_t maxlen, unsigned num)
{
    int l = snprintf((char *)buf, maxlen, "%u", num);
    if (l < 1 || (unsigned)l >= maxlen) return -1;
#if '0' != 0x30
    for (int i = 0; i < l; ++i) buf[i] += (0x30 - '0');
#endif
    return l;
}

static int parsePetsciiNum(uint16_t *num, size_t *pos,
	const uint8_t *content, size_t size)
{
    size_t p = *pos;
    while (p < size && content[p] == 0x20) ++p;
    if (content[p] < 0x31 || content[p] > 0x39) return -1;
    uint16_t n = content[p++] - 0x30;
    if (p == size) return -1;
    if (content[p] > 0x2f && content[p] < 0x3a)
    {
	n *= 10;
	n += (content[p++] - 0x30);
	if (p == size) return -1;
    }
    if (content[p] > 0x2f && content[p] < 0x3a)
    {
	n *= 10;
	n += (content[p++] - 0x30);
	if (p == size) return -1;
    }
    if (content[p] > 0x2f && content[p] < 0x3a) return -1;
    while (p < size && content[p] == 0x20) ++p;
    if (p == size) return -1;
    if (content[p] != 0x0d) return -1;
    if (++p == size) return -1;
    *pos = p;
    *num = n;
    return 0;
}

static int findLynxHeader(size_t *sigpos, uint8_t *dirblocks, size_t *dirpos,
	const uint8_t *content, size_t size)
{
    size_t pos = *sigpos;
    while (pos < size && content[pos] == 0x20) ++pos;
    if (pos == size) return -1;
    if (content[pos] < 0x31 || content[pos] > 0x39) return -1;
    uint8_t blocks = content[pos] - 0x30;
    if (++pos == size) return -1;
    if (content[pos] > 0x2f && content[pos] < 0x3a)
    {
	blocks *= 10;
	blocks += (content[pos] - 0x30);
	if (++pos == size) return -1;
    }
    if (content[pos] > 0x2f && content[pos] < 0x3a) return -1;
    while (pos < size && content[pos] == 0x20) ++pos;
    size_t sigp = pos;
    while (pos < size && content[pos] != 0x0d)
    {
	if (!content[pos]) return -1;
	++pos;
    }
    if (++pos > size - 5) return -1;
    *sigpos = sigp;
    *dirblocks = blocks;
    *dirpos = pos;
    return 0;
}

static int findHeader(size_t *sigpos, uint8_t *dirblocks, size_t *dirpos,
	const uint8_t *content, size_t size)
{
    size_t pos = 0;
    if (size < 255) return -1;
    size_t base = content[0] | (content[1] << 8);
    size_t next = content[2] | (content[3] << 8);
    if (next <= base) goto lynxhdr;
    pos = next - base + 2;
    if (pos > size - 5)
    {
	pos = 0;
	goto lynxhdr;
    }
    if (content[pos-1])
    {
	pos = 0;
	goto lynxhdr;
    }
    while (content[pos] || content[pos+1])
    {
	size_t nextline = content[pos] | (content[pos+1] << 8);
	if (nextline <= next) return -1;
	next = nextline;
	pos = next - base + 2;
	if (pos > size - 5) return -1;
	if (content[pos-1]) return -1;
    }
    pos += 2;
    if (content[pos++] != 0x0d) return -1;
lynxhdr:
    if (findLynxHeader(&pos, dirblocks, dirpos, content, size) < 0) return -1;
    *sigpos = pos;
    return 0;
}

SOEXPORT int isLynx(const FileData *file)
{
    size_t size = FileData_size(file);
    const uint8_t *content = FileData_rcontent(file);
    size_t sigpos;
    uint8_t dirblocks;
    size_t dirpos;
    if (findHeader(&sigpos, &dirblocks, &dirpos, content, size) < 0) return 0;
    return 1;
}

SOEXPORT int extractLynx(CbmdosVfs *vfs, const FileData *file)
{
    size_t size = FileData_size(file);
    const uint8_t *content = FileData_rcontent(file);
    size_t sigpos;
    uint8_t dirblocks;
    size_t pos;
    if (findHeader(&sigpos, &dirblocks, &pos, content, size) < 0)
    {
	logmsg(L_ERROR, "extractLynx: not a valid LyNX file.");
	return -1;
    }
    char sig[80] = {0};
    size_t siglen = pos - sigpos - 1;
    if (siglen >= sizeof sig) siglen = sizeof sig - 1;
    memcpy(sig, content + sigpos, siglen);
    logfmt(L_INFO, "extractLynx: found signature `%s'.", sig);
    uint16_t numfiles;
    if (parsePetsciiNum(&numfiles, &pos, content, size) < 0)
    {
	logmsg(L_ERROR, "extractLynx: couldn't read number of files.");
	return -1;
    }
    LynxDirEntry *dir = xmalloc(numfiles * sizeof *dir);
    memset(dir, 0, numfiles * sizeof *dir);
    int rc = -1;

    for (int i = 0; i < numfiles; ++i)
    {
	uint8_t namelen = 0;
	while (pos + namelen < size
		&& content[pos + namelen] != 0x0d) ++namelen;
	if (pos + namelen + 1 >= size)
	{
	    logmsg(L_ERROR, "extractLynx: unexpected end of file.");
	    goto done;
	}
	size_t tmppos = pos + namelen + 1;
	while (content[pos + namelen - 1] == 0xa0) --namelen;
	dir[i].file = CbmdosFile_create();
	CbmdosFile_setName(dir[i].file, (const char *)(content + pos), namelen);
	pos = tmppos;
	if (parsePetsciiNum(&dir[i].blocks, &pos, content, size) < 0)
	{
	    logmsg(L_ERROR, "extractLynx: error parsing block size of file.");
	    goto done;
	}
	if (!content[pos])
	{
	    logmsg(L_ERROR, "extractLynx: invalid file type found.");
	    goto done;
	}
	int t;
	for (t = 1; t < (int) sizeof lynxfiletype; ++t)
	{
	    if (lynxfiletype[t] == content[pos]) break;
	}
	if (t == sizeof lynxfiletype)
	{
	    logmsg(L_ERROR, "extractLynx: invalid file type found.");
	    goto done;
	}
	CbmdosFileType type = (CbmdosFileType) t;
	if (++pos == size)
	{
	    logmsg(L_ERROR, "extractLynx: unexpected end of file.");
	    goto done;
	}
	if (content[pos] != 0x0d)
	{
	    logmsg(L_ERROR, "extractLynx: invalid file type found.");
	    goto done;
	}
	if (++pos == size)
	{
	    logmsg(L_ERROR, "extractLynx: unexpected end of file.");
	    goto done;
	}
	CbmdosFile_setType(dir[i].file, type);
	if (type == CFT_REL)
	{
	    uint16_t recordlen;
	    if (parsePetsciiNum(&recordlen, &pos, content, size) < 0)
	    {
		logmsg(L_ERROR,
			"extractLynx: error parsing record length of file.");
		goto done;
	    }
	    CbmdosFile_setRecordLength(dir[i].file, recordlen);
	}
	uint16_t lsu;
	if (parsePetsciiNum(&lsu, &pos, content, size) < 0)
	{
	    if (i == numfiles - 1)
	    {
		logmsg(L_INFO, "extractLynx: last block usage of last file "
			"missing, assuming size from the container.");
		dir[i].size = 0;
	    }
	    else
	    {
		logmsg(L_ERROR, "extractLynx: error parsing last block usage "
			"of file.");
		goto done;
	    }
	}
	else
	{
	    dir[i].size = 254 * (dir[i].blocks - 1) + (lsu - 1);
	}
    }

    for (int i = 0; i < numfiles; ++i)
    {
	size_t pad = 254 - (pos%254);
	pos += pad;
	if (pos >= size)
	{
	    logmsg(L_ERROR, "extractLynx: unexpected end of file.");
	    goto done;
	}
	if (i == 0 && pos != dirblocks * 254)
	{
	    logmsg(L_WARNING, "extractLynx: inconsistent file, directory "
		    "block size is wrong.");
	}
	if (CbmdosFile_type(dir[i].file) == CFT_REL)
	{
	    uint8_t sidesectnum = (dir[i].blocks / 120)
		+ !!(dir[i].blocks % 120);
	    pos += 254 * sidesectnum;
	    if (pos >= size)
	    {
		logmsg(L_ERROR, "extractLynx: unexpected end of file.");
		goto done;
	    }
	}
	if (i == numfiles - 1 && !dir[i].size)
	{
	    dir[i].size = size - pos;
	    if (dir[i].size < (dir[i].blocks-1U) * 254)
	    {
		logmsg(L_ERROR, "extractLynx: unexpected end of file.");
		goto done;
	    }
	    if (dir[i].size > dir[i].blocks * 254)
	    {
		logmsg(L_WARNING, "extractLynx: unexpected garbage after end "
			"of file, archive might be corrupt.");
		dir[i].size = dir[i].blocks * 254;
	    }
	}
	if (pos + dir[i].size > size)
	{
	    logmsg(L_ERROR, "extractLynx: unexpected end of file.");
	    goto done;
	}
	if (FileData_append(CbmdosFile_data(dir[i].file),
		    content + pos, dir[i].size) < 0)
	{
	    logmsg(L_ERROR, "extractLynx: error writing file.");
	    goto done;
	}
	pos += dir[i].size;
    }
    if (pos < size)
    {
	size_t pad = 254 - (pos%254);
	if (pos + pad != size)
	{
	    logmsg(L_WARNING,
		    "extractLynx: file doesn't have the expected size");
	}
    }

    for (int i = 0; i < numfiles; ++i)
    {
	if (CbmdosVfs_append(vfs, dir[i].file) < 0)
	{
	    logmsg(L_ERROR, "extractLynx: error adding file to filesystem.");
	    goto done;
	}
	else dir[i].file = 0;
    }

    rc = 0;

done:
    for (int i = 0; i < numfiles; ++i) CbmdosFile_destroy(dir[i].file);
    free(dir);
    return rc;
}

SOEXPORT FileData *archiveLynxFiles(
	const CbmdosFile **files, unsigned filecount)
{
    FileData *archive = FileData_create();
    FileData *result = 0;

    if (FileData_append(archive, lynxheader, sizeof lynxheader) < 0) goto done;
    size_t pos = sizeof lynxheader;
    uint8_t buf[4];
    int l;

    if ((l = formatPetsciiNum(buf, sizeof buf, filecount)) < 0) goto done;
    if (FileData_append(archive, buf, l) < 0) goto done;
    pos += l;
    if (FileData_appendByte(archive, 0x20) < 0) goto done;
    if (FileData_appendByte(archive, 0x0d) < 0) goto done;
    pos += 2;

    uint8_t namebuf[16];
    for (unsigned i = 0; i < filecount; ++i)
    {
	const CbmdosFile *file = files[i];
	memset(namebuf, 0xa0, sizeof namebuf);
	uint8_t namelen;
	const char *name = CbmdosFile_name(file, &namelen);
	memcpy(namebuf, name, namelen);
	if (FileData_append(archive, namebuf, sizeof namebuf) < 0) goto done;
	pos += sizeof namebuf;
	if (FileData_appendByte(archive, 0x0d) < 0) goto done;
	if (FileData_appendByte(archive, 0x20) < 0) goto done;
	pos += 2;
	uint16_t blocksize = CbmdosFile_realBlocks(file);
	if ((l = formatPetsciiNum(buf, sizeof buf, blocksize)) < 0) goto done;
	if (FileData_append(archive, buf, l) < 0) goto done;
	pos += l;
	if (FileData_appendByte(archive, 0x20) < 0) goto done;
	if (FileData_appendByte(archive, 0x0d) < 0) goto done;
	pos += 2;
	CbmdosFileType type = CbmdosFile_type(file);
	uint8_t lynxtype = lynxfiletype[type];
	if (!lynxtype) goto done;
	if (FileData_appendByte(archive, lynxtype) < 0) goto done;
	if (FileData_appendByte(archive, 0x0d) < 0) goto done;
	if (FileData_appendByte(archive, 0x20) < 0) goto done;
	pos += 3;
	if (type == CFT_REL)
	{
	    if ((l = formatPetsciiNum(buf, sizeof buf,
			    CbmdosFile_recordLength(file))) < 0) goto done;
	    if (FileData_append(archive, buf, l) < 0) goto done;
	    pos +=l;
	    if (FileData_appendByte(archive, 0x20) < 0) goto done;
	    if (FileData_appendByte(archive, 0x0d) < 0) goto done;
	    if (FileData_appendByte(archive, 0x20) < 0) goto done;
	    pos += 3;
	}
	const FileData *fileData = CbmdosFile_rdata(file);
	unsigned lsu = (unsigned)(
		FileData_size(fileData) - 254*(blocksize-1) + 1);
	if ((l = formatPetsciiNum(buf, sizeof buf, lsu)) < 0) goto done;
	if (FileData_append(archive, buf, l) < 0) goto done;
	pos += l;
	if (FileData_appendByte(archive, 0x20) < 0) goto done;
	if (FileData_appendByte(archive, 0x0d) < 0) goto done;
	pos += 2;
    }
    size_t pad = 254 - (pos%254);
    if (FileData_appendBytes(archive, 0, pad) < 0) goto done;
    pos += pad;
    if ((l = formatPetsciiNum(buf, 3, pos / 254)) < 0) goto done;
    if (FileData_setByte(archive, buf[0], 0x60) < 0) goto done;
    if (l == 2 && FileData_setByte(archive, buf[1], 0x61) < 0) goto done;
    for (unsigned i = 0; i < filecount; ++i)
    {
	const CbmdosFile *file = files[i];
	if (CbmdosFile_type(file) == CFT_REL)
	{
	    uint8_t sidesects[6 * 254] = {0};
	    uint16_t blocksize = CbmdosFile_realBlocks(file);
	    if (blocksize > 720) goto done;
	    uint8_t sidesectnum = (blocksize / 120) + !!(blocksize % 120);
	    uint8_t recordlength = CbmdosFile_recordLength(file);
	    for (uint8_t j = 0; j < sidesectnum; ++j)
	    {
		sidesects[254*j] = j;
		sidesects[254*j + 1] = recordlength;
		for (uint8_t k = 0; k < sidesectnum; ++k)
		{
		    sidesects[254*j + 2*k + 2] = 19;
		    sidesects[254*j + 2*k + 3] = lynxfakess[k];
		}
		for (uint8_t k = 0;
			k < (j < sidesectnum - 1 ? 120 : blocksize % 120); ++k)
		{
		    sidesects[254*j + 2*k + 14] = 1;
		    sidesects[254*j + 2*k + 15] = 1;
		}
	    }
	    size_t sslen = sidesectnum * 254;
	    if (FileData_append(archive, sidesects, sslen) < 0) goto done;
	    pos += sslen;
	}
	const FileData *fileData = CbmdosFile_rdata(file);
	size_t fileSize = FileData_size(fileData);
	if (FileData_append(archive,
		    FileData_rcontent(fileData), fileSize) < 0) goto done;
	pos += fileSize;
	if (i < filecount - 1)
	{
	    pad = 254 - (pos%254);
	    if (FileData_appendBytes(archive, 0, pad) < 0) goto done;
	    pos += pad;
	}
    }

    result = archive;
done:
    if (!result) FileData_destroy(archive);
    return result;
}

SOEXPORT FileData *archiveLynx(const CbmdosVfs *vfs)
{
    unsigned filecount = CbmdosVfs_fileCount(vfs);
    if (!filecount) return 0;
    const CbmdosFile **files = xmalloc(filecount * sizeof *files);
    unsigned realFilecount = 0;
    for (unsigned i = 0; i < filecount; ++i)
    {
	const CbmdosFile *file = CbmdosVfs_rfile(vfs, i);
	if (CbmdosFile_type(file) != CFT_DEL)
	{
	    files[realFilecount++] = file;
	}
    }
    if (!realFilecount)
    {
	free(files);
	return 0;
    }
    FileData *archive = archiveLynxFiles(files, realFilecount);
    free(files);
    return archive;
}

