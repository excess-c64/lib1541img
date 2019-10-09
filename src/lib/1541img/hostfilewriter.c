#include <1541img/filedata.h>
#include <1541img/hostfilewriter.h>

int writeHostFile(const FileData *data, FILE *file)
{
    if (fwrite(FileData_rcontent(data), FileData_size(data), 1, file) != 1)
    {
	return -1;
    }
    else
    {
	return 0;
    }
}

