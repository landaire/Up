#ifndef __STATIC__HG
#define __STATIC__HG

#include "../typedefs.h"
#include "../StdAfx.h"
#include <vector>
#include <QDateTime>
#include <QMetaType>
#include <../QtGui/QImage>

#ifdef _WIN32
#include <windows.h>
#endif
class Drive;
namespace Streams
{
class xDeviceFileStream;
}

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


static char KnownIds[][9] =
{
    "000D0000","00009000","00040000","02000000","00080000",
    "00020000","000A0000","000C0000","00400000","00004000",
    "000B0000","00002000","000F0000","00000002","00100000",
    "00300000","00500000","00010000","00000003","00000001",
    "00050000","00030000","00200000","00090000","00600000",
    "00070000","00005000","00060000","00001000","00005000",
    "000E0000","FFFE07D1","00007000","00008000"
};

static char KnownEquivalent[][0x20] =
{
    "Arcade Title", "Avatar Item",              "Cache File",       "Community Game",           "Game Demo",
    "Gamer Picture","Game Title",               "Game Trailer",     "Game Video",               "Installed Game",
    "Installer",    "IPTV Pause Buffer",        "License Store",    "Marketplace Content",      "Movie",
    "Music Video",  "Podcast Video","Profile",  "Publisher",        "Saved Game",               "Storage Download",
    "Theme",        "TV",                       "Video",            "Viral Video",              "Xbox Download",
    "Xbox Original Game","Xbox Saved Game",     "Installed Xbox 360 Title","Xbox Title",        "XNA",
    "Xbox 360 Dashboard","Games on Demand",     "Storage Pack"
};

static char KnownFolders[][0x20] =
{
    "Arcade Titles", "Avatar Items",            "Cache Files",      "Community Games",          "Game Demos",
    "Gamer Pictures","Game Titles",             "Game Trailers",    "Game Videos",              "Installed Games",
    "Installers",    "IPTV Pause Buffers",      "License Store",    "Marketplace Content",      "Movies",
    "Music Videos",  "Podcast Videos",          "Profiles",         "Publisher",                "Saved Games",
    "Storage Downloads","Themes",               "TV Content",       "Videos",                   "Viral Videos",
    "Xbox Downloads","Xbox Original Games",     "Xbox Saved Games", "Installed Xbox 360 Titles","Xbox Titles",
    "XNA Content",   "Xbox 360 Dashboard Content","Games on Demand","Storage Pack"
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
        UINT16 Seconds  : 5;
        UINT16 Minute   : 6;
        UINT16 Hour     : 5;
        UINT16 Day      : 5;
        UINT16 Month    : 4;
        UINT16 Year     : 7; // Relative to 2000
    } DateTime;
    DWORD AsDWORD;
} FAT_TIME_STAMP;
typedef FAT_TIME_STAMP *PFAT_TIME_STAMP;

struct xDirent
{
    // Size of the file/folder name
    BYTE NameSize;
    // System attributes
    BYTE Attributes;
    // Name of the file/folder
    char Name[0x2B];
    // The beginning cluster (cluster chain index 0)
    UINT32 ClusterStart;
    // Size of the file; 0 if folder/null
    UINT32 FileSize;
    // Entry creation date
    FAT_TIME_STAMP DateCreated;
    // Entry date for when it was last written
    FAT_TIME_STAMP DateLastWritten;
    // Entry date for when it was last accessed
    FAT_TIME_STAMP DateAccessed;
    // Offset of the entry (relative to disk start)
    UINT64 Offset;
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
};

Q_DECLARE_METATYPE(Progress)

struct File;
struct xVolume;

struct Folder
{
    std::vector<UINT32>     ClusterChain;
    xDirent                 Dirent;
    std::vector<Folder*>    CachedFolders;
    std::vector<File*>      CachedFiles;
    bool                    FatxEntriesRead;
    std::string             FullPath;
    Folder                  *Parent;
    xVolume                 *Volume;
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
    std::vector<UINT32> ClusterChain;
    xDirent             Dirent;
    std::string         FullPath;
    Folder              *Parent;
    xVolume             *Volume;
    QDateTime           DateCreated;
    QDateTime           DateModified;
    QDateTime           DateAccessed;
};

typedef struct _DEV_PARTITION
{
    std::string Name;
	unsigned int Sector;
	INT64 Size;
} DevPartition;

struct xVolume
{

    std::string		 Name;
	unsigned int Magic;					// Partition magic
    unsigned int SerialNumber;			// Partition serial number
	unsigned int SectorsPerCluster;		// Number of sectors per cluster
	unsigned int RootDirectoryCluster;	// The cluster in which the root directory is located
	UINT64		 DataStart;
	unsigned int Clusters;				// Total number of clusters in the partition
	BYTE EntrySize;						// Size of a chainmap entry
	UINT64		 Offset;				// Offset of the partition
	UINT64		 Size;					// Size of the partition
	UINT64		 AllocationTableSize;
	UINT64		 ClusterSize;
    unsigned int FatEntryShift;
    Folder       *Root;
    Drive        *Disk;
    xVolume()
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
    static const INT64 SystemAux = 0x8115200;
    static const INT64 SystemExtended = 0x12000400;
    static const INT64 Cache = 0x8000400;
    static const INT64 Data = 0x20000000;
};

struct UsbSizes
{
    static const INT64 CacheNoSystem = 0x4000000;
	static const INT64 Cache = 0x47FF000;
    static const INT64 SystemAux = 0x8000000;
    static const INT64 SystemExtended = 0xDFFFC00;
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

static const INT64 MaxFileSize		= 0x100000000;
static const int   FatxMagic		= 0x58544146;

struct HddSizes
{
    static const INT64 SystemCache		= 0x80000000;
    static const INT64 GameCache		= 0xA0E30000;
    static const INT64 Compatibility	= 0x10000000;
    static const INT64 SystemAux		= 0xCE30000;
    static const INT64 SystemExtended	= 0x8000000;
};

struct HddOffsets
{
    static const INT64 Data				= 0x130EB0000;
    static const INT64 Josh				= 0x800;
    static const INT64 SecuritySector	= 0x2000;
    static const INT64 SystemCache		= 0x80000;
    static const INT64 GameCache		= 0x80080000;
    static const INT64 SystemAux		= 0x10C080000;
    static const INT64 SystemExtended	= 0x118EB0000;
    static const INT64 Compatibility	= 0x120EB0000;
};
#endif
