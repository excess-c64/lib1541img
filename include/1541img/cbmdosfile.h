#ifndef I1541_CBMDOSFILE_H
#define I1541_CBMDOSFILE_H

/** Declarations for the CbmdosFile class
 * @file
 */

#include <stdint.h>
#include <stdio.h>

#include <1541img/decl.h>

#include <1541img/cbmdosfileeventargs.h>
#include <1541img/cbmdosfsoptions.h>

C_CLASS_DECL(FileData);
C_CLASS_DECL(Event);

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
DECLEXPORT const char *CbmdosFileType_name(CbmdosFileType type);

/** A cbmdos file.
 * This class models a file on disk in cbmdos format
 * @class CbmdosFile cbmdosfile.h <1541img/cbmdosfile.h>
 */
C_CLASS_DECL(CbmdosFile);

/** default constructor.
 * Creates a new CbmdosFile
 * @memberof CbmdosFile
 * @returns the newly created CbmdosFile
 */
DECLEXPORT CbmdosFile *CbmdosFile_create(void);

/** copy constructor.
 * Creates a deep copy of an existing CbmdosFile
 * @memberof CbmdosFile
 * @param other the file to copy
 * @returns the copied CbmdosFile
 */
DECLEXPORT CbmdosFile *CbmdosFile_clone(const CbmdosFile *other);

/** Type of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the type of this file
 */
DECLEXPORT CbmdosFileType CbmdosFile_type(const CbmdosFile *self);

/** Invalid type of the file.
 * If the file has an invalid type, the raw number of the type is returned.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the invalid type of this file, or -1 if the type is valid.
 */
DECLEXPORT int CbmdosFile_invalidType(const CbmdosFile *self);

/** Set the type of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param type the new type of the file
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFile_setType(CbmdosFile *self, CbmdosFileType type);

/** The raw name of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param length if not NULL, the length of the raw name is written here
 * @returns a pointer to the raw name (this is NOT a NULL-terminated C string!)
 */
DECLEXPORT const char *CbmdosFile_name(
	const CbmdosFile *self, uint8_t *length);

/** Set the raw name of the file.
 * This sets a new name for the file. If the length is greater than 16, it
 * will be truncated.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param name pointer to the new name (doesn't need to be NULL-terminated)
 * @param length the length of the new name
 */
DECLEXPORT void CbmdosFile_setName(
	CbmdosFile *self, const char *name, uint8_t length);

/** Map uppercase graphics chars in filename to lowercase chars.
 * This changes the filename, so Uppercase/Gfx chars are replaced by chars
 * that also work in lowercase mode, see PETSCII module.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 */
DECLEXPORT void CbmdosFile_mapUpperGfxToLower(CbmdosFile *self);

/** The read-only contents of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns a read-only pointer to the file contents
 */
DECLEXPORT const FileData *CbmdosFile_rdata(const CbmdosFile *self);

/** The contents of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns a pointer to the file contents
 */
DECLEXPORT FileData *CbmdosFile_data(CbmdosFile *self);

/** Set new file contents.
 * The current file contents are replaced. The new contents supplied will be
 * owned by this CbmdosFile, so don't free them yourself.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param data the new file contents
 */
DECLEXPORT void CbmdosFile_setData(CbmdosFile *self, FileData *data);

/** Export to a raw host file.
 * Exports the current file contents to a raw file on the host. Note that for
 * REL files, this will lose the record length information.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param file a file opened for writing to write the contents to
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFile_exportRaw(const CbmdosFile *self, FILE *file);

/** Export to a PC64 (Pxx/Sxx/Uxx/Rxx) host file.
 * Exports the current file contents to a file on the host in PC64 format.
 * This includes the original filename and, for REL files, the record length.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param file a file opened for writing to write the contents to
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFile_exportPC64(const CbmdosFile *self, FILE *file);

/** Import a host file as new file content
 * Imports a host file as the new file content. PC64 files are automatically
 * detected and name and record length are set accordingly. All other files
 * are treated as raw file data.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param file a file opened for reading to read the contents from
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFile_import(CbmdosFile *self, FILE *file);

/** The record length of a REL file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the length of a record, or 0 if not a REL file
 */
DECLEXPORT uint8_t CbmdosFile_recordLength(const CbmdosFile *self);

/** Set the record length of a REL file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param recordLength the new record length, maximum 254
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosFile_setRecordLength(
	CbmdosFile *self, uint8_t recordLength);

/** Number of blocks the file currently needs on disk
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns number of blocks
 */
DECLEXPORT uint16_t CbmdosFile_realBlocks(const CbmdosFile *self);

/** Number of blocks actually shown in directory.
 * This is either the actual number of blocks, or, if set, the "forced" block
 * size.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns number of blocks
 */
DECLEXPORT uint16_t CbmdosFile_blocks(const CbmdosFile *self);

/** Number of blocks forced to be shown in directory.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns forced block size, 0xffff means no forced size
 */
DECLEXPORT uint16_t CbmdosFile_forcedBlocks(const CbmdosFile *self);

/** Set number of forced blocks to be shown in directory.
 * If this is set, a directory containing this file will show the given
 * size (in blocks) instead of the actual size.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param forcedBlocks forced block size, 0xffff means no forced size
 */
DECLEXPORT void CbmdosFile_setForcedBlocks(
	CbmdosFile *self, uint16_t forcedBlocks);

/** Overrides for some CbmdosFs options
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the option overrides
 */
DECLEXPORT CbmdosFsOptOverrides CbmdosFile_optOverrides(
        const CbmdosFile *self);

/** Set overrides for some CbmdosFs options
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param overrides the new option overrides
 */
DECLEXPORT void CbmdosFile_setOptOverrides(
        CbmdosFile *self, CbmdosFsOptOverrides overrides);

/** Locked flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns 1 if file is locked, 0 otherwise
 */
DECLEXPORT int CbmdosFile_locked(const CbmdosFile *self);

/** Set locked flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param locked 1 sets file locked, 0 unlocked
 */
DECLEXPORT void CbmdosFile_setLocked(CbmdosFile *self, int locked);

/** Closed flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns 1 if file is closed, 0 otherwise
 */
DECLEXPORT int CbmdosFile_closed(const CbmdosFile *self);

/** Set closed flag of the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param closed 1 sets file closed, 0 unclosed (default is 1)
 */
DECLEXPORT void CbmdosFile_setClosed(CbmdosFile *self, int closed);

/** AutoMapToLc flag of the file.
 * The flag controls whether a filename set is automatically mapped with
 * petscii_mapUpperGfxToLower(), see PETSCII module.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns the AutoMapToLc flag
 */
DECLEXPORT int CbmdosFile_autoMapToLc(const CbmdosFile *self);

/** Set AutoMapToLc flag of the file.
 * The flag controls whether a filename set is automatically mapped with
 * petscii_mapUpperGfxToLower(), see PETSCII module.
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param autoMapToLc the AutoMapToLc flag (default is 0)
 */
DECLEXPORT void CbmdosFile_setAutoMapToLc(CbmdosFile *self, int autoMapToLc);

/** Get a directory entry line.
 * Gets a line as displayed in a directory on the C64, in PETSCII encoding
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @param line a pointer to exactly 28 bytes, the line in PETSCII encoding
 *     will be written here, without any 0-termination.
 */
DECLEXPORT void CbmdosFile_getDirLine(const CbmdosFile *self, uint8_t *line);

/** Event that gets raised on any changes to the file
 * @memberof CbmdosFile
 * @param self the cbmdos file
 * @returns an Event to register to / unregister from
 */
DECLEXPORT Event *CbmdosFile_changedEvent(CbmdosFile *self);

/** CbmdosFile destructor
 * @memberof CbmdosFile
 * @param self the cbmdos file
 */
DECLEXPORT void CbmdosFile_destroy(CbmdosFile *self);

#endif
