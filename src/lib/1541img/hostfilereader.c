#include <stdint.h>

#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>

#define RBUFSIZE 1024

FileData *readHostFile(FILE *file)
{
    uint8_t buf[RBUFSIZE];
    size_t nread;

    FileData *filedata = FileData_create();

    while ((nread = fread(buf, 1, RBUFSIZE, file)))
    {
	if (FileData_append(filedata, buf, nread) < 0)
	{
	    FileData_destroy(filedata);
	    return 0;
	}
    }

    return filedata;
}

