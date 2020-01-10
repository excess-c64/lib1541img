#ifndef I1541_D64READER_H
#define I1541_D64READER_H

/** Contains functions to read a D64 disc image
 * @file
 */

#include <stdio.h>

#include <1541img/decl.h>

C_CLASS_DECL(D64);
C_CLASS_DECL(FileData);

/** Read a D64 disc image from a FileData instance
 * @relatesalso D64
 *
 *     #include <1541img/d64reader.h>
 *
 * @param file a FileData instance to read the disc image from
 * @returns a D64 disc image, or NULL on error
 */
DECLEXPORT D64 *readD64FromFileData(const FileData *file);

/** Read a D64 disc image from a (host) file.
 * @relatesalso D64
 *
 *     #include <1541img/d64reader.h>
 *
 * @param file a file opened for reading to read the disc image from
 * @returns a D64 disc image, or NULL on error
 */
DECLEXPORT D64 *readD64(FILE *file);

#endif
