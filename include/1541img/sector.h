#ifndef I1541_SECTOR_H
#define I1541_SECTOR_H

/** declarations for the Sector class
 * @file
 */

#include <stdint.h>

#include <1541img/decl.h>

/** The raw size of a sector */
#define SECTOR_SIZE 256

/** A sector on a disk image
 * @class Sector sector.h <1541img/sector.h>
 */
C_CLASS_DECL(Sector);

/** Sector default constructor
 * @memberof Sector
 * @returns a newly created Sector
 */
DECLEXPORT Sector *Sector_create(void);

/** Sector constructor at given position
 * Creates a new Sector with a given track and sector number
 * @memberof Sector
 * @param tracknum the track number (starting at 1)
 * @param sectornum the sector number (starting at 0)
 * @returns a newly created Sector
 */
DECLEXPORT Sector *Sector_createAt(uint8_t tracknum, uint8_t sectornum);

/** Gets the read-only content of the Sector
 * @memberof Sector
 * @param self the Sector
 * @returns a read-only pointer to 256 bytes of sector content
 */
DECLEXPORT const uint8_t *Sector_rcontent(const Sector *self);

/** Gets the content of the Sector
 * @memberof Sector
 * @param self the Sector
 * @returns a pointer to 256 bytes of sector content
 */
DECLEXPORT uint8_t *Sector_content(Sector *self);

/** The track number
 * @memberof Sector
 * @param self the Sector
 * @returns the track number of this sector (starting at 1)
 */
DECLEXPORT uint8_t Sector_trackNum(const Sector *self);

/** The sector number
 * @memberof Sector
 * @param self the Sector
 * @returns the sector number of this sector (starting at 0)
 */
DECLEXPORT uint8_t Sector_sectorNum(const Sector *self);

/** Set the track number
 * @memberof Sector
 * @param self the Sector
 * @param tracknum the track number to set (starting at 1)
 */
DECLEXPORT void Sector_setTrackNum(Sector *self, uint8_t tracknum);

/** Set the sector number
 * @memberof Sector
 * @param self the Sector
 * @param sectornum the sector number to set (starting at 0)
 */
DECLEXPORT void Sector_setSectorNum(Sector *self, uint8_t sectornum);

/** Sector destructor
 * @memberof Sector
 * @param self the Sector
 */
DECLEXPORT void Sector_destroy(Sector *self);

#endif
