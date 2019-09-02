#ifndef I1541_SECTOR_H
#define I1541_SECTOR_H

#include <stdint.h>

#define SECTOR_SIZE 256

typedef struct Sector Sector;

Sector *Sector_create(void);
Sector *Sector_createAt(uint8_t tracknum, uint8_t sectornum);
const uint8_t *Sector_rcontent(const Sector *self);
uint8_t *Sector_content(Sector *self);
uint8_t Sector_trackNum(const Sector *self);
uint8_t Sector_sectorNum(const Sector *self);
void Sector_setTrackNum(Sector *self, uint8_t tracknum);
void Sector_setSectorNum(Sector *self, uint8_t sectornum);
void Sector_destroy(Sector *self);

#endif
