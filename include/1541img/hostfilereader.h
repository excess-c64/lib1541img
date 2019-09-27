#ifndef I1541_HOSTFILEREADER_H
#define I1541_HOSTFILEREADER_H

/** Contains a function to read FileData from a (host) file
 * @file
 */

#include <stdio.h>

typedef struct FileData FileData;

/** Read FileData from a (host) file.
 * @relatesalso FileData
 *
 *     #include <1541img/hostfilereader.h>
 *
 * @param file a file opened for reading to read the FileData from
 * @returns a new FileData instance, or NULL on error
 */
FileData *readHostFile(FILE *file);

#endif
