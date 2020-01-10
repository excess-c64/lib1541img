#ifndef I1541_ZC45EXTRACTOR_H
#define I1541_ZC45EXTRACTOR_H

/** Contains a function for extracting a D64 disc image from zipcode
 * @file
 */

#include <1541img/decl.h>

C_CLASS_DECL(D64);
C_CLASS_DECL(ZcFileSet);

/** Extract a D64 disc image from zipcode.
 * @relatesalso ZcFileSet
 *
 *     #include <1541img/zc45extractor.h>
 * 
 * This extracts a D64 disc image from a 4-pack (35 tracks) or 5-pack
 * (40 tracks) zipcode file set.
 * @param fileset the zipcode fileset to extract
 * @returns the extracted D64 disc image, or NULL on error
 */
DECLEXPORT D64 *extractZc45(const ZcFileSet *fileset);

#endif
