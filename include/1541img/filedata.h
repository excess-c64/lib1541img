#ifndef I1541_FILEDATA_H
#define I1541_FILEDATA_H
#ifdef __cplusplus
extern "C" {
#endif

/** Declarations for the FileData class
 * @file
 */

#include <stddef.h>
#include <stdint.h>

typedef struct Event Event;

/** Maximum size of file content (1 MiB)
 * @relatesalso FileData
 */
#define FILEDATA_MAXSIZE 1024*1024UL

/** Some file content.
 * This class models file content (bytes), for example for use as the content
 * of a CbmdosFile.
 * @class FileData filedata.h <1541img/filedata.h>
 */
typedef struct FileData FileData;

/** default constructor.
 * Creates empty file content
 * @memberof FileData
 * @returns the newly created FileData
 */
FileData *FileData_create(void);

/** copy constructor.
 * Creates a new file content that's an exact copy of a given one
 * @memberof FileData
 * @param self the file content to copy
 * @returns newly created FileData with copied content
 */
FileData *FileData_clone(const FileData *self);

/** The size of the content
 * @memberof FileData
 * @param self the file content
 * @returns the size of the content
 */
size_t FileData_size(const FileData *self);

/** Gets the read-only bytes of the content
 * @memberof FileData
 * @param self the file content
 * @returns a read-only pointer to the actual bytes
 */
const uint8_t *FileData_rcontent(const FileData *self);

/** Append a chunk of bytes to the content
 * @memberof FileData
 * @param self the file content
 * @param data the bytes to append
 * @param size the number of bytes to append
 * @returns 0 on success, -1 on error
 */
int FileData_append(FileData *self, const uint8_t *data, size_t size);

/** Append a single byte to the content
 * @memberof FileData
 * @param self the file content
 * @param byte the byte to append
 * @returns 0 on success, -1 on error
 */
int FileData_appendByte(FileData *self, uint8_t byte);

/** Event that gets raised when the actual content changes
 * @memberof FileData
 * @param self the file content
 * @returns an Event to register to / unregister from
 */
Event *FileData_changedEvent(FileData *self);

/** FileData destructor
 * @memberof FileData
 * @param self the file content
 */
void FileData_destroy(FileData *self);

#ifdef __cplusplus
}
#endif
#endif
