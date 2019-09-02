#ifndef I1541_ZCFILESET_H
#define I1541_ZCFILESET_H

typedef enum ZcType
{
    ZT_4PACK,
    ZT_5PACK,
    ZT_6PACK
} ZcType;

typedef struct FileData FileData;
typedef struct CbmdosVfs CbmdosVfs;

typedef struct ZcFileSet ZcFileSet;

ZcFileSet *ZcFileSet_create(ZcType type, const char *name);
ZcFileSet *ZcFileSet_fromFile(const char *filename);
ZcType ZcFileSet_type(const ZcFileSet *self);
int ZcFileSet_count(const ZcFileSet *self);
const char *ZcFileSet_name(const ZcFileSet *self);
const FileData *ZcFileSet_rfileData(const ZcFileSet *self, int index);
FileData *ZcFileSet_fileData(ZcFileSet *self, int index);
int ZcFileSet_save(const ZcFileSet *self, const char *filename);
int ZcFileSet_saveVfs(const ZcFileSet *self, CbmdosVfs *vfs);
void ZcFileSet_destroy(ZcFileSet *self);

#endif
