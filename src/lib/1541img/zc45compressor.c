#include <string.h>

#include <1541img/d64.h>
#include <1541img/sector.h>
#include <1541img/zcfileset.h>
#include <1541img/filedata.h>
#include <1541img/zc45writer.h>
#include "log.h"

#include <1541img/zc45compressor.h>

SOEXPORT ZcFileSet *compressZc45(const D64 *d64)
{
    char name[17];
    uint8_t buf[MAXZCFILESIZE];
    if (!d64) return 0;
    D64Type type = D64_type(d64);
    const uint8_t *bam = Sector_rcontent(D64_rsector(d64, 18, 0));
    int namelen = 16;
    while (namelen && bam[0x8f + namelen] == 0xa0) --namelen;
    memcpy(name, bam+0x90, namelen);
    name[namelen] = 0;

    ZcFileSet *compressed = ZcFileSet_create(
            type == D64_STANDARD ? ZT_4PACK : ZT_5PACK, name);

    for (int i = 0; i < (type == D64_STANDARD ? 4 : 5); ++i)
    {
        size_t filelen = zc45_write(buf, MAXZCFILESIZE, i+1, d64);
        if (!filelen)
        {
            logfmt(L_ERROR, "compressZc45: compression failed in part %d.",
                    i+1);
            ZcFileSet_destroy(compressed);
            return 0;
        }
        FileData *fd = ZcFileSet_fileData(compressed, i);
        if (FileData_append(fd, buf, filelen) < 0)
        {
            logfmt(L_ERROR, "compressZc45: compression failed in part %d.",
                    i+1);
            ZcFileSet_destroy(compressed);
            return 0;
        }
    }

    logfmt(L_DEBUG, "compressZc45: %d-file zipcode successfully created.",
            type == D64_40TRACK ? 5 : 4);
    return compressed;
}

