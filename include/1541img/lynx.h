#ifndef I1541_LYNX_H
#define I1541_LYNX_H
#ifdef __cplusplus
extern "C" {
#endif

/** Contains functions for handling LyNX archives
 * @file
 */

typedef struct CbmdosFile CbmdosFile;
typedef struct CbmdosVfs CbmdosVfs;
typedef struct FileData FileData;

/** Check whether a file is in LyNX format
 * @relatesalso FileData
 *
 *     #include <1541img/lynx.h>
 *
 * @param file a FileData instance to check whether it contains a LyNX archive
 * @returns 1 if the file is in LyNX format, 0 otherwise
 */
int isLynx(const FileData *file);

/** Extract files from a LyNX archive
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/lynx.h>
 * 
 * @param vfs a CbmdosVfs instance to write extracted files to
 * @param file a FileData instance containing a LyNX archive
 * @returns 0 on success, -1 on error
 */
int extractLynx(CbmdosVfs *vfs, const FileData *file);

/** Create a LyNX archive from a set of Cbmdos files
 * @relatesalso CbmdosFile
 *
 *     #include <1541img/lynx.h>
 *
 * @param files pointer to an array of Cbmdos files
 * @param filecount number of files in the array
 * @returns a new FileData instance of the LyNX archive, or NULL on error
 */
FileData *archiveLynxFiles(const CbmdosFile **files, unsigned filecount);

/** Create a LyNX archive from all files in a Cbmdos vfs
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/lynx.h>
 *
 * @param vfs a CbmdosVfs instance containing the files to archive with LyNX
 * @returns a new FileData instance of the LyNX archive, or NULL on error
 */
FileData *archiveLynx(const CbmdosVfs *vfs);

#ifdef __cplusplus
}
#endif
#endif
