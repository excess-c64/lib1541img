#ifndef I1541_CBMDOSFILEEVENTARGS_H
#define I1541_CBMDOSFILEEVENTARGS_H

typedef struct CbmdosFileEventArgs
{
    enum {
        CFE_TYPECHANGED,
        CFE_NAMECHANGED,
        CFE_DATACHANGED,
        CFE_LOCKEDCHANGED,
        CFE_CLOSEDCHANGED,
	CFE_FORCEDBLOCKSCHANGED
    } what;
} CbmdosFileEventArgs;

#endif
