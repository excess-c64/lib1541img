#ifndef I1541_ZC45WRITER_H
#define I1541_ZC45WRITER_H

#include <stddef.h>
#include <stdint.h>

#define MAXZCFILESIZE (175*258 + 2)

typedef struct D64 D64;

size_t zc45_write(uint8_t *zcfile, size_t zcfilelen, int zcfileno,
        const D64 *d64);

#endif
