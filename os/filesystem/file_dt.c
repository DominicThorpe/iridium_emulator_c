#include "file_dt.h"
#include "file_meta.h"


// Read last write date and time into datetime bit field passed by pointer from Filedir struct
void create_date_time(DateTime* dt, Filedir* filedir) {
    dt->year = (filedir->DIR_WrtDate & 0xFE00) >> 9;
    dt->month = (filedir->DIR_WrtDate & 0x01E0) >> 5;
    dt->day = filedir->DIR_WrtDate & 0x001F;

    dt->hour = (filedir->DIR_WrtTime & 0xF800) >> 11;
    dt->minute = (filedir->DIR_WrtTime & 0x07E0) >> 5;
    dt->second = filedir->DIR_WrtTime & 0x001F;
    dt->subsec = 0; // TODO: add if necessary
}
