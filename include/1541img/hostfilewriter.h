#ifndef I1541_HOSTFILEWRITER_H
#define I1541_HOSTFILEWRITER_H
#ifdef __cplusplus
extern "C" {
#endif

/** Contains a function to write FileData to a (host) file
 * @file
 */

#include <stdio.h>

typedef struct FileData FileData;

/** Write FileData to a (host) file.
 * @relatesalso FileData
 *
 *     #include <1541img/hostfilewriter.h>
 *
 * @param data the FileData instance to write
 * @param file a file opened for writing to write the FileData to
 * @returns 0 on success, -1 on error
 */
int writeHostFile(const FileData *data, FILE *file);

#ifdef __cplusplus
}
#endif
#endif
