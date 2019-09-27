#ifndef I1541_ZC45READER_H
#define I1541_ZC45READER_H

/** Contains a function for extracting a single 4-pack or 5-pack zipcode file
 * @file
 */

#include <stddef.h>
#include <stdint.h>

typedef struct D64 D64;

/** Extract a single 4-pack or 5-pack zipcode file to a given D64 disc image.
 * @relatesalso D64
 *
 *     #include <1541img/zc45reader.h>
 *
 * This function reads compressed sectors from a single 4-pack or 5-pack
 * zipcode file and writes them uncompressed to the given D64 disc image.
 * @param d64 the D64 disc image to write the sectors to
 * @param zcfile the raw bytes of a zipcode file to read compresse sectors from
 * @param zcfilelen the length of the zipcode file
 * @returns 0 on success, -1 on error
 */
int zc45_read(D64 *d64, const uint8_t *zcfile, size_t zcfilelen);

#endif
