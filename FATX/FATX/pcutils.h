#ifndef PCUTILS_H
#define PCUTILS_H
#include "Drive.h"

class PCUtils
{
public:
    static std::vector<Drive*> GetFATXDrives( bool IncludeHardDisks );
private:
    typedef struct _DISK_DRIVE_INFORMATION
    {
        TCHAR Path[1024];
        TCHAR FriendlyName[1024];
    } DISK_DRIVE_INFORMATION;

    static std::vector<Drive*> GetLogicalPartitions( void );
    static void GetPhysicalDisks( std::vector<DISK_DRIVE_INFORMATION> &OutVector );
};

#endif // PCUTILS_H
