#ifndef I1541_TRACK_H
#define I1541_TRACK_H

/** declarations for the Track class
 * @file
 */

#include <stdint.h>

#include <1541img/decl.h>

C_CLASS_DECL(Sector);

/** A track on a disk image
 * @class Track track.h <1541img/track.h>
 */
C_CLASS_DECL(Track);

/** Track default constructor
 * @memberof Track
 * @param tracknum number of the track (starting at 1)
 * @returns a newly created Track
 */
DECLEXPORT Track *Track_create(uint8_t tracknum);

/** Gets a read-only Sector
 * @memberof Track
 * @param self the Track
 * @param sectornum number of the sector (starting at 0)
 * @returns a read-only pointer to the sector (or NULL on error)
 */
DECLEXPORT const Sector *Track_rsector(const Track *self, uint8_t sectornum);

/** Gets a Sector
 * @memberof Track
 * @param self the Track
 * @param sectornum number of the sector (starting at 0)
 * @returns a pointer to the sector (or NULL on error)
 */
DECLEXPORT Sector *Track_sector(Track *self, uint8_t sectornum);

/** Number of the track
 * @memberof Track
 * @param self the Track
 * @returns the number of the track (starting at 1)
 */
DECLEXPORT uint8_t Track_trackNum(const Track *self);

/** Number of sectors on the track
 * @memberof Track
 * @param self the Track
 * @returns the number of sectors on this track
 */
DECLEXPORT uint8_t Track_sectors(const Track *self);

/** Track destructor
 * @memberof Track
 * @param self the Track
 */
DECLEXPORT void Track_destroy(Track *self);

#endif
