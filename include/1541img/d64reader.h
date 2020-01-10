#ifndef I1541_D64READER_H
#define I1541_D64READER_H
#ifdef __cplusplus
extern "C" {
#endif

/** Contains functions to read a D64 disc image
 * @file
 */

#include <stdio.h>

typedef struct D64 D64;
typedef struct FileData FileData;

/** Read a D64 disc image from a FileData instance
 * @relatesalso D64
 *
 *     #include <1541img/d64reader.h>
 *
 * @param file a FileData instance to read the disc image from
 * @returns a D64 disc image, or NULL on error
 */
D64 *readD64FromFileData(const FileData *file);

/** Read a D64 disc image from a (host) file.
 * @relatesalso D64
 *
 *     #include <1541img/d64reader.h>
 *
 * @param file a file opened for reading to read the disc image from
 * @returns a D64 disc image, or NULL on error
 */
D64 *readD64(FILE *file);

#ifdef __cplusplus
}
#endif
#endif
