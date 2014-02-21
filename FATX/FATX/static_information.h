#ifndef __STATIC__HG
#define __STATIC__HG

#include "../typedefs.h"
#include "../StdAfx.h"
#include <vector>
#include <QDateTime>
#include <QMetaType>
#include <QImage>

#ifdef _WIN32
#include <windows.h>
#endif

class Drive;
namespace Streams
{
class DeviceFileStream;
}

/** FAT Information **/
#define FAT32 4
#define FAT16 2

#define FAT_CLUSTER_AVAILABLE           (UINT32)0x00000000
#define FAT_CLUSTER_RESERVED            (UINT32)0xfffffff0
#define FAT_CLUSTER_BAD                 (UINT32)0xfffffff7
#define FAT_CLUSTER_MEDIA               (UINT32)0xfffffff8
#define FAT_CLUSTER_LAST                (UINT32)0xffffffff

#define FAT_CLUSTER16_AVAILABLE         (UINT16)0x0000
#define FAT_CLUSTER16_RESERVED          (UINT16)0xfff0
#define FAT_CLUSTER16_BAD               (UINT16)0xfff7
#define FAT_CLUSTER16_MEDIA             (UINT16)0xfff8
#define FAT_CLUSTER16_LAST              (UINT16)0xffff

#define FAT_FILE_NAME_LENGTH            0x2A

#define FAT_PATH_NAME_LIMIT             0xFA

#define FAT_DIRENT_NEVER_USED           0x00
#define FAT_DIRENT_DELETED              0xE5
#define FAT_DIRENT_NEVER_USED2          0xFF

/** STFS Information **/
#define STFS_PACKAGE_LIVE               0x4C495645
#define STFS_PACKAGE_PIRS               0x50495253
#define STFS_PACKAGE_CON                0x434F4E20

#define STFS_CONTENT_TYPE_ARCADE_TITLE      0xD0000
#define STFS_CONTENT_TYPE_AVATAR_ITEM       0x9000
#define STFS_CONTENT_TYPE_CACHE_FILE        0x40000
#define STFS_CONTENT_TYPE_COMMUNITY_GAME    0x2000000
#define STFS_CONTENT_TYPE_GAME_DEMO         0x80000
#define STFS_CONTENT_TYPE_GAMER_PICTURE     0x20000
#define STFS_CONTENT_TYPE_GAME_TITLE        0xA0000
#define STFS_CONTENT_TYPE_GAME_TRAILER      0xC0000
#define STFS_CONTENT_TYPE_GAME_VIDEO        0x400000
#define STFS_CONTENT_TYPE_INSTALLED_GAME    0x4000
#define STFS_CONTENT_TYPE_INSTALLER         0xB0000
#define STFS_CONTENT_TYPE_IPTV_PAUSE_BUFFER 0x2000
#define STFS_CONTENT_TYPE_LICENSE_STORE     0xF0000
#define STFS_CONTENT_TYPE_MARKETPLACE_CONTENT 0x2
#define STFS_CONTENT_TYPE_MOVIE             0x100000
#define STFS_CONTENT_TYPE_MUSIC_VIDEO       0x300000
#define STFS_CONTENT_TYPE_PODCAST_VIDEO     0x500000
#define STFS_CONTENT_TYPE_PROFILE           0x10000
#define STFS_CONTENT_TYPE_PUBLISHER         0x3
#define STFS_CONTENT_TYPE_SAVED_GAME        0x1
#define STFS_CONTENT_TYPE_STORAGE_DOWNLOAD  0x50000
#define STFS_CONTENT_TYPE_THEME             0x30000
#define STFS_CONTENT_TYPE_TV                0x200000
#define STFS_CONTENT_TYPE_VIDEO             0x90000
#define STFS_CONTENT_TYPE_VIRAL_VIDEO       0x600000
#define STFS_CONTENT_TYPE_XBOX_DOWNLOAD     0x70000
#define STFS_CONTENT_TYPE_XBOX_ORIGINAL_GAME 0x5000
#define STFS_CONTENT_TYPE_XBOX_SAVED_GAME   0x60000
#define STFS_CONTENT_TYPE_XBOX_360_TITLE    0x1000
#define STFS_CONTENT_TYPE_XNA               0xE0000


static std::map<const char*, const std::string> KnownContent = {
    {"000D0000", "Arcade Title"             },
    {"00009000", "Avatar Item"              },
    {"00040000", "Cache File"               },
    {"02000000", "Community Game"           },
    {"00080000", "Game Demo"                },
    {"00020000", "Gamer Picture"            },
    {"000A0000", "Game Title"               },
    {"000C0000", "Game Trailer"             },
    {"00400000", "Game Video"               },
    {"00004000", "Installed Game"           },
    {"000B0000", "Installer"                },
    {"00002000", "IPTV Pause Buffer"        },
    {"000F0000", "License Store"            },
    {"00000002", "Marketplace Content"      },
    {"00100000", "Movie"                    },
    {"00300000", "Music Video"              },
    {"00500000", "Podcast Video"            },
    {"00010000", "Profile"                  },
    {"00000003", "Publisher"                },
    {"00000001", "Saved Game"               },
    {"00050000", "Storage Download"         },
    {"00030000", "Theme"                    },
    {"00200000", "TV"                       },
    {"00090000", "Video"                    },
    {"00600000", "Viral Video"              },
    {"00070000", "Xbox Download"            },
    {"00005000", "Xbox Original Game"       },
    {"00060000", "Xbox Save Game"           },
    {"00001000", "Installed Xbox 360 Title" },
    {"00005000", "Xbox Title"               },
    {"000E0000", "XNA"                      },
    {"FFFE07D1", "Xbox 360 Dashboard"       },
    {"00007000", "Games on Demand"          },
    {"00008000", "Storage Pack"             },
};

enum DeviceType
{
	DeviceDisk,
	DeviceUsb,
    DeviceBackup
};

typedef union _FAT_TIME_STAMP
{
    struct
    {
        uint16_t Seconds  : 5;
        uint16_t Minute   : 6;
        uint16_t Hour     : 5;
        uint16_t Day      : 5;
        uint16_t Month    : 4;
        uint16_t Year     : 7; // Relative to 2000
    } DateTime;
    uint32_t AsDWORD;
} TimeStamp;

struct FATXDirent
{
    // Size of the file/folder name
    uint8_t NameSize;
    // System attributes
    uint8_t Attributes;
    // Name of the file/folder
    char Name[0x2B];
    // The beginning cluster (cluster chain index 0)
    uint32_t ClusterStart;
    // Size of the file; 0 if folder/null
    uint32_t FileSize;
    // Entry creation date
    TimeStamp DateCreated;
    // Entry date for when it was last written
    TimeStamp DateLastWritten;
    // Entry date for when it was last accessed
    TimeStamp DateAccessed;
    // Offset of the entry (relative to disk start)
    uint64_t Offset;
};

struct Progress
{
    int Current;
    int Maximum;
    std::string FilePath;
    std::string FileName;
    Drive *Device;
    QImage PackageImage;
    bool IsStfsPackage;
    QString PackageName;
    bool Done;
    volatile bool Cancel;
};

Q_DECLARE_METATYPE(Progress)

struct File;
struct FATXVolume;

struct Folder
{
    std::vector<uint32_t>     ClusterChain;
    FATXDirent                 Dirent;
    std::vector<std::shared_ptr<Folder>> CachedFolders;
    std::vector<std::shared_ptr<File>>   CachedFiles;
    bool                    FatxEntriesRead;
    std::string             FullPath;
    std::shared_ptr<Folder> Parent;
    std::shared_ptr<FATXVolume> Volume;
    QDateTime               DateCreated;
    QDateTime               DateModified;
    QDateTime               DateAccessed;

    bool IsTitleIDFolder( void )
    {
        std::string Name(Dirent.Name);
        // Title ID's are 8 digits long; this shit isn't a Title ID folder if it's
        // longer or shorter
        if (Name.length() != 8)
        {
            return false;
        }
        // It was 8 digits long, let's check if the characters are valid hex digits
        char AcceptableChars[17] = "0123456789ABCDEF";
        for (int i = 0; i < Dirent.NameSize; i++)
        {
            bool Acceptable = false;
            for (int j = 0; j < 16; j++)
            {
                if (Dirent.Name[i] == AcceptableChars[j])
                {
                    Acceptable = true;
                    break;
                }
            }
            if (!Acceptable)
            {
                return false;
            }
        }
        return true;
    }
};

struct File
{
    std::vector<uint32_t> ClusterChain;
    FATXDirent             Dirent;
    std::string         FullPath;
    std::shared_ptr<Folder>     Parent;
    std::shared_ptr<FATXVolume> Volume;
    QDateTime           DateCreated;
    QDateTime           DateModified;
    QDateTime           DateAccessed;
    bool                FullClusterChainRead;
};

typedef struct _DEV_PARTITION
{
    std::string Name;
    uint32_t Sector;
	uint64_t4_t Size;
} DevPartition;

/**
 * @brief The FATX Volume struct
 */
struct FATXVolume
{
    std::string		 Name;
    uint32_t Magic;					// Partition magic
    uint32_t SerialNumber;			// Partition serial number
    uint32_t SectorsPerCluster;		// Number of sectors per cluster
    uint32_t RootDirectoryCluster;	// The cluster in which the root directory is located
    uint64_t DataStart;
    uint32_t Clusters;				// Total number of clusters in the partition
    uint8_t  EntrySize;						// Size of a chainmap entry
    uint64_t Offset;				// Offset of the partition
    uint64_t Size;					// Size of the partition
    uint64_t AllocationTableSize;
    uint64_t ClusterSize;
    uint32_t FatEntryShift;
    uint64_t FreeClusterRangeStart;
    std::shared_ptr<Folder> Root;
    std::shared_ptr<Drive> Disk;
    FATXVolume()
    {
        // Set all members to 0
        Magic = 0;
        SerialNumber = 0;
        SectorsPerCluster = 0;
        RootDirectoryCluster = 0;
        DataStart = 0;
        Clusters = 0;
        EntrySize = 0;
        Offset = 0;
        Size = 0;
        AllocationTableSize = 0;
        ClusterSize = 0;
        FatEntryShift = 0;
        Root = 0;
        Disk = 0;
    }
};

enum StfsOffsets
{
    StfsTitleID = 0x360,
    StfsConsoleID = 0x36c,
    StfsDeviceID = 0x3FD,
    StfsDisplayName = 0x411,
    StfsTitleName = 0x1691,
    StfsProfileID = 0x371,
    StfsContentImage = 0x171A,
    StfsContentImageSize = 0x1712,
    StfsTitleImageSize = 0x1716,
    StfsTitleImage = 0x571A
};

struct UsbOffsets
{
    static const uint64_t4_t SystemAux = 0x8115200;
    static const uint64_t4_t SystemExtended = 0x12000400;
    static const uint64_t4_t Cache = 0x8000400;
    static const uint64_t4_t Data = 0x20000000;
};

struct UsbSizes
{
    static const uint64_t4_t CacheNoSystem = 0x4000000;
	static const uint64_t4_t Cache = 0x47FF000;
    static const uint64_t4_t SystemAux = 0x8000000;
    static const uint64_t4_t SystemExtended = 0xDFFFC00;
};

struct Attributes
{
    static const int READONLY     = 0x00000001;
    static const int HIDDEN       = 0x00000002;
    static const int SYSTEM       = 0x00000004;
    static const int DIRECTORY    = 0x00000010;
    static const int ARCHIVE      = 0x00000020;
    static const int DEVICE       = 0x00000040;
    static const int NORMAL       = 0x00000080;
    static const int TEMPORARY    = 0x00000100;
};

static const uint64_t4_t MaxFileSize		= 0x100000000;
static const int   FatxMagic		= 0x58544146;

struct HddSizes
{
    static const uint64_t4_t SystemCache		= 0x80000000;
    static const uint64_t4_t GameCache		= 0xA0E30000;
    static const uint64_t4_t SystemPartition	= 0x10000000;
    static const uint64_t4_t SystemAux		= 0xCE30000;
    static const uint64_t4_t SystemExtended	= 0x8000000;
};

struct HddOffsets
{
    static const uint64_t4_t Data				= 0x130EB0000;
    static const uint64_t4_t Josh				= 0x800;
    static const uint64_t4_t SecuritySector	= 0x2000;
    static const uint64_t4_t SystemCache		= 0x80000;
    static const uint64_t4_t GameCache		= 0x80080000;
    static const uint64_t4_t SystemAux		= 0x10C080000;
    static const uint64_t4_t SystemExtended	= 0x118EB0000;
    static const uint64_t4_t SystemPartition	= 0x120EB0000;
};
#endif
