#include <1541img/d64.h>
#include <1541img/zcfileset.h>
#include <1541img/filedata.h>
#include <1541img/zc45reader.h>
#include "log.h"

#include <1541img/zc45extractor.h>

static const int sectorcount[] = { 168, 168, 172, 175, 85 };

SOEXPORT D64 *extractZc45(const ZcFileSet *fileset)
{
    if (!fileset) return 0;
    ZcType type = ZcFileSet_type(fileset);
    if (type != ZT_4PACK && type != ZT_5PACK)
    {
        logmsg(L_ERROR, "extractZc45: trying to extract something that isn't "
                "a 4 or 5 file disk Zippack.");
        return 0;
    }

    D64 *extracted = D64_create(type == ZT_4PACK ? D64_STANDARD : D64_40TRACK);
    for (int i = 0; i < (type == ZT_4PACK ? 4 : 5); ++i)
    {
	const FileData *fd = ZcFileSet_rfileData(fileset, i);
	if (zc45_read(extracted, FileData_rcontent(fd), FileData_size(fd))
		!= sectorcount[i])
	{
            logfmt(L_ERROR, "extractZc45: extraction failed in part %d.", i+1);
	    D64_destroy(extracted);
	    return 0;
	}
    }

    logfmt(L_DEBUG, "extractZc45: %d-file zipcode successfully extracted.",
            type == ZT_4PACK ? 4 : 5);
    return extracted;
}

