#ifndef DIRDATA_H
#define DIRDATA_H

#include <stdint.h>

#define DIRCHUNK 144

typedef struct DirEntry
{
    uint16_t blocks;
    uint8_t starttrack;
    uint8_t startsector;
    uint8_t sidetrack;
    uint8_t sidesector;
} DirEntry;

typedef struct DirData
{
    DirEntry *entries;
    unsigned size;
    unsigned capa;
} DirData;

#endif
