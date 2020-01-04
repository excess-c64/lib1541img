#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "util.h"
#include "log.h"
#include "filename.h"

#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>
#include <1541img/d64.h>
#include <1541img/d64reader.h>
#include <1541img/cbmdosvfs.h>
#include <1541img/cbmdosfile.h>
#include <1541img/cbmdosvfsreader.h>

#include <1541img/zcfileset.h>

struct ZcFileSet
{
    ZcType type;
    int count;
    char *name;
    FileData *files[];
};

ZcFileSet *ZcFileSet_create(ZcType type, const char *name)
{
    int count;
    switch (type)
    {
        case ZT_4PACK:
            count = 4;
            break;
        case ZT_5PACK:
            count = 5;
            break;
        case ZT_6PACK:
            count = 6;
            break;
        default:
            logmsg(L_ERROR, "ZcFileSet: invalid type.");
            return 0;
    }

    ZcFileSet *self = xmalloc(sizeof *self + count * sizeof *self->files);
    self->type = type;
    self->count = count;
    self->name = copystr(name);
    for (int i = 0; i < count; ++i) self->files[i] = FileData_create();
    return self;
}

static ZcFileSet *fromFileData(const char *name, FileData **files)
{
    ZcFileSet *self = 0;

    if (files[0] && files[1] && files[2] && files[3])
    {
        if (files[4])
        {
            logmsg(L_INFO, "ZcFileSet: 5-file disk-packed zipcode "
                    "read successfully.");
            self = xmalloc(sizeof *self + 5 * sizeof *self->files);
            self->type = ZT_5PACK;
            self->count = 5;
            memcpy(self->files, files, 5 * sizeof *self->files);
        }
        else
        {
            logmsg(L_INFO, "ZcFileSet: 4-file disk-packed zipcode "
                    "read successfully.");
            self = xmalloc(sizeof *self + 4 * sizeof *self->files);
            self->type = ZT_4PACK;
            self->count = 4;
            memcpy(self->files, files, 4 * sizeof *self->files);
        }
        self->name = copystr(name);
    }
    else
    {
        logmsg(L_ERROR, "ZcFileSet: reading failed (missing or "
                "unreadable files).");
        for (int i = 0; i < 5; ++i) FileData_destroy(files[i]);
    }

    return self;
}

static ZcFileSet *fromD64(const char *filename)
{
    FILE *d64file = fopen(filename, "rb");
    if (!d64file)
    {
        logfmt(L_ERROR, "ZcFileSet: error reading `%s'.", filename);
        return 0;
    }
    D64 *d64 = readD64(d64file);
    fclose(d64file);
    if (!d64) return 0;
    CbmdosVfs *vfs = CbmdosVfs_create();
    int rc = readCbmdosVfs(vfs, d64, 0);
    D64_destroy(d64);
    if (rc < 0)
    {
        CbmdosVfs_destroy(vfs);
        return 0;
    }

    logfmt(L_INFO, "ZcFileSet: checking disk `%s,%s'", CbmdosVfs_name(vfs, 0),
            CbmdosVfs_id(vfs, 0));

    const char *compare = 0;
    FileData *files[5] = { 0 };
    for (unsigned pos = 0; pos < CbmdosVfs_fileCount(vfs); ++pos)
    {
        const CbmdosFile *file = CbmdosVfs_rfile(vfs, pos);
        if (CbmdosFile_type(file) != CFT_PRG) continue;
        uint8_t namelen;
        const char *name = CbmdosFile_name(file, &namelen);
        if (namelen < 3) continue;
        if (name[1] != '!' || name[2] == '!') continue;
        if (name[0] < '1' || name[0] > '5') continue;
        if (compare)
        {
            if (strcmp(name+2, compare)) continue;
        }
        else compare = name+2;
        int partidx = name[0] - '1';
        if (files[partidx])
        {
            logfmt(L_WARNING, "ZcFileSet: skipping duplicate `%s.PRG'", name);
            continue;
        }
        files[partidx] = FileData_clone(CbmdosFile_rdata(file));
        logfmt(L_INFO, "ZcFileSet: found `%s.PRG' (%lu bytes)", name,
                (unsigned long)FileData_size(files[partidx]));
    }

    ZcFileSet *self = fromFileData(compare, files);
    CbmdosVfs_destroy(vfs);
    return self;
}

ZcFileSet *ZcFileSet_fromFile(const char *filename)
{
    ZcFileSet *self = 0;
    Filename *fn = Filename_create();
    Filename_setFull(fn, filename);
    char *ext = upperstr(Filename_ext(fn));
    if (!ext || !strcmp(ext, "PRG"))
    {
	char *base = copystr(Filename_base(fn));
	if (base[0] >= '1' && base[0] <= '5'
		&& base[1] == '!' && base[2] != '!')
	{
            logmsg(L_INFO, "ZcFileSet: 4/5-file disk-packed zipcode "
                    "detected, looking for all member files...");
	    FileData *files[5] = { 0 };
	    for (base[0] = '1'; base[0] <= '5'; ++base[0])
	    {
		Filename *pn = Filename_clone(fn);
		Filename_setBase(pn, base);
                const char *ffn = Filename_full(pn);
                logfmt(L_INFO, "ZcFileSet: trying to read `%s'.", ffn);
		FILE *p = fopen(ffn, "rb");
		if (p)
		{
		    files[base[0]-'1'] = readHostFile(p);
		    fclose(p);
		}
	    }
	    self = fromFileData(base+2, files);
	}
        else
        {
            logmsg(L_WARNING, "ZcFileSet: no known ZipCode structure found.");
        }
	free(base);
    }
    else if (!strcmp(ext, "D64"))
    {
        self = fromD64(filename);
    }
    else
    {
        logmsg(L_WARNING, "ZcFileSet: no known ZipCode structure found.");
    }
    free(ext);
    return self;
}

ZcType ZcFileSet_type(const ZcFileSet *self)
{
    return self->type;
}

int ZcFileSet_count(const ZcFileSet *self)
{
    return self->count;
}

const char *ZcFileSet_name(const ZcFileSet *self)
{
    return self->name;
}

static int checkIndex(const ZcFileSet *self, int index)
{
    if (index < 0 || index >= self->count)
    {
        logfmt(L_ERROR, "ZcFileSet: non-existing member file %d requested.",
                index);
        return -1;
    }
    return 0;
}

FileData *ZcFileSet_fileData(ZcFileSet *self, int index)
{
    if (checkIndex(self, index) < 0) return 0;
    return self->files[index];
}

const FileData *ZcFileSet_rfileData(const ZcFileSet *self, int index)
{
    if (checkIndex(self, index) < 0) return 0;
    return self->files[index];
}

int ZcFileSet_save(const ZcFileSet *self, const char *filename)
{
    Filename *fn = Filename_create();
    Filename_setFull(fn, filename);
    const char *basename = self->name;
    const char *fnbase = Filename_base(fn);
    if (fnbase)
    {
        if (fnbase[0] && fnbase[1] == '!') basename = fnbase+2;
        else basename = fnbase;
    }

    char *nextname = xmalloc(strlen(basename) + 3);
    nextname[1] = '!';
    strcpy(nextname+2, basename);

    const char *ext = Filename_ext(fn);
    if (!ext) Filename_setExt(fn, "prg");

    for (nextname[0] = '1';
            nextname[0] < (self->type == ZT_5PACK ? '6' : '5');
            ++nextname[0])
    {
        Filename_setBase(fn, nextname);
        FILE *f = fopen(Filename_full(fn), "wb");
        if (!f)
        {
            free(nextname);
            logfmt(L_ERROR, "ZcFileSet_save: error opening `%s' for writing.",
                    Filename_full(fn));
            Filename_destroy(fn);
            return -1;
        }
        const FileData *fd = self->files[nextname[0]-'1'];
        if (!fwrite(FileData_rcontent(fd), FileData_size(fd), 1, f))
        {
            fclose(f);
            free(nextname);
            logfmt(L_ERROR, "ZcFileSet_save: error writing to `%s'.",
                    Filename_full(fn));
            Filename_destroy(fn);
            return -1;
        }
        logfmt(L_INFO, "ZcFileSet_save: saved `%s'.", Filename_full(fn));
        fclose(f);
    }
    free(nextname);
    Filename_destroy(fn);
    return 0;
}

int ZcFileSet_saveVfs(const ZcFileSet *self, CbmdosVfs *vfs)
{
    char name[16];
    uint8_t baselen = strlen(self->name) > 14 ? 14 : strlen(self->name);
    memcpy(name+2, self->name, baselen);
    name[1] = '!';
    for (name[0] = '1'; name[0] < (self->type == ZT_5PACK ? '6' : '5');
	    ++name [0])
    {
	CbmdosFile *file = CbmdosFile_create();
	CbmdosFile_setName(file, name, baselen+2);
	FileData *cfd = CbmdosFile_data(file);
	const FileData *fd = self->files[name[0]-'1'];
	if (FileData_append(cfd, FileData_rcontent(fd), FileData_size(fd)) < 0)
	{
	    return -1;
	}
	if (CbmdosVfs_append(vfs, file))
	{
	    return -1;
	}
    }
    return 0;
}

void ZcFileSet_destroy(ZcFileSet *self)
{
    if (!self) return;
    free(self->name);
    for (int i = 0; i < self->count; ++i) FileData_destroy(self->files[i]);
    free(self);
}

