#ifndef I1541_CBMDOSFS_H
#define I1541_CBMDOSFS_H

/** Declarations for the CbmdosFs class
 * @file
 */

#include <stdint.h>

#include <1541img/decl.h>

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
DECLDATA DECLEXPORT const CbmdosFsOptions CFO_DEFAULT;

C_CLASS_DECL(CbmdosVfs);
C_CLASS_DECL(D64);

/** Class modeling a concrete cbmdos filesystem.
 * This models the concrete filesystem (mapping to the blocks of a D64 disk
 * image) in cbmdos format. It's connected to a D64 disk image, and to a
 * CbmdosVfs managing the actual files.
 * @class CbmdosFs cbmdosfs.h <1541img/cbmdosfs.h>
 */
C_CLASS_DECL(CbmdosFs);

/** default constructor.
 * Creates a new cbmdos filesystem
 * @memberof CbmdosFs
 * @param options filesystem options
 * @returns a newly created CbmdosFs
 */
DECLEXPORT CbmdosFs *CbmdosFs_create(CbmdosFsOptions options);

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
DECLEXPORT CbmdosFs *CbmdosFs_fromImage(D64 *d64, CbmdosFsOptions options);

/** Create CbmdosFs from a cbmdos virtual filesystem
 * @memberof CbmdosFs
 * @param vfs the virtual filesystem
 * @param options filesystem options
 * @returns a CbmdosFs corresponding to the given vfs, or NULL on error
 */
DECLEXPORT CbmdosFs *CbmdosFs_fromVfs(CbmdosVfs *vfs, CbmdosFsOptions options);

/** Status of the filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns the current status
 */
DECLEXPORT CbmdosFsStatus CbmdosFs_status(const CbmdosFs *self);

/** Gets the read-only virtual filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a read-only pointer to the virtual filesystem
 */
DECLEXPORT const CbmdosVfs *CbmdosFs_rvfs(const CbmdosFs *self);

/** Gets the virtual filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a pointer to the virtual filesystem
 */
DECLEXPORT CbmdosVfs *CbmdosFs_vfs(CbmdosFs *self);

/** Gets the read-only disk image associated with this filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns a read-only pointer to the D64 disk image
 */
DECLEXPORT const D64 *CbmdosFs_image(const CbmdosFs *self);

/** Gets the current options of the filesystem
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns the current options of the filesystem
 */
DECLEXPORT CbmdosFsOptions CbmdosFs_options(const CbmdosFs *self);

/** Sets options for the filesystem.
 * If necessary for the options to apply, this will automatically update the
 * disk's BAM or even rewrite the entire disk image. If any changes were made
 * to the disk image, it's indicated by a return value of 1.
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @param options the new options for the filesystem
 * @returns 1 if any changes were made to the disk image, 0 otherwise,
 *     -1 on error (invalid options)
 */
DECLEXPORT int CbmdosFs_setOptions(CbmdosFs *self, CbmdosFsOptions options);

/** Check whether changed options would rewrite the image.
 * This can be used to determine whether a full rewrite of the disk image will
 * be triggered when setting new filesystem options. Use this if you want to
 * warn the user before setting options that will cause a full rewrite. If
 * only the BAM would be updated, this function will still return 0.
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @param options the new options for the filesystem
 * @returns 1 if the options will trigger a full rewrite, 0 otherwise and
 *     -1 on error (invalid options)
 */
DECLEXPORT int CbmdosFs_optionsWillRewrite(
	const CbmdosFs *self, CbmdosFsOptions options);

/** Re-writes the filesystem to the disk image
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFs_rewrite(CbmdosFs *self);

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
DECLEXPORT uint16_t CbmdosFs_freeBlocks(const CbmdosFs *self);

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
DECLEXPORT void CbmdosFs_getFreeBlocksLine(const CbmdosFs *self, uint8_t *line);

/** CbmdosFs destructor
 * @memberof CbmdosFs
 * @param self the cbmdos filesystem
 */
DECLEXPORT void CbmdosFs_destroy(CbmdosFs *self);

#endif
