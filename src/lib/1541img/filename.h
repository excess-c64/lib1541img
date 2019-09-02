#ifndef FILENAME_H
#define FILENAME_H

typedef struct Filename Filename;

Filename *Filename_create(void);
Filename *Filename_clone(const Filename *self);
const char *Filename_full(const Filename *self);
const char *Filename_dir(const Filename *self);
const char *Filename_base(const Filename *self);
const char *Filename_ext(const Filename *self);
void Filename_setFull(Filename *self, const char *full);
void Filename_setDir(Filename *self, const char *dir);
void Filename_setBase(Filename *self, const char *base);
void Filename_setExt(Filename *self, const char *ext);
void Filename_destroy(Filename *self);

#endif
