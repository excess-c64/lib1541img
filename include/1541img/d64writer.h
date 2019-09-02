#ifndef I1541_D64WRITER_H
#define I1541_D64WRITER_H

#include <stdio.h>

typedef struct D64 D64;

int writeD64(FILE *file, const D64 *d64);

#endif
