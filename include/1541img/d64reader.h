#ifndef I1541_D64READER_H
#define I1541_D64READER_H

#include <stdio.h>

typedef struct D64 D64;

D64 *readD64(FILE *file);

#endif
