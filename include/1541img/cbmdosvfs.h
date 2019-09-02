#ifndef I1541_CBMDOSVFS_H
#define I1541_CBMDOSVFS_H

#include <stdint.h>

#include <1541img/cbmdosvfseventargs.h>

typedef struct Event Event;
typedef struct CbmdosFile CbmdosFile;

typedef struct CbmdosVfs CbmdosVfs;

CbmdosVfs *CbmdosVfs_create(void);
uint8_t CbmdosVfs_dosver(const CbmdosVfs *self);
void CbmdosVfs_setDosver(CbmdosVfs *self, uint8_t dosver);
const char *CbmdosVfs_name(const CbmdosVfs *self, uint8_t *length);
void CbmdosVfs_setName(CbmdosVfs *self, const char *name, uint8_t length);
const char *CbmdosVfs_id(const CbmdosVfs *self, uint8_t *length);
void CbmdosVfs_setId(CbmdosVfs *self, const char *id, uint8_t length);
unsigned CbmdosVfs_fileCount(const CbmdosVfs *self);
const CbmdosFile *CbmdosVfs_rfile(const CbmdosVfs *self, unsigned pos);
CbmdosFile *CbmdosVfs_file(CbmdosVfs *self, unsigned pos);
int CbmdosVfs_delete(CbmdosVfs *self, const CbmdosFile *file);
int CbmdosVfs_deleteAt(CbmdosVfs *self, unsigned pos);
int CbmdosVfs_append(CbmdosVfs *self, CbmdosFile *file);
int CbmdosVfs_insert(CbmdosVfs *self, CbmdosFile *file, unsigned pos);
Event *CbmdosVfs_changedEvent(CbmdosVfs *self);
void CbmdosVfs_destroy(CbmdosVfs *self);

#endif
