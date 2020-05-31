#ifndef CBMDOSINODE_H
#define CBMDOSINODE_H

#include <1541img/cbmdosinode.h>

CbmdosInode *CbmdosInode_create(void);
CbmdosInode *CbmdosInode_clone(const CbmdosInode *other);
void CbmdosInode_attach(CbmdosInode *self);
void CbmdosInode_detach(CbmdosInode *self);
void CbmdosInode_tryDestroy(CbmdosInode *self);

#endif
