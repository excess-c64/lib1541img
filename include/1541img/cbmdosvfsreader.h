#ifndef I1541_CBMDOSVFSREADER_H
#define I1541_CBMDOSVFSREADER_H
#ifdef __cplusplus
extern "C" {
#endif

/** Contains a function to read a cbmdos vfs from a D64 disc image
 * @file
 */

typedef struct D64 D64;
typedef struct CbmdosVfs CbmdosVfs;

/** Read a cbmdos vfs from a D64 disc image.
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/cbmdosvfsreader.h>
 *
 * @param vfs an empty cbmdos vfs. Reading will fail if you pass a non-empty
 *    vfs.
 * @param d64 the D64 disc image to read from
 * @returns 0 on success, -1 on error
 */
int readCbmdosVfs(CbmdosVfs *vfs, const D64 *d64);

#ifdef __cplusplus
}
#endif
#endif
