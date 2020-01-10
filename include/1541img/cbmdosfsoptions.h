#ifndef I1541_CBMDOSFSOPTIONS_H
#define I1541_CBMDOSFSOPTIONS_H

/** Declaration of CbmdosFs Options
 * @file
 */

#include <1541img/decl.h>

/** Some flags defining behavior of the filesystem.
 * Most of them can be combined freely. CFF_42TRACK will override CFF_40TRACK
 * if both are set. It is invalid to combine CFF_PROLOGICDOSBAM with any of
 * the other extended BAM options, because ProLogic DOS uses a different BAM
 * layout where the disk name is relocated somewhere else.
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
    CFF_42TRACK = 1 << 3,           /**< filesystem spans over 40 tracks
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

#endif
