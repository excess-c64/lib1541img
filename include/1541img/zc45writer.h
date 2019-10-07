#ifndef I1541_ZC45WRITER_H
#define I1541_ZC45WRITER_H
#ifdef __cplusplus
extern "C" {
#endif

/** Contains a function for compressing a single 4-pack or 5-pack zipcode file
 * @file
 */

#include <stddef.h>
#include <stdint.h>

/** The maximum size a single 4-pack or 5-pack zipcode file can have
 * @relatesalso D64
 */
#define MAXZCFILESIZE (175*258 + 2)

typedef struct D64 D64;

/** Compress a single 4-pack or 5-pack zipcode file from a given D64 disc image.
 * @relatesalso D64
 *
 *     #include <1541img/zc45writer.h>
 * 
 * This function writes sectors from a given D64 disc image compressed to a
 * single 4-pack or 5-pack zipcode file.
 * @param zcfile a pointer to the raw bytes of a zipcode file. You should
 *     provide a buffer of size MAXZCFILESIZE here to be sure the call
 *     succeeds.
 * @param zcfilelen the size of the buffer provided for zcfile
 * @param zcfileno the index number of the zipcode file
 * @param d64 the D64 disc image to read sectors from
 * @returns the size of the zipcode file, or 0 on error
 */
size_t zc45_write(uint8_t *zcfile, size_t zcfilelen, int zcfileno,
        const D64 *d64);

#ifdef __cplusplus
}
#endif
#endif
