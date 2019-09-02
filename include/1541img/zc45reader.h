#ifndef I1541_ZC45READER_H
#define I1541_ZC45READER_H

#include <stddef.h>
#include <stdint.h>

typedef struct D64 D64;

int zc45_read(D64 *d64, const uint8_t *zcfile, size_t zcfilelen);

#endif
