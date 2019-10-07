#ifndef I1541_ZCFILESET_H
#define I1541_ZCFILESET_H
#ifdef __cplusplus
extern "C" {
#endif

/** Declarations for the ZcFileSet class
 * @file
 */

/** Type of the zipcode file set */
typedef enum ZcType
{
    ZT_4PACK,   /**< 4-pack (compressed 35-tracks image in 4 files) */
    ZT_5PACK,   /**< 5-pack (compressed 40-tracks image in 5 files) */
    ZT_6PACK    /**< 6-pack (compressed raw GCR image in 6 files,
                  CURRENTLY UNIMPLEMENTED) */
} ZcType;

typedef struct FileData FileData;
typedef struct CbmdosVfs CbmdosVfs;

/** A zipcode file set.
 * This class models zipcode compressed disk images, consisting of multiple
 * files. Standard 35-track images are compressed to 4 files, 40-track images
 * to 5 files and raw GCR images (currently not implemented) to 6 files.
 * @class ZcFileSet zcfileset.h <1541img/zcfileset.h>
 */
typedef struct ZcFileSet ZcFileSet;

/** default constructor.
 * Creates an empty zipcode file set
 * @memberof ZcFileSet
 * @param type the zipcode type
 * @param name the base name of the file set
 * @returns a newly created ZcFileSet
 */
ZcFileSet *ZcFileSet_create(ZcType type, const char *name);

/** Create a ZcFileSet from a host file.
 * This creates a ZcFileSet from either a D64 disc image which must contain
 * all the files, or from the files directly from the host file system, in
 * which case you should pass one of the names, the others are found
 * automatically according to the naming convention of zipcode (all file
 * names start with the index number (1 to 6) followed by an exclamation mark)
 * @memberof ZcFileSet
 * @param filename the name of a zipcode member file or a D64 disc image
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
ZcFileSet *ZcFileSet_fromFile(const char *filename);

/** The zipcode type of this set
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the zipcode type
 */
ZcType ZcFileSet_type(const ZcFileSet *self);

/** The number of files in this set
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the number of files
 */
int ZcFileSet_count(const ZcFileSet *self);

/** The base name of this set.
 * The base name is the name of the zipcode files without the prefix (e.g.
 * "1!") and without the ".prg" suffix.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the base name
 */
const char *ZcFileSet_name(const ZcFileSet *self);

/** Gets the read-only content of a member file
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param index the index number of the member file
 * @returns a read-only pointer to the file content, or NULL on error
 */
const FileData *ZcFileSet_rfileData(const ZcFileSet *self, int index);

/** Gets the content of a member file
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param index the index number of the member file
 * @returns a pointer to the file content, or NULL on error
 */
FileData *ZcFileSet_fileData(ZcFileSet *self, int index);

/** Save this fileset to the host filesystem.
 * For saving, you can either just provide a directory, in which case the
 * base name of this set is used. You can also provide a full name, that may
 * optionally include a zipcode prefix (e.g. "1!") and/or a ".prg" suffix.
 * Both are added automatically if they are missing.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param filename where to save the file set
 * @returns 0 on success, -1 on error
 */
int ZcFileSet_save(const ZcFileSet *self, const char *filename);

/** Save this fileset to a CbmdosVfs.
 * This will use the base name of this set for saving `PRG` files to the given
 * cbmdos vfs.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param vfs the cbmdos vfs to save to
 * @returns 0 on success, -1 on error
 */
int ZcFileSet_saveVfs(const ZcFileSet *self, CbmdosVfs *vfs);

/** ZcFileSet destructor
 * @memberof ZcFileSet
 * @param self the zipcode file set
 */
void ZcFileSet_destroy(ZcFileSet *self);

#ifdef __cplusplus
}
#endif
#endif
