#ifndef I1541_ZCFILESET_H
#define I1541_ZCFILESET_H

/** Declarations for the ZcFileSet class
 * @file
 */

#include <1541img/decl.h>

C_CLASS_DECL(CbmdosVfs);
C_CLASS_DECL(FileData);

/** Type of the zipcode file set */
typedef enum ZcType
{
    ZT_4PACK,   /**< 4-pack (compressed 35-tracks image in 4 files) */
    ZT_5PACK,   /**< 5-pack (compressed 40-tracks image in 5 files) */
    ZT_6PACK    /**< 6-pack (compressed raw GCR image in 6 files,
                  CURRENTLY UNIMPLEMENTED) */
} ZcType;

/** A zipcode file set.
 * This class models zipcode compressed disk images, consisting of multiple
 * files. Standard 35-track images are compressed to 4 files, 40-track images
 * to 5 files and raw GCR images (currently not implemented) to 6 files.
 * @class ZcFileSet zcfileset.h <1541img/zcfileset.h>
 */
C_CLASS_DECL(ZcFileSet);

/** default constructor.
 * Creates an empty zipcode file set
 * @memberof ZcFileSet
 * @param type the zipcode type
 * @param name the base name of the file set
 * @returns a newly created ZcFileSet
 */
DECLEXPORT ZcFileSet *ZcFileSet_create(ZcType type, const char *name);

/** Create a ZcFileSet from a Cbmdos vfs.
 * This creates a ZcFileSet from a Cbmdos vfs, which must contain all the
 * zipcode files.
 * @memberof ZcFileSet
 * @param vfs the Cbmdos vfs to search for zipcode files.
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
DECLEXPORT ZcFileSet *ZcFileSet_fromVfs(const CbmdosVfs *vfs);

/** Try to create a ZcFileSet from a Cbmdos vfs.
 * This does exactly the same as ZcFileSet_fromVfs(), but won't log any error
 * messages. Use this if you don't know whether your VFS should contain
 * zipcode files or not.
 * @memberof ZcFileSet
 * @param vfs the Cbmdos vfs to search for zipcode files.
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
DECLEXPORT ZcFileSet *ZcFileSet_tryFromVfs(const CbmdosVfs *vfs);

/** Create a ZcFileSet from a FileData instance.
 * This creates a ZcFileSet from a FileData instance containing a D64 image,
 * which must contain all the zipcode files.
 * @memberof ZcFileSet
 * @param file the FileData instance containing a D64 image
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
DECLEXPORT ZcFileSet *ZcFileSet_fromFileData(const FileData *file);

/** Try to create a ZcFileSet from a FileData instance.
 * This does exactly the same as ZcFileSet_fromFileData(), but won't log any
 * error messages. Use this if you don't know whether your File should contain
 * a D64 image with zipcode files or not.
 * @memberof ZcFileSet
 * @param file the FileData instance containing a D64 image
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
DECLEXPORT ZcFileSet *ZcFileSet_tryFromFileData(const FileData *file);

/** Create a ZcFileSet from a host file.
 * This creates a ZcFileSet from either a D64 disc image which must contain
 * all the files, or from the files directly from the host file system, in
 * which case you should pass one of the names, the others are found
 * automatically according to the naming convention of zipcode (all file
 * names start with the index number (1 to 6) followed by an exclamation mark)
 * @memberof ZcFileSet
 * @param filename the name of a zipcode member file or a D64 disc image. On
 *     windows, this *must* be in UTF-8 encoding.
 * @returns a ZcFileSet containing the zipcode files, or NULL on error
 */
DECLEXPORT ZcFileSet *ZcFileSet_fromFile(const char *filename);

/** The zipcode type of this set
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the zipcode type
 */
DECLEXPORT ZcType ZcFileSet_type(const ZcFileSet *self);

/** The number of files in this set
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the number of files
 */
DECLEXPORT int ZcFileSet_count(const ZcFileSet *self);

/** The base name of this set.
 * The base name is the name of the zipcode files without the prefix (e.g.
 * "1!") and without the ".prg" suffix.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @returns the base name
 */
DECLEXPORT const char *ZcFileSet_name(const ZcFileSet *self);

/** Gets the read-only content of a member file
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param index the index number of the member file
 * @returns a read-only pointer to the file content, or NULL on error
 */
DECLEXPORT const FileData *ZcFileSet_rfileData(
	const ZcFileSet *self, int index);

/** Gets the content of a member file
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param index the index number of the member file
 * @returns a pointer to the file content, or NULL on error
 */
DECLEXPORT FileData *ZcFileSet_fileData(ZcFileSet *self, int index);

/** Save this fileset to the host filesystem.
 * For saving, you can either just provide a directory, in which case the
 * base name of this set is used. You can also provide a full name, that may
 * optionally include a zipcode prefix (e.g. "1!") and/or a ".prg" suffix.
 * Both are added automatically if they are missing.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param filename where to save the file set. On windows, this *must* be in
 *     UTF-8 encoding.
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int ZcFileSet_save(const ZcFileSet *self, const char *filename);

/** Save this fileset to a CbmdosVfs.
 * This will use the base name of this set for saving `PRG` files to the given
 * cbmdos vfs.
 * @memberof ZcFileSet
 * @param self the zipcode file set
 * @param vfs the cbmdos vfs to save to
 * @returns 0 on success, -1 on error
 */
DECLEXPORT int ZcFileSet_saveVfs(const ZcFileSet *self, CbmdosVfs *vfs);

/** ZcFileSet destructor
 * @memberof ZcFileSet
 * @param self the zipcode file set
 */
DECLEXPORT void ZcFileSet_destroy(ZcFileSet *self);

#endif
