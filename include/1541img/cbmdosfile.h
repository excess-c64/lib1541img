#ifndef I1541_CBMDOSFILE_H
#define I1541_CBMDOSFILE_H
#ifdef __cplusplus
extern "C" {
#endif

/** Declarations for the CbmDosFile class
 * @file
 */

#include <stdint.h>

#include <1541img/cbmdosfileeventargs.h>

typedef struct FileData FileData;
typedef struct Event Event;

/** Type of a cbmdos file */
typedef enum CbmdosFileType
{
    CFT_DEL, /**< a DEL file (not a real file) */
    CFT_SEQ, /**< a SEQ file (sequential access) */
    CFT_PRG, /**< a PRG file (binary with load address, typically a program) */
    CFT_USR, /**< a USR file (plain binary, custom structure) */
    CFT_REL  /**< a REL file (file containing fixed-length records) */
} CbmdosFileType;

/** The name of a cbmdos file type
 * @param type the file type
 * @returns pointer to a string with the name, e.g. CFT_PRG -> "PRG"
 */
const char *CbmdosFileType_name(CbmdosFileType type);

/** A cbmdos file.
 * This class models a file on disk in cbmdos format
 * @class CbmdosFile cbmdosfile.h <1541img/cbmdosfile.h>
 */
typedef struct CbmdosFile CbmdosFile;

/** default constructor.
 * Creates a new CbmdosFile
 * @memberof CbmdosFile
 * @returns the newly created CbmdosFile
 */
CbmdosFile *CbmdosFile_create(void);

/** Type of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the type of this file
 */
CbmdosFileType CbmdosFile_type(const CbmdosFile *self);

/** Set the type of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param type the new type of the file
 * @returns 0 on success, -1 on error
 */
int CbmdosFile_setType(CbmdosFile *self, CbmdosFileType type);

/** The raw name of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param length if not NULL, the length of the raw name is written here
 * @returns a pointer to the raw name (this is NOT a NULL-terminated C string!)
 */
const char *CbmdosFile_name(const CbmdosFile *self, uint8_t *length);

/** Set the raw name of the file.
 * This sets a new name for the file. If the length is greater than 16, it
 * will be truncated.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param name pointer to the new name (doesn't need to be NULL-terminated)
 * @param length the length of the new name
 */
void CbmdosFile_setName(CbmdosFile *self, const char *name, uint8_t length);

/** The read-only contents of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns a read-only pointer to the file contents
 */
const FileData *CbmdosFile_rdata(const CbmdosFile *self);

/** The contents of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns a pointer to the file contents
 */
FileData *CbmdosFile_data(CbmdosFile *self);

/** Set new file contents.
 * The current file contents are replaced. The new contents supplied will be
 * owned by this CbmdosFile, so don't free them yourself.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param data the new file contents
 */
void CbmdosFile_setData(CbmdosFile *self, FileData *data);

/** The record length of a REL file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the length of a record, or 0 if not a REL file
 */
uint8_t CbmdosFile_recordLength(const CbmdosFile *self);

/** Set the record length of a REL file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param recordLength the new record length, maximum 254
 * @returns 0 on success, -1 on error
 */
int CbmdosFile_setRecordLength(CbmdosFile *self, uint8_t recordLength);

/** Number of blocks the file currently needs on disk
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns number of blocks
 */
uint16_t CbmdosFile_realBlocks(const CbmdosFile *self);

/** Number of blocks actually shown in directory.
 * This is either the actual number of blocks, or, if set, the "forced" block
 * size.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns number of blocks
 */
uint16_t CbmdosFile_blocks(const CbmdosFile *self);

/** Number of blocks forced to be shown in directory.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns forced block size, 0xffff means no forced size
 */
uint16_t CbmdosFile_forcedBlocks(const CbmdosFile *self);

/** Set number of forced blocks to be shown in directory.
 * If this is set, a directory containing this file will show the given
 * size (in blocks) instead of the actual size.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param forcedBlocks forced block size, 0xffff means no forced size
 */
void CbmdosFile_setForcedBlocks(CbmdosFile *self, uint16_t forcedBlocks);

/** Locked flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns 1 if file is locked, 0 otherwise
 */
int CbmdosFile_locked(const CbmdosFile *self);

/** Set locked flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param locked 1 sets file locked, 0 unlocked
 */
void CbmdosFile_setLocked(CbmdosFile *self, int locked);

/** Closed flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns 1 if file is closed, 0 otherwise
 */
int CbmdosFile_closed(const CbmdosFile *self);

/** Set closed flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param closed 1 sets file closed, 0 unclosed (default is 1)
 */
void CbmdosFile_setClosed(CbmdosFile *self, int closed);

/** Get a directory entry line.
 * Gets a line as displayed in a directory on the C64, in PETSCII encoding
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param line a pointer to exactly 27 bytes, the line in PETSCII encoding
 *     will be written here, without any 0-termination.
 */
void CbmdosFile_getDirLine(const CbmdosFile *self, uint8_t *line);

/** Event that gets raised on any changes to the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns an Event to register to / unregister from
 */
Event *CbmdosFile_changedEvent(CbmdosFile *self);

/** CbmdosFile destructor
 * @memberof CbmdosFile
 * @param self the cbmdos file
 */
void CbmdosFile_destroy(CbmdosFile *self);

#ifdef __cplusplus
}
#endif
#endif
