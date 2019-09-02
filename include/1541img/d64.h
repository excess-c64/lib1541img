#ifndef I1541_D64_H
#define I1541_D64_H

#include <stdint.h>

typedef enum D64Type
{
    D64_STANDARD,
    D64_40TRACK
} D64Type;

typedef struct D64 D64;
typedef struct Track Track;
typedef struct Sector Sector;

D64 *D64_create(D64Type type);
D64Type D64_type(const D64 *self);
uint8_t D64_tracks(const D64 *self);
const Track *D64_rtrack(const D64 *self, uint8_t tracknum);
Track *D64_track(D64 *self, uint8_t tracknum);
const Sector *D64_rsector(const D64 *self, uint8_t tracknum, uint8_t sectornum);
Sector *D64_sector(D64 *self, uint8_t tracknum, uint8_t sectornum);
void D64_destroy(D64 *self);

#endif
