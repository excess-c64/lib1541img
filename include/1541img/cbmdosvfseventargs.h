#ifndef I1541_CBMDOSVFSEVENTARGS_H
#define I1541_CBMDOSVFSEVENTARGS_H

#include <1541img/cbmdosfileeventargs.h>

typedef struct CbmdosVfsEventArgs
{
    enum {
        CVE_DOSVERCHANGED,
        CVE_NAMECHANGED,
        CVE_IDCHANGED,
        CVE_FILEADDED,
        CVE_FILEDELETED,
        CVE_FILECHANGED
    } what;
    const CbmdosFileEventArgs *fileEventArgs;
    unsigned filepos;
} CbmdosVfsEventArgs;

#endif
