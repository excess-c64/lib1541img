#ifndef I1541_CBMDOSFILE_H
#define I1541_CBMDOSFILE_H

/** Declarations for the CbmdosInode class
 * @file
 */

#include <stdint.h>
#include <stdio.h>

#include <1541img/decl.h>

#include <1541img/cbmdosfileeventargs.h>
#include <1541img/cbmdosfsoptions.h>

C_CLASS_DECL(FileData);
C_CLASS_DECL(Event);

/** A cbmdos inode.
 * This class models a file's contents on disk in cbmdos format
 * @class CbmdosInode cbmdosinode.h <1541img/cbmdosinode.h>
 */
C_CLASS_DECL(CbmdosInode);

/** default constructor.
 * Creates a new CbmdosInode
 * @memberof CbmdosInode
 * @returns the newly created CbmdosInode
 */
DECLEXPORT CbmdosInode *CbmdosInode_create(void);

/** copy constructor.
 * Creates a deep copy of an existing CbmdosInode
 * @memberof CbmdosInode
 * @param other the file to copy
 * @returns the copied CbmdosInode
 */
DECLEXPORT CbmdosInode *CbmdosInode_clone(const CbmdosInode *other);

/** The read-only contents of the file
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns a read-only pointer to the file contents
 */
DECLEXPORT const FileData *CbmdosInode_rdata(const CbmdosInode *self);

/** The contents of the file
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns a pointer to the file contents
 */
DECLEXPORT FileData *CbmdosInode_data(CbmdosInode *self);

/** Set new file contents.
 * The current file contents are replaced. The new contents supplied will be
 * owned by this CbmdosInode, so don't free them yourself.
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @param data the new file contents
 */
DECLEXPORT void CbmdosInode_setData(CbmdosInode *self, FileData *data);

/** Export to a raw host file.
 * Exports the current file contents to a raw file on the host. Note that for
 * REL files, this will lose the record length information.
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @param file a file opened for writing to write the contents to
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosInode_exportRaw(const CbmdosInode *self, FILE *file);

/** Export to a PC64 (Pxx/Sxx/Uxx/Rxx) host file.
 * Exports the current file contents to a file on the host in PC64 format.
 * This includes the original filename and, for REL files, the record length.
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @param file a file opened for writing to write the contents to
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosInode_exportPC64(const CbmdosInode *self, FILE *file);

/** Import a host file as new file content
 * Imports a host file as the new file content. PC64 files are automatically
 * detected and name and record length are set accordingly. All other files
 * are treated as raw file data.
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @param file a file opened for reading to read the contents from
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int CbmdosInode_import(CbmdosInode *self, FILE *file);

/** Number of blocks the file currently needs on disk
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns number of blocks
 */
DECLEXPORT uint16_t CbmdosInode_realBlocks(const CbmdosInode *self);

/** Overrides for some CbmdosFs options
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns the option overrides
 */
DECLEXPORT CbmdosFsOptOverrides CbmdosInode_optOverrides(
        const CbmdosInode *self);

/** Set overrides for some CbmdosFs options
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @param overrides the new option overrides
 */
DECLEXPORT void CbmdosInode_setOptOverrides(
        CbmdosInode *self, CbmdosFsOptOverrides overrides);

/** Event that gets raised on any changes to the file
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns an Event to register to / unregister from
 */
DECLEXPORT Event *CbmdosInode_changedEvent(CbmdosInode *self);

/** CbmdosInode destructor
 * @memberof CbmdosInode
 * @param self the cbmdos file
 */
DECLEXPORT void CbmdosInode_destroy(CbmdosInode *self);

#endif
