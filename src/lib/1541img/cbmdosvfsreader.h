#ifndef CBMDOSVFSREADER_H
#define CBMDOSVFSREADER_H

#include <1541img/cbmdosvfsreader.h>

typedef struct DirData DirData;

int readCbmdosVfsInternal(CbmdosVfs *vfs, const D64 *d64,
	const CbmdosFsOptions *options,
	uint8_t (*bamdata)[21], DirData *dirdata);

#endif
