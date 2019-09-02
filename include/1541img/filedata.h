#ifndef I1541_FILEDATA_H
#define I1541_FILEDATA_H

#include <stddef.h>
#include <stdint.h>

typedef struct Event Event;

#define FILEDATA_MAXSIZE 1024*1024UL

typedef struct FileData FileData;

FileData *FileData_create(void);
FileData *FileData_clone(const FileData *self);
size_t FileData_size(const FileData *self);
const uint8_t *FileData_rcontent(const FileData *self);
int FileData_append(FileData *self, const uint8_t *data, size_t size);
int FileData_appendByte(FileData *self, uint8_t byte);
Event *FileData_changedEvent(FileData *self);
void FileData_destroy(FileData *self);

#endif
