#ifndef I1541_FILEDATA_H
#define I1541_FILEDATA_H

/** Declarations for the FileData class
 * @file
 */

#include <stddef.h>
#include <stdint.h>

#include <1541img/decl.h>

C_CLASS_DECL(Event);

/** Maximum size of file content (1 MiB)
 * @relatesalso FileData
 */
#define FILEDATA_MAXSIZE 1024*1024UL

/** Some file content.
 * This class models file content (bytes), for example for use as the content
 * of a CbmdosFile.
 * @class FileData filedata.h <1541img/filedata.h>
 */
C_CLASS_DECL(FileData);

/** default constructor.
 * Creates empty file content
 * @memberof FileData
 * @returns the newly created FileData
 */
DECLEXPORT FileData *FileData_create(void);

/** copy constructor.
 * Creates a new file content that's an exact copy of a given one
 * @memberof FileData
 * @param self the file content to copy
 * @returns newly created FileData with copied content
 */
DECLEXPORT FileData *FileData_clone(const FileData *self);

/** The size of the content
 * @memberof FileData
 * @param self the file content
 * @returns the size of the content
 */
DECLEXPORT size_t FileData_size(const FileData *self);

/** Gets the read-only bytes of the content
 * @memberof FileData
 * @param self the file content
 * @returns a read-only pointer to the actual bytes
 */
DECLEXPORT const uint8_t *FileData_rcontent(const FileData *self);

/** Append a chunk of bytes to the content
 * @memberof FileData
 * @param self the file content
 * @param data the bytes to append
 * @param size the number of bytes to append
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int FileData_append(
	FileData *self, const uint8_t *data, size_t size);

/** Append a single byte to the content
 * @memberof FileData
 * @param self the file content
 * @param byte the byte to append
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int FileData_appendByte(FileData *self, uint8_t byte);

/** Append a single byte repeatedly to the content
 * @memberof FileData
 * @param self the file content
 * @param byte the byte to append
 * @param count how often to append this byte
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int FileData_appendBytes(
	FileData *self, uint8_t byte, size_t count);

/** Set a single byte at a given position of the content.
 * The position must already exist.
 * @memberof FileData
 * @param self the file content
 * @param byte the byte to set
 * @param pos the position at which to set the byte
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int FileData_setByte(FileData *self, uint8_t byte, size_t pos);

/** Event that gets raised when the actual content changes
 * @memberof FileData
 * @param self the file content
 * @returns an Event to register to / unregister from
 */
DECLEXPORT Event *FileData_changedEvent(FileData *self);

/** FileData destructor
 * @memberof FileData
 * @param self the file content
 */
DECLEXPORT void FileData_destroy(FileData *self);

#endif
