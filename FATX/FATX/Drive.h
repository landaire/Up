#ifndef __DRIVE__HG
#define __DRIVE__HG
#include "../stdafx.h"
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

// Disable warnings 4018 and 4082
//(unsigned type mismatch and nonstandard extension used in qualified name, respectively)
#ifdef _WIN32
#pragma warning(disable : 4018 4482)
#endif

class Drive : public QObject
{
    Q_OBJECT
private:
    std::vector<xVolume*> *ValidVolumes;
    void	SetValidPartitions      ( void );
    void	InitializePartitions    ( void );
    void	FatxProcessBootSector   ( xVolume* ref );
    BYTE	cntlzw                  (unsigned int val);
    std::vector<std::string> _partitions;

    void    DestroyFolder           (Folder *Directory);

public:
    Drive(std::string path, std::string friendlyName, bool isUsb );
    ~Drive(void);

    INT64 GetLength( void );

    void    ReadClusterChain        (std::vector<UINT32>& Chain, xDirent Entry, xVolume RelativePartition, int Count=-1);

    Streams::IStream*       DeviceStream;
    std::vector<std::string>Partitions          ( void );
    QString                 GetDiskName         ( void );
    UINT64                  PartitionGetLength  ( std::string Partition );
    void                    Close               ( void );
    DWORD                   GetFileCount( Folder *f );
    DWORD                   GetTotalFileCount   ( Folder *f );
    DWORD                   GetFolderCount      ( Folder *f );
    DWORD                   GetTotalFolderCount ( Folder *f );
    void                    ReadDirectoryEntries(Folder* Directory);
    Folder*                 FolderFromPath      ( std::string Path );
    File*                   FileFromPath        ( std::string Path );
    void                    CopyFileToLocalDisk ( File *dest, const std::string &Output);
    void                    CopyFileToLocalDisk ( const std::string &Path, const std::string &Output);
    void                    CopyFolderToLocalDisk( Folder *f, const std::string &Output );
    void                    CopyFolderToLocalDisk( const std::string &Path, const std::string &Output);
    void                    CreateDirent        ( const xDirent& d );
    void                    FindFreeClusters(DWORD StartingCluster, DWORD ClusterCount, xVolume* Partition, std::vector<DWORD>& OutChain);
    DWORD                   GetSectorsFromSecuritySector( void );

    std::string             FriendlyName;
    std::string             FriendlySize;

    DeviceType              Type;
    bool                    IsDevKitDrive;
signals:
    void                    FileProgressChanged(const Progress &progress);
};
#endif
