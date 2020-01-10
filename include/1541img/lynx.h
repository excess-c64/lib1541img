#ifndef I1541_LYNX_H
#define I1541_LYNX_H

/** Contains functions for handling LyNX archives
 * @file
 */

#include <1541img/decl.h>

C_CLASS_DECL(CbmdosFile);
C_CLASS_DECL(CbmdosVfs);
C_CLASS_DECL(FileData);

/** Check whether a file is in LyNX format
 * @relatesalso FileData
 *
 *     #include <1541img/lynx.h>
 *
 * @param file a FileData instance to check whether it contains a LyNX archive
 * @returns 1 if the file is in LyNX format, 0 otherwise
 */
DECLEXPORT int isLynx(const FileData *file);

/** Extract files from a LyNX archive
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/lynx.h>
 * 
 * @param vfs a CbmdosVfs instance to write extracted files to
 * @param file a FileData instance containing a LyNX archive
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int extractLynx(CbmdosVfs *vfs, const FileData *file);

/** Create a LyNX archive from a set of Cbmdos files
 * @relatesalso CbmdosFile
 *
 *     #include <1541img/lynx.h>
 *
 * @param files pointer to an array of Cbmdos files
 * @param filecount number of files in the array
 * @returns a new FileData instance of the LyNX archive, or NULL on error
 */
DECLEXPORT FileData *archiveLynxFiles(
	const CbmdosFile **files, unsigned filecount);

/** Create a LyNX archive from all files in a Cbmdos vfs
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/lynx.h>
 *
 * @param vfs a CbmdosVfs instance containing the files to archive with LyNX
 * @returns a new FileData instance of the LyNX archive, or NULL on error
 */
DECLEXPORT FileData *archiveLynx(const CbmdosVfs *vfs);

#endif
