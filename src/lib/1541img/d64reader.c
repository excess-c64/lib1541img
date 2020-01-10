#include <string.h>

#include "log.h"
#include <1541img/filedata.h>
#include <1541img/hostfilereader.h>
#include <1541img/d64.h>
#include <1541img/track.h>
#include <1541img/sector.h>

#include <1541img/d64reader.h>

D64 *readD64FromFileData(const FileData *file)
{
    size_t size = FileData_size(file);
    D64Type type;

    switch (size)
    {
        case 174848UL:
        case 175531UL:
            type = D64_STANDARD;
            break;
        case 196608UL:
        case 197376UL:
            type = D64_40TRACK;
            break;
        case 205312UL:
        case 206114UL:
            type = D64_42TRACK;
            break;
        default:
            logmsg(L_WARNING, "readD64FromFileData: not a valid D64 file.");
            return 0;
    }

    D64 *d64 = D64_create(type);
    const uint8_t *bytes = FileData_rcontent(file);
    uint8_t tracks = D64_tracks(d64);
    for (uint8_t trackno = 1; trackno <= tracks; ++trackno)
    {
        Track *track = D64_track(d64, trackno);
        uint8_t sectors = Track_sectors(track);
        for (uint8_t sectno = 0; sectno < sectors; ++sectno)
        {
            uint8_t *sectbytes = Sector_content(Track_sector(track, sectno));
            memcpy(sectbytes, bytes, 256);
            bytes += 256;
        }
    }
    return d64;
}

D64 *readD64(FILE *file)
{
    FileData *data = readHostFile(file);
    if (!data)
    {
        logmsg(L_WARNING, "readD64: error reading file.");
        return 0;
    }

    D64 *d64 = readD64FromFileData(data);
    FileData_destroy(data);
    return d64;
}
