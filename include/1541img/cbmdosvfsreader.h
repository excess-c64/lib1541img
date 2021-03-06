#ifndef I1541_CBMDOSVFSREADER_H
#define I1541_CBMDOSVFSREADER_H

/** Contains a function to read a cbmdos vfs from a D64 disc image
 * @file
 */

#include <1541img/decl.h>

C_CLASS_DECL(D64);
C_CLASS_DECL(CbmdosVfs);
C_CLASS_DECL(CbmdosFsOptions);

/** Read a cbmdos vfs from a D64 disc image.
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/cbmdosvfsreader.h>
 *
 * @param vfs an empty cbmdos vfs. Reading will fail if you pass a non-empty
 *    vfs.
 * @param d64 the D64 disc image to read from
 * @param options cbmdos filesystem options to use when reading. Probed
 *    options are assumed when passing NULL here.
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int readCbmdosVfs(
	CbmdosVfs *vfs, const D64 *d64, const CbmdosFsOptions *options);

/** Determine cbmdos fs options needed to read from a D64 disc image.
 * @relatesalso CbmdosFsOptions
 *
 *     #include <1541img/cbmdosvfsreader.h>
 * 
 * This functions tries to find a cbmdos filesystem on the given disc image
 * and determine the correct CbmdosFsOptions for reading it. The result is
 * just a "best guess", but it should normally work for reading from the
 * image. If the filesystem on the disc image is broken/inconsistent, this
 * function will fail unless the CFF_RECOVER flag in the options passed is
 * already set.
 * @param options a pointer to the options to probe. This *must* be properly
 *     initialized, because it is checked for the CFF_RECOVER flag.
 * @param d64 the D64 disc image to read from
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int probeCbmdosFsOptions(CbmdosFsOptions *options, const D64 *d64);

#endif
