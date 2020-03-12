#ifndef I1541_CBMDOSFSOPTIONS_H
#define I1541_CBMDOSFSOPTIONS_H

/** Declaration of CbmdosFs Options
 * @file
 */

#include <stdint.h>

#include <1541img/decl.h>

/** Some flags defining behavior of the filesystem.
 * Most of them can be combined freely. CFF_42TRACK will override CFF_40TRACK
 * if both are set. It is invalid to combine CFF_PROLOGICDOSBAM with any of
 * the other extended BAM options, because ProLogic DOS uses a different BAM
 * layout where the disk name is relocated somewhere else.
 *
 * CFF_TALLOC_TRACKLOAD and CFF_TALLOC_SIMPLE are different track allocation
 * strategies that also can't be combined. If none of them is set, the
 * strategy of the original CBM DOS is used: The first available free sector
 * is searched for on a track as close as possible to track 18, starting from
 * track 17 (then 19, then 16, etc). This is a good strategy for random
 * access to files and prefers the files at the top of the directory.
 *
 * CFF_TALLOC_TRACKLOAD is suitable for files that are always read in the same
 * order, like it is done in many demos -- it will look for the first free
 * sector starting at track 1.
 *
 * CFF_TALLOC_SIMPLE is a compromise between trackload and the original
 * "nearest track" strategy.
 *
 * CFF_TALLOC_PREFDIRTRACK modifies any track allocation strategy by using
 * track 18 first for files. It only has an effect if CFF_FILESONDIRTRACK is
 * set as well and is probably a bad idea with the CFF_TALLOC_TRACKLOAD
 * strategy, because the head would have to seek from track 18 to track 1 in
 * the middle of loading.
 *
 * If CFF_SIMPLEINTERLEAVE is set, interleave values are applied as one
 * would expect, by adding the interleave and taking the result modulo the
 * number of sectors.
 *
 * Otherwise, interleave is applied in a more complicated way the original
 * CBM DOS uses: If applying the interleave wraps over sector 0, the result is
 * decremented by 1.
 */
typedef enum CbmdosFsFlags
{
    CFF_COMPATIBLE = 0,             /**< no flags, 100% compatible to cbmdos */
    CFF_ALLOWLONGDIR = 1 << 0,      /**< long directories (more than one track)
                                         allowed */
    CFF_FILESONDIRTRACK = 1 << 1,   /**< allow files to be placed on the
                                         directory track */
    CFF_40TRACK = 1 << 2,           /**< filesystem spans over 40 tracks
                                         (instead of the default 35) */
    CFF_42TRACK = 1 << 3,           /**< filesystem spans over 42 tracks
                                         (instead of the default 35) */
    CFF_DOLPHINDOSBAM = 1 << 4,     /**< create a BAM for extended tracks in
                                         dolphindos format */
    CFF_SPEEDDOSBAM = 1 << 5,       /**< create a BAM for extended tracks in
                                         speeddos format */
    CFF_PROLOGICDOSBAM = 1 << 6,    /**< create a BAM for extended tracks in
                                         prologic format */
    CFF_ZEROFREE = 1 << 7,          /**< report no free blocks in directory */
    CFF_RECOVER = 1 << 8,	    /**< when reading from disk image, try
				         to recover a broken filesystem */
    CFF_OVERRIDE_INTERLEAVE = 0x1ff,/**< when used as per-file override, the
                                         mask for the interleave value */
    CFF_SIMPLEINTERLEAVE = 1 << 9,  /**< when applying interleave, use a
                                         simple modulo */
    CFF_TALLOC_TRACKLOAD = 1 << 10, /**< search for first free sector starting
                                         from track 1 */
    CFF_TALLOC_SIMPLE = 1 << 11,    /**< search for first free sector starting
                                         from track 19, wrapping around to
                                         track 1, but still don't use tracks
                                         41 and 42 unless no other tracks have
                                         free sectors */
    CFF_TALLOC_PREFDIRTRACK = 1 << 12, /**< if files on dir track are allowed,
                                         put files there first */
    CFF_TALLOC_CHAININTERLV = 1 << 13, /**< when having to change the track
                                         while chaining a file, still apply
                                         interleave instead of starting at
                                         sector 0 on the new track */
} CbmdosFsFlags;

/** Filesystem options
 * @struct CbmdosFsOptions cbmdosfsoptions.h <1541img/cbmdosfsoptions.h>
 */
C_CLASS_DECL(CbmdosFsOptions);

struct CbmdosFsOptions
{
    CbmdosFsFlags flags;            /**< filesystem flags */
    uint8_t dirInterleave;          /**< sector interleave to use for
                                         directory */
    uint8_t fileInterleave;         /**< sector interleave to use for files */
};

/** Per file overrides of filesystem options.
 * @struct CbmdosFsOptOverrides cbmdosfsoptions.h <1541img/cbmdosfsoptions.h>
 *
 * This struct contains two sets of flags, one containing actual flag values,
 * the other determining which of the flags should override the ones from the
 * global filesystem options.
 *
 * Flags CFF_ALLOWLONDIR to CFF_RECOVER (Bits 0 to 8) cannot be overridden and
 * are used here to store an overridden file interleave value instead.
 */
C_CLASS_DECL(CbmdosFsOptOverrides);

struct CbmdosFsOptOverrides
{
    CbmdosFsFlags flags;    /**< the overridden flag values */
    CbmdosFsFlags mask;     /**< the mask telling which flags to override */
};

/** Default value for option overrides (no overrides)
 */
#define CFOO_NONE ((CbmdosFsOptOverrides){ 0, 0 })

/** Apply overrides to given options.
 * @memberof CbmdosFsOptions
 * @param self the filesystem options
 * @param overrides the overrides to apply
 */
void CbmdosFsOptions_applyOverrides(CbmdosFsOptions *self,
        const CbmdosFsOptOverrides *overrides);

/** Overridden file interleave value.
 * @memberof CbmdosFsOptOverrides
 * @param self the filesystem option overrides
 * @returns the overridden interleave value, or 0xff if not overridden
 */
uint8_t CbmdosFsOptOverrides_interleave(const CbmdosFsOptOverrides *self);

/** Set overridden file interleave value.
 * @memberof CbmdosFsOptOverrides
 * @param self the filesystem option overrides
 * @param interleave overriden interleave value, or 0xff to not override
 */
void CbmdosFsOptOverrides_setInterleave(
        CbmdosFsOptOverrides *self, uint8_t interleave);

#endif
