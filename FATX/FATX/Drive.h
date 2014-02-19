#ifndef __DRIVE__HG
#define __DRIVE__HG
/** Standard lib **/
#include <vector>
/** Qt **/
#include <QRegExp>
#include <QDateTime>
#include <QDebug>
#include <QObject>

#include "stdafx.h"
#include "io/device_stream.h"
#include "io/file_stream.h"
#include "io/multi_file_stream.h"
#include "typedefs.h"
#include "static_information.h"

// Disable warnings 4018 and 4082
//(unsigned type mismatch and nonstandard extension used in qualified name, respectively)
#ifdef _WIN32
#pragma warning(disable : 4018 4482)
#endif

class Drive : public QObject
{
    using std::string;
    using std::vector;
    using std::stringstream;
    using shared_ptr;

    Q_OBJECT

private:
    std::vector<FatxVolume*> validVolumes;
    void	SetValidPartitions      ( void );
    void	InitializePartitions    ( void );
    void	FatxProcessBootSector   ( FatxVolume* ref );
    void    DestroyFolder           (shared_ptr<Folder> Directory);
    uint8_t	cntlzw                  (uint32_t val);
    std::vector<std::string> _partitions;

    string              friendlyName;
    string              friendlySize;
    DeviceType          type;
    bool                isDevKitDrive;
    Streams::IStream*   deviceStream;

public:
    Drive(string path, string friendlyName, bool isUsb );
    ~Drive(void);

    uint64_t GetLength( void );

    void    ReadClusterChain        (std::vector<uint32_t>& Chain, xDirent Entry, const shared_ptr<FatxVolume> RelativePartition, int Count=-1);

    std::vector<std::string>Partitions          ( void );
    string                  GetDiskName         ( void );
    uint64_t                PartitionGetLength  ( string Partition );
    void                    Close               ( void );
    size_t                  GetFileCount( shared_ptr<Folder> f );
    size_t                  GetTotalFileCount   ( shared_ptr<Folder> f );
    size_t                  GetFolderCount      ( shared_ptr<Folder> f );
    size_t                  GetTotalFolderCount ( shared_ptr<Folder> f );
    void                    ReadDirectoryEntries(shard_ptr<Folder> Directory);
    shared_ptr<Folder>      FolderFromPath      ( string Path );
    shared_ptr<File>        FileFromPath( string Path );
    void                    CopyFileToLocalDisk ( File *dest, const string &Output);
    void                    CopyFileToLocalDisk (const string &path, const string &outputDirectory);
    void                    CopyFolderToLocalDisk( shared_ptr<Folder> f, const string &Output );
    void                    CopyFolderToLocalDisk( const string &folderPath, const string &outputFolderPath);
    void                    CreateDirent        ( xDirent const &d );
    void                    FindFreeClusters(uint32_t startingCluster, size_t clusterCount, const shared_ptr<FatxVolume> partition, std::vector<uint32_t>& outChain);
    size_t                  GetSectorsFromSecuritySector( void );
signals:
    void                    FileProgressChanged(const Progress &progress);
};
#endif
