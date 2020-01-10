#ifndef I1541_CBMDOSFILEEVENTARGS_H
#define I1541_CBMDOSFILEEVENTARGS_H

/** Declaration of CbmdosFile Event arguments
 * @file
 */

#include <1541img/decl.h>

/** Arguments for events raised by CbmdosFile
 * @struct CbmdosFileEventArgs cbmdosfileeventargs.h \
 *     <1541img/cbmdosfileeventargs.h>
 */
C_CLASS_DECL(CbmdosFileEventArgs);

struct CbmdosFileEventArgs
{
    enum {
        CFE_TYPECHANGED,            /**< type of a file changed */
        CFE_NAMECHANGED,            /**< name of a file changed */
        CFE_DATACHANGED,            /**< content of a file changed */
        CFE_LOCKEDCHANGED,          /**< locked flag of a file changed */
        CFE_CLOSEDCHANGED,          /**< closed flag of a file changed */
	CFE_FORCEDBLOCKSCHANGED,    /**< forced blocks size of a file changed */
        CFE_RECORDLENGTHCHANGED     /**< REL record length of a file changed */
    } what;     /**< describes what happened to the file */
};

#endif
