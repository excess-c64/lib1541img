#ifndef I1541_CBMDOSFS_H
#define I1541_CBMDOSFS_H
#ifdef __cplusplus
extern "C" {
#endif

/** Declarations for the CbmdosFs class
 * @file
 */

#include <stdint.h>

#include <1541img/cbmdosfsoptions.h>

/** Status of a filesystem */
typedef enum CbmdosFsStatus
{
    CFS_OK = 0,                     /**< filesystem is valid */
    CFS_INVALIDBAM = 1 << 0,        /**< the BAM of the filesystem is invalid */
    CFS_DISKFULL = 1 << 1,          /**< the disk is full */
    CFS_DIRFULL = 1 << 2,           /**< no space left in directory */
    CFS_BROKEN = 1 << 3             /**< filesystem is invalid */
} CbmdosFsStatus;

/** Default options of a cbmdos filesystem.
 * An instance of CbmdosFsOptions with the following values:
 *
 * * flags: **CFF_COMPATIBLE**
 * * dirInterleave: **3**
 * * fileInterleave: **10**
 * @relates CbmdosFsOptions
 */
extern const CbmdosFsOptions CFO_DEFAULT;

typedef struct CbmdosVfs CbmdosVfs;
typedef struct D64 D64;

/** Class modeling a concrete cbmdos filesystem.
 * This models the concrete filesystem (mapping to the blocks of a D64 disk
 * image) in cbmdos format. It's connected to a D64 disk image, and to a
 * CbmdosVfs managing the actual files.
 * @class CbmdosFs cbmdosfs.h <1541img/cbmdosfs.h>
 */
typedef struct CbmdosFs CbmdosFs;

/** default constructor.
 * Creates a new cbmdos filesystem
 * @memberof CbmdosFs
 * @param options filesystem options
 * @returns a newly created CbmdosFs
 */
CbmdosFs *CbmdosFs_create(CbmdosFsOptions options);

/** Create CbmdosFs from a D64 disk image.
 * The filesystem is read from the given D64 image and associated with this
 * image, so after this call, the image will be owned by the CbmdosFs.
 * Therefore, you must not destroy the image yourself after constructing a
 * CbmdosFs from it.
 * @memberof CbmdosFs
 * @param d64 the disk image
 * @param options filesystem options, must be compatible with the image
 * @returns a CbmdosFs reflecting the disk image, or NULL on error
 */
CbmdosFs *CbmdosFs_fromImage(D64 *d64, CbmdosFsOptions options);

/** Create CbmdosFs from a cbmdos virtual filesystem
 * @memberof CbmdosFs
 * @param vfs the virtual filesystem
 * @param options filesystem options
 * @returns a CbmdosFs corresponding to the given vfs, or NULL on error
 */
CbmdosFs *CbmdosFs_fromVfs(CbmdosVfs *vfs, CbmdosFsOptions options);

/** Status of the filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns the current status
 */
CbmdosFsStatus CbmdosFs_status(const CbmdosFs *self);

/** Gets the read-only virtual filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a read-only pointer to the virtual filesystem
 */
const CbmdosVfs *CbmdosFs_rvfs(const CbmdosFs *self);

/** Gets the virtual filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a pointer to the virtual filesystem
 */
CbmdosVfs *CbmdosFs_vfs(CbmdosFs *self);

/** Gets the read-only disk image associated with this filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a read-only pointer to the D64 disk image
 */
const D64 *CbmdosFs_image(const CbmdosFs *self);

/** Gets the current options of the filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns the current options of the filesystem
 */
CbmdosFsOptions CbmdosFs_options(const CbmdosFs *self);

/** Sets options for the filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @param options the new options for the filesystem
 * @returns 0 on success, -1 on error (invalid options)
 */
int CbmdosFs_setOptions(CbmdosFs *self, CbmdosFsOptions options);

/** Re-writes the filesystem to the disk image
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns 0 on success, -1 on error
 */
int CbmdosFs_rewrite(CbmdosFs *self);

/** The number of free blocks in this filesystem.
 * This returns the number of free blocks as reported by cbmdos (optionally
 * with speeddos or dolphin dos for a 40track filesystem). Note that the option
 * CFF_ZEROFREE is ignored here.
 * If the filesystem is too full (so it can't be written to a disk), this will
 * return the special error value 0xffff.
 * This is a "best guess" implementation, that can return wrong values in
 * special cases like manipulated file block sizes or allowing to put files
 * on the directory track.
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns number of free blocks, or 0xffff on error
 */
uint16_t CbmdosFs_freeBlocks(const CbmdosFs *self);

/** Get the "free blocks message" as shown in the directory in the C64.
 * This gets the "xxx blocks free." message as shown in the directory, using
 * the number calculated by CbmdosFs_freeBlocks(), as a byte array containing
 * PETSCII characters. If CbmdosFs_freeBlocks() reports an error, this will
 * report "-1 blocks free.". Otherwise, if the option CFF_ZEROFREE is set, it
 * will report "0 blocks free."
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @param line a pointer to exactly 16 bytes, the line in PETSCII encoding
 *     will be written here, without any 0-termination
 */
void CbmdosFs_getFreeBlocksLine(const CbmdosFs *self, uint8_t *line);

/** CbmdosFs destructor
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 */
void CbmdosFs_destroy(CbmdosFs *self);

#ifdef __cplusplus
}
#endif
#endif
