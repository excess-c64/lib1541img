#ifndef I1541_D64WRITER_H
#define I1541_D64WRITER_H

/** Contains a function to write a D64 disc image to a (host) file
 * @file
 */

#include <stdio.h>

typedef struct D64 D64;

/** Write a D64 disc image to a (host) file.
 * @relatesalso D64
 *
 *     #include <1541img/d64writer.h>
 *
 * @param file a file opened for writing to write the disc image to
 * @param d64 the D64 disc image to write
 * @returns 0 on success, -1 on error
 */
int writeD64(FILE *file, const D64 *d64);

#endif
