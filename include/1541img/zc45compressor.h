#ifndef I1541_ZC45COMPRESSOR_H
#define I1541_ZC45COMPRESSOR_H

/** Contains a function for compressing a D64 disc image to zipcode
 * @file
 */

#include <1541img/decl.h>

C_CLASS_DECL(D64);
C_CLASS_DECL(ZcFileSet);

/** Compress a D64 disc image to zipcode.
 * @relatesalso ZcFileSet
 *
 *     #include <1541img/zc45compressor.h>
 *
 * This compresses a D64 disc image to a 4-pack (35 tracks) or 5-pack
 * (40 tracks) zipcode file set.
 * @param d64 the disc image to compress
 * @returns the zipcode file set, or NULL on error
 */
DECLEXPORT ZcFileSet *compressZc45(const D64 *d64);

#endif
