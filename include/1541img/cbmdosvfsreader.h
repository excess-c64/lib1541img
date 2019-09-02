#ifndef I1541_CBMDOSVFSREADER_H
#define I1541_CBMDOSVFSREADER_H

typedef struct D64 D64;
typedef struct CbmdosVfs CbmdosVfs;

int readCbmdosVfs(CbmdosVfs *vfs, const D64 *d64);

#endif
