#ifndef I1541_CBMDOSINODE_H
#define I1541_CBMDOSINODE_H

/** Declarations for the CbmdosInode class
 * @file
 */

#include <stdint.h>
#include <stdio.h>

#include <1541img/decl.h>

#include <1541img/cbmdosinodeeventargs.h>
#include <1541img/cbmdosfsoptions.h>

C_CLASS_DECL(FileData);
C_CLASS_DECL(Event);

/** A cbmdos inode.
 * This class models a file's contents on disk in cbmdos format
 * @class CbmdosInode cbmdosinode.h <1541img/cbmdosinode.h>
 */
C_CLASS_DECL(CbmdosInode);

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

/** Number of blocks the file currently needs on disk
 * @memberof CbmdosInode
 * @param self the cbmdos file
 * @returns number of blocks
 */
DECLEXPORT uint16_t CbmdosInode_blocks(const CbmdosInode *self);

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

#endif
