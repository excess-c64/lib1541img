#ifndef I1541_D64READER_H
#define I1541_D64READER_H

/** Contains a function to read a D64 disc image from a (host) file
 * @file
 */

#include <stdio.h>

typedef struct D64 D64;

/** Read a D64 disc image from a (host) file.
 * @relatesalso D64
 *
 *     #include <1541img/d64reader.h>
 *
 * @param file a file opened for reading to read the disc image from
 * @returns a D64 disc image, or NULL on error
 */
D64 *readD64(FILE *file);

#endif
