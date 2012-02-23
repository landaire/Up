#ifndef __DRIVE__HG
#define __DRIVE__HG
#include "../StdAfx.h"
#include <vector>
#include "../IO/xDeviceStream.h"
#include "../IO/xFileStream.h"
#include "../IO/xMultiFileStream.h"
#include "../typedefs.h"
#include "../FATX/StaticInformation.h"
#include <QRegExp>
#include <QDateTime>
#include <QDebug>
#include <QObject>


using namespace std;

#ifdef _WIN32
#include <windows.h>
#include <devguid.h>    // for GUID_DEVCLASS_CDROM etc
#include <setupapi.h>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN, CM_Get_Parent and CM_Get_Device_ID
#define INITGUID
#include <DEVPKEY.H>

#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

#pragma comment (lib, "setupapi.lib")

#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __APPLE__
#include <sys/disk.h>
#include <sys/ioctl.h>
#endif

// linux
#ifdef __linux
#include <linux/hdreg.h>
#endif

#endif

// Disable warnings 4018 and 4082
//(unsigned type mismatch and nonstandard extension used in qualified name, respectively)
#ifdef _WIN32
#pragma warning(disable : 4018 4482)
#endif

class Drive : public QObject
{
    Q_OBJECT
private:

    typedef struct _DISK_DRIVE_INFORMATION
    {
        TCHAR Path[1024];
        TCHAR FriendlyName[1024];
    } DISK_DRIVE_INFORMATION;



    static vector<Drive *> GetLogicalPartitions( void );

    static vector<DISK_DRIVE_INFORMATION> GetPhysicalDisks( void );

    vector<xVolume*> ValidVolumes;
    void	SetValidPartitions      ( void );
    void	InitializePartitions    ( void );
    void	FatxProcessBootSector   ( xVolume* ref );
    BYTE	cntlzw                  (unsigned int val);
    vector<string> _partitions;
    void    ReadDirectoryEntries    (Folder* Directory);

    void    DestroyFolder           (Folder *Directory);

public:
    Drive( TCHAR* Path, TCHAR* FriendlyName, bool IsUsb );
    ~Drive(void);

    INT64 GetLength( void );

    void    ReadClusterChain        (std::vector<UINT32>& Chain, xDirent Entry, xVolume RelativePartition);

    Streams::IStream*       DeviceStream;
    static vector<Drive *> GetFATXDrives( bool HardDisks );
    vector<string>          Partitions          ( void );
    UINT64                  PartitionGetLength  ( string Partition );
    void                    Close               ( void );
    Folder                  *FolderFromPath     ( string Path );
    File                    *FileFromPath       ( string Path );
    void                    CopyFileToLocalDisk ( File *dest, string Output);
    void                    CopyFileToLocalDisk ( string Path, string Output);

    wstring                 FriendlyName;
    string                  FriendlySize;

    DeviceType              Type;
    bool                    IsDevKitDrive;
signals:
    void                    FileProgressChanged(const Progress &progress);
};
#endif
