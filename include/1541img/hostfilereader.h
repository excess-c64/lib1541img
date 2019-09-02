#ifndef I1541_HOSTFILEREADER_H
#define I1541_HOSTFILEREADER_H

#include <stdio.h>

typedef struct FileData FileData;

FileData *readHostFile(FILE *file);

#endif
