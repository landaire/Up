#ifndef __DRIVE__HG
#define __DRIVE__HG
#include "../StdAfx.h"
#include <vector>
#include "../xexception.h"
#include "../IO/xDeviceStream.h"
#include "../IO/xFileStream.h"
#include "../IO/xMultiFileStream.h"
#include "../typedefs.h"
#include "../FATX/StaticInformation.h"
#include <QRegExp>
#include <QDateTime>
#include <QDebug>
#include <QObject>


#ifdef _WIN32
#include <windows.h>
#include <devguid.h>    // for GUID_DEVCLASS_CDROM etc
#include <setupapi.h>
#include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN, CM_Get_Parent and CM_Get_Device_ID
#define INITGUID
#include <DEVPKEY.h>

#define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

#pragma comment (lib, "setupapi.lib")

#else
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

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



    static std::vector<Drive *> GetLogicalPartitions( void );

    static void GetPhysicalDisks( std::vector<DISK_DRIVE_INFORMATION> &OutVector);

    std::vector<xVolume*> *ValidVolumes;
    void	SetValidPartitions      ( void );
    void	InitializePartitions    ( void );
    void	FatxProcessBootSector   ( xVolume* ref );
    BYTE	cntlzw                  (unsigned int val);
    std::vector<std::string> _partitions;

    void    DestroyFolder           (Folder *Directory);

public:
    Drive( TCHAR* Path, TCHAR* FriendlyName, bool IsUsb );
    ~Drive(void);

    INT64 GetLength( void );

    void    ReadClusterChain        (std::vector<UINT32>& Chain, xDirent Entry, xVolume RelativePartition);

    Streams::IStream*       DeviceStream;
    static std::vector<Drive *>  GetFATXDrives( bool HardDisks );
    std::vector<std::string>Partitions          ( void );
    QString                 GetDiskName         ( void );
    UINT64                  PartitionGetLength  ( std::string Partition );
    void                    Close               ( void );
    DWORD                   GetFileCount( Folder *f );
    DWORD                   GetTotalFileCount   ( Folder *f );
    DWORD                   GetFolderCount      ( Folder *f );
    DWORD                   GetTotalFolderCount ( Folder *f );
    void                    ReadDirectoryEntries(Folder* Directory);
    Folder                  *FolderFromPath     ( std::string Path );
    File                    *FileFromPath       ( std::string Path );
    void                    CopyFileToLocalDisk ( File *dest, std::string Output);
    void                    CopyFileToLocalDisk ( std::string Path, std::string Output);
    // do not know why this declaration is here.  leaving it here for whatever reason
    // void                    CopyFileToLocalDisk ( Folder *dest, std::string Output );
    void                    CopyFolderToLocalDisk( Folder *f, std::string Output );
    void                    CopyFolderToLocalDisk( std::string Path, std::string Output);

    std::wstring            FriendlyName;
    std::string             FriendlySize;

    DeviceType              Type;
    bool                    IsDevKitDrive;
signals:
    void                    FileProgressChanged(const Progress &progress);
};
#endif
