#ifndef I1541_CBMDOSVFSEVENTARGS_H
#define I1541_CBMDOSVFSEVENTARGS_H

/** Declaration of CbmdosVfs Event arguments
 * @file
 */

#include <1541img/cbmdosfileeventargs.h>

/** Arguments for events raised by CbmdosVfs */
typedef struct CbmdosVfsEventArgs
{
    enum {
        CVE_DOSVERCHANGED,  /**< the dos version number changed */
        CVE_NAMECHANGED,    /**< the name of the filesystem changed */
        CVE_IDCHANGED,      /**< the disc ID of the filesystem changed */
        CVE_FILEADDED,      /**< a file was added to the filesystem */
        CVE_FILEDELETED,    /**< a file was deleted from the filesystem */
        CVE_FILECHANGED     /**< a file on the filesystem was changed */
    } what;     /**< describes what happened to the vfs */
    const CbmdosFileEventArgs *fileEventArgs;   /**< for changed files, 
                                a pointer to the corresponding event args */
    unsigned filepos;       /**< for changes concerning a file, the position
                                 affected by the change */
} CbmdosVfsEventArgs;

#endif
