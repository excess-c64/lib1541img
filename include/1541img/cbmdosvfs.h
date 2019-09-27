#ifndef I1541_CBMDOSVFS_H
#define I1541_CBMDOSVFS_H

/** Declarations for the CbmdosVfs class
 * @file
 */

#include <stdint.h>

#include <1541img/cbmdosvfseventargs.h>

typedef struct Event Event;
typedef struct CbmdosFile CbmdosFile;

/** Class modeling a virtual cbmdos filesystem.
 * This models the virtual filesystem, containing a flat directory and
 * the actual files, in cbmdos format. It doesn't have a direct connection
 * to the physical disc structure, which is provided by the concrete
 * filesystem in CbmdosFs.
 * @class CbmdosVfs cbmdosvfs.h <1541img/cbmdosvfs.h>
 */
typedef struct CbmdosVfs CbmdosVfs;

/** default constructor.
 * Creates an empty cbmdos vfs
 * @memberof CbmdosVfs
 * @returns a newly created CbmdosVfs
 */
CbmdosVfs *CbmdosVfs_create(void);

/** The dos version number
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @returns the dos version number
 */
uint8_t CbmdosVfs_dosver(const CbmdosVfs *self);

/** Set the dos version number
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param dosver the new dos version number (default is 0x41)
 */
void CbmdosVfs_setDosver(CbmdosVfs *self, uint8_t dosver);

/** The raw name of the filesystem.
 * This is what appears as the title of the disc in the directory.
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param length if not NULL, the length of the raw name is written here
 * @returns a pointer to the raw name (this is NOT a NULL-terminated C string!)
 */
const char *CbmdosVfs_name(const CbmdosVfs *self, uint8_t *length);

/** Set the raw name of the filesystem.
 * This sets a new name of the filesystem (what appears as the title of the
 * disc in the directory). If the length is greater than 16, it will be
 * truncated.
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param name pointer to the new name (doesn't need to be NULL-terminated)
 * @param length the length of the new name
 */
void CbmdosVfs_setName(CbmdosVfs *self, const char *name, uint8_t length);

/** The disc ID of the filesystem.
 * This is what appears in the ID field (and possibly extending over the
 * DOS type field) in the directory.
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param length if not NULL, the length of the disc ID is written here
 * @returns a pointer to the disc ID (this is NOT a NULL-terminated C string!)
 */
const char *CbmdosVfs_id(const CbmdosVfs *self, uint8_t *length);

/** Set the disc ID of the filesystem.
 * This sets a new disc ID. It can have up to 5 bytes, to extend over the DOS
 * type field, which normally reads `2A`. If this isn't set, the ID remains
 * empty (technically filled with 0xa0 characters). On a standard disc, the ID
 * only has 2 characters; the 1541 floppy drive will only use the first 2
 * characters. If the length is grater than 5, it will be truncated.
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param id pointer to the new ID (doesn't need to be NULL-terminated)
 * @param length the length of the new ID
 */
void CbmdosVfs_setId(CbmdosVfs *self, const char *id, uint8_t length);

/** Number of files in this filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @returns the current number of files in this filesystem
 */
unsigned CbmdosVfs_fileCount(const CbmdosVfs *self);

/** Gets a read-only file from this filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param pos the position of the file to get (starting at 0)
 * @returns a read-only pointer to the file, or NULL on error
 */
const CbmdosFile *CbmdosVfs_rfile(const CbmdosVfs *self, unsigned pos);

/** Gets a file from this filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param pos the position of the file to get (starting at 0)
 * @returns a pointer to the file, or NULL on error
 */
CbmdosFile *CbmdosVfs_file(CbmdosVfs *self, unsigned pos);

/** Delete a given file from the filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param file the file to delete
 * @returns 0 on success, -1 on error
 */
int CbmdosVfs_delete(CbmdosVfs *self, const CbmdosFile *file);

/** Delete a file at a given position
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param pos position of the file to delete (starting at 0)
 * @returns 0 on success, -1 on error
 */
int CbmdosVfs_deleteAt(CbmdosVfs *self, unsigned pos);

/** Append a file to the filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param file the file to append
 * @returns 0 on success, -1 on error
 */
int CbmdosVfs_append(CbmdosVfs *self, CbmdosFile *file);

/** Insert a file at a given position
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @param file the file to insert
 * @param pos the position to insert the file at (starting at 0). If this is
 *            greater or equal to the number of files, the file is appended.
 * @returns 0 on success, -1 on error
 */
int CbmdosVfs_insert(CbmdosVfs *self, CbmdosFile *file, unsigned pos);

/** Event that gets raised on any changes to the filesystem
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 * @returns an Event to register to / unregister from
 */
Event *CbmdosVfs_changedEvent(CbmdosVfs *self);

/** CbmdosVfs destructor
 * @memberof CbmdosVfs
 * @param self the cbmdos vfs
 */
void CbmdosVfs_destroy(CbmdosVfs *self);

#endif
