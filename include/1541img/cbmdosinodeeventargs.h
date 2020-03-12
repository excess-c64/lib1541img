#ifndef I1541_CBMDOSINODEEVENTARGS_H
#define I1541_CBMDOSINODEEVENTARGS_H

/** Declaration of CbmdosInode Event arguments
 * @inode
 */

#include <1541img/decl.h>

/** Arguments for events raised by CbmdosInode
 * @struct CbmdosInodeEventArgs cbmdosinodeeventargs.h \
 *     <1541img/cbmdosinodeeventargs.h>
 */
C_CLASS_DECL(CbmdosInodeEventArgs);

struct CbmdosInodeEventArgs
{
    enum {
        CIE_DATACHANGED,            /**< file content of an inode changed */
	CIE_OPTOVERRIDESCHANGED,    /**< overridden fs options changed */
    } what;     /**< describes what happened to the inode */
};

#endif
