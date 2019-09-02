#ifndef I1541_CBMDOSFS_H
#define I1541_CBMDOSFS_H

typedef enum CbmdosFsFlags
{
    CFF_COMPATIBLE = 0,
    CFF_ALLOWLONGDIR = 1 << 0,
    CFF_FILESONDIRTRACK = 1 << 1,
    CFF_40TRACK = 1 << 2,
    CFF_DOLPHINDOSBAM = 1 << 3,
    CFF_SPEEDDOSBAM = 1 << 4,
    CFF_ZEROFREE = 1 << 5,
} CbmdosFsFlags;

typedef struct CbmdosFsOptions
{
    CbmdosFsFlags flags;
    uint8_t dirInterleave;
    uint8_t fileInterleave;
} CbmdosFsOptions;

typedef enum CbmdosFsStatus
{
    CFS_OK = 0,
    CFS_INVALIDBAM = 1 << 0,
    CFS_DISKFULL = 1 << 1,
    CFS_DIRFULL = 1 << 2,
    CFS_BROKEN = 1 << 3
} CbmdosFsStatus;

extern const CbmdosFsOptions CFO_DEFAULT;

typedef struct CbmdosVfs CbmdosVfs;
typedef struct D64 D64;

typedef struct CbmdosFs CbmdosFs;

CbmdosFs *CbmdosFs_create(CbmdosFsOptions options);
CbmdosFs *CbmdosFs_fromImage(D64 *d64, CbmdosFsOptions options);
CbmdosFs *CbmdosFs_fromVfs(CbmdosVfs *vfs, CbmdosFsOptions options);
CbmdosFsStatus CbmdosFs_status(const CbmdosFs *self);
const CbmdosVfs *CbmdosFs_rvfs(const CbmdosFs *self);
CbmdosVfs *CbmdosFs_vfs(CbmdosFs *self);
const D64 *CbmdosFs_image(const CbmdosFs *self);
CbmdosFsOptions CbmdosFs_options(const CbmdosFs *self);
void CbmdosFs_setOptions(CbmdosFs *self, CbmdosFsOptions options);
int CbmdosFs_rewrite(CbmdosFs *self);
void CbmdosFs_destroy(CbmdosFs *self);

#endif
