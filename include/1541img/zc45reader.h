#ifndef I1541_ZC45READER_H
#define I1541_ZC45READER_H

/** Contains a function for extracting a single 4-pack or 5-pack zipcode file
 * @file
 */

#include <stddef.h>
#include <stdint.h>

#include <1541img/decl.h>

C_CLASS_DECL(D64);

/** Extract a single 4-pack or 5-pack zipcode file to a given D64 disc image.
 * @relatesalso D64
 *
 *     #include <1541img/zc45reader.h>
 *
 * This function reads compressed sectors from a single 4-pack or 5-pack
 * zipcode file and writes them uncompressed to the given D64 disc image.
 * @param d64 the D64 disc image to write the sectors to
 * @param sectors the number of sectors to read, or -1 to read up to the end
 *     of the zipcode file contents.
 * @param zcfile the raw bytes of a zipcode file to read compressed
 *     sectors from
 * @param zcfilelen the length of the zipcode file
 * @returns the number of sectors actually read, or -1 on error
 */
DECLEXPORT int zc45_read(
        D64 *d64, int sectors, const uint8_t *zcfile, size_t zcfilelen);

#endif
