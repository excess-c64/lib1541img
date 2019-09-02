#ifndef I1541_TRACK_H
#define I1541_TRACK_H

#include <stdint.h>

typedef struct Track Track;
typedef struct Sector Sector;

Track *Track_create(uint8_t tracknum);
const Sector *Track_rsector(const Track *self, uint8_t sectornum);
Sector *Track_sector(Track *self, uint8_t sectornum);
uint8_t Track_trackNum(const Track *self);
uint8_t Track_sectors(const Track *self);
void Track_destroy(Track *self);

#endif
