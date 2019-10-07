#ifndef I1541_D64_H
#define I1541_D64_H
#ifdef __cplusplus
extern "C" {
#endif

/** declarations for the D64 class
 * @file
 */

#include <stdint.h>

/** The type of a D64 disk image
 */
typedef enum D64Type
{
    D64_STANDARD, /**< standard image with 35 tracks */
    D64_40TRACK   /**< extended image with 40 tracks */
} D64Type;

/** A D64 disk image.
 * @class D64 d64.h <1541img/d64.h>
 */
typedef struct D64 D64;
typedef struct Track Track;
typedef struct Sector Sector;

/** D64 default constructor.
 * Creates a new D64 image
 * @memberof D64
 * @param type the type of the D64
 * @returns a newly created D64 image
 */
D64 *D64_create(D64Type type);

/** Type of the D64
 * @memberof D64
 * @param self the D64 image
 * @returns the type of this D64 image
 */
D64Type D64_type(const D64 *self);

/** Number of tracks on the D64 image
 * @memberof D64
 * @param self the D64 image
 * @returns the number of tracks on this D64 image
 */
uint8_t D64_tracks(const D64 *self);

/** Gets a read-only Track
 * @memberof D64
 * @param self the D64 image
 * @param tracknum number of the track (starting at 1)
 * @returns a read-only pointer to the track (or NULL on error)
 */
const Track *D64_rtrack(const D64 *self, uint8_t tracknum);

/** Gets a Track
 * @memberof D64
 * @param self the D64 image
 * @param tracknum number of the track (starting at 1)
 * @returns a pointer to the track (or NULL on error)
 */
Track *D64_track(D64 *self, uint8_t tracknum);

/** Gets a read-only Sector
 * @memberof D64
 * @param self the D64 image
 * @param tracknum number of the track (starting at 1)
 * @param sectornum number of the sector (starting at 0)
 * @returns a read-only pointer to the sector (or NULL on error)
 */
const Sector *D64_rsector(const D64 *self, uint8_t tracknum, uint8_t sectornum);

/** Gets a Sector
 * @memberof D64
 * @param self the D64 image
 * @param tracknum number of the track (starting at 1)
 * @param sectornum number of the sector (starting at 0)
 * @returns a pointer to the sector (or NULL on error)
 */
Sector *D64_sector(D64 *self, uint8_t tracknum, uint8_t sectornum);

/** D64 destructor
 * @memberof D64
 * @param self the D64 image
 */
void D64_destroy(D64 *self);

#ifdef __cplusplus
}
#endif
#endif
