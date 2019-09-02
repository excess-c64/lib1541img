#ifndef I1541_CBMDOSFILE_H
#define I1541_CBMDOSFILE_H

#include <stdint.h>

#include <1541img/cbmdosfileeventargs.h>

typedef struct FileData FileData;
typedef struct Event Event;

typedef enum CbmdosFileType
{
    CFT_DEL,
    CFT_SEQ,
    CFT_PRG,
    CFT_USR,
    CFT_REL
} CbmdosFileType;

const char *CbmdosFileType_name(CbmdosFileType type);

typedef struct CbmdosFile CbmdosFile;

CbmdosFile *CbmdosFile_create(void);
CbmdosFileType CbmdosFile_type(const CbmdosFile *self);
int CbmdosFile_setType(CbmdosFile *self, CbmdosFileType type);
const char *CbmdosFile_name(const CbmdosFile *self, uint8_t *length);
void CbmdosFile_setName(CbmdosFile *self, const char *name, uint8_t length);
const FileData *CbmdosFile_rdata(const CbmdosFile *self);
FileData *CbmdosFile_data(CbmdosFile *self);
uint8_t CbmdosFile_recordLength(const CbmdosFile *self);
int CbmdosFile_setRecordLength(CbmdosFile *self, uint8_t recordLength);
uint16_t CbmdosFile_forcedBlocks(const CbmdosFile *self);
void CbmdosFile_setForcedBlocks(CbmdosFile *self, uint16_t forcedBlocks);
int CbmdosFile_locked(const CbmdosFile *self);
void CbmdosFile_setLocked(CbmdosFile *self, int locked);
int CbmdosFile_closed(const CbmdosFile *self);
void CbmdosFile_setClosed(CbmdosFile *self, int closed);
Event *CbmdosFile_changedEvent(CbmdosFile *self);
void CbmdosFile_destroy(CbmdosFile *self);

#endif
