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
typedef struct CbmdosFsOptions CbmdosFsOptions;

/** Read a cbmdos vfs from a D64 disc image.
 * @relatesalso CbmdosVfs
 *
 *     #include <1541img/cbmdosvfsreader.h>
 *
 * @param vfs an empty cbmdos vfs. Reading will fail if you pass a non-empty
 *    vfs.
 * @param d64 the D64 disc image to read from
 * @param options cbmdos filesystem options to use when reading. Probed
 *    options are assumed when passing NULL here.
 * @returns 0 on success, -1 on error
 */
int readCbmdosVfs(CbmdosVfs *vfs, const D64 *d64,
	const CbmdosFsOptions *options);

/** Determine cbmdos fs options needed to read from a D64 disc image.
 * @relatesalso CbmdosFsOptions
 *
 *     #include <1541img/cbmdosvfsreader.h>
 * 
 * This functions tries to find a cbmdos filesystem on the given disc image
 * and determine the correct CbmdosFsOptions for reading it. The result is
 * just a "best guess", but it should normally work for reading from the
 * image.
 * @param options a pointer to the options to fill
 * @param d64 the D64 disc image to read from
 * @returns 0 on success, -1 on error
 */
int probeCbmdosFsOptions(CbmdosFsOptions *options, const D64 *d64);

#ifdef __cplusplus
}
#endif
#endif
