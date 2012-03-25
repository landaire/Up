#include "stfspackage.h"
using namespace std;
using namespace Streams;

STFSPackage::STFSPackage( Streams::IStream *Stream )
{
    this->Stream = Stream;
}

STFSPackage::~STFSPackage( void )
{

}

bool STFSPackage::IsStfsPackage( void )
{
    if (Stream->Length() < 0xA000)
        return false;
    DWORD Magic = this->Magic();
    switch (Magic)
    {
    case STFS_PACKAGE_CON:
    case STFS_PACKAGE_LIVE:
    case STFS_PACKAGE_PIRS:
        return true;
    default:
        return false;
    }
}

DWORD STFSPackage::Magic( void )
{
    Stream->SetPosition(0x0);
    return Stream->ReadUInt32();
}

DWORD STFSPackage::ContentType( void )
{
    Stream->SetPosition(0x344);
    return Stream->ReadUInt32();
}

DWORD STFSPackage::TitleId( void )
{
    Stream->SetPosition(0x360);
    return Stream->ReadUInt32();
}

UINT64 STFSPackage::ConsoleId( void )
{
    Stream->SetPosition(0x369);
    return Stream->ReadUInt64() & 0xFFFFFF0000000000;
}

UINT64 STFSPackage::ProfileId( void )
{
    Stream->SetPosition(0x371);
    return Stream->ReadUInt64();
}

QString STFSPackage::DisplayName( int Locale )
{
    Stream->SetPosition(0x411 + (0x80 * Locale));
    BYTE dn[0x80] = {0};
    Stream->Read((BYTE*)&dn, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dn + i, sizeof(short), sizeof(char));
    return QString::fromUtf16((const ushort*)&dn);
}

QString STFSPackage::Description( int Locale )
{
    Stream->SetPosition(0xD11 + (0x80 * Locale));
    BYTE dd[0x80] = {0};
    Stream->Read((BYTE*)&dd, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dd + i, sizeof(short), sizeof(char));
    return QString::fromUtf16((const ushort*)&dd);
}

QString STFSPackage::TitleName( int Locale )
{
    Stream->SetPosition(0x1691 + (0x80 * Locale));
    BYTE dd[0x80] = {0};
    Stream->Read((BYTE*)&dd, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dd + i, sizeof(short), sizeof(char));
    return QString::fromUtf16((const ushort*)&dd);
}

QImage STFSPackage::ThumbnailImage( void )
{
    Stream->SetPosition(0x1712);
    DWORD Size = Stream->ReadUInt32();
    Stream->SetPosition(0x171A);
    BYTE Image[0x4000] = {0};
    Stream->Read((BYTE*)&Image, Size);
    return QImage::fromData(QByteArray::fromRawData((const char*)&Image, Size));
}

QImage STFSPackage::TitleImage( void )
{
    Stream->SetPosition(0x1716);
    DWORD Size = Stream->ReadUInt32();
    Stream->SetPosition(0x571A);
    BYTE Image[0x4000] = {0};
    Stream->Read((BYTE*)&Image, Size);
    return QImage::fromData(QByteArray::fromRawData((const char*)&Image, Size));
}

QString STFSPackage::ContentType_s( void )
{
    DWORD ContentType = this->ContentType();
    switch (ContentType)
    {
    case STFS_CONTENT_TYPE_ARCADE_TITLE:
        return QString::fromAscii("Arcade Title");
    case STFS_CONTENT_TYPE_AVATAR_ITEM:
        return QString::fromAscii("Avatar Item");
    case STFS_CONTENT_TYPE_CACHE_FILE:
        return QString::fromAscii("Cache File");
    case STFS_CONTENT_TYPE_COMMUNITY_GAME:
        return QString::fromAscii("Community Game");
    case STFS_CONTENT_TYPE_GAME_DEMO:
        return QString::fromAscii("Game Demo");
    case STFS_CONTENT_TYPE_GAMER_PICTURE:
        return QString::fromAscii("Gamer Picture");
    case STFS_CONTENT_TYPE_GAME_TITLE:
        return QString::fromAscii("Game Title");
    case STFS_CONTENT_TYPE_GAME_TRAILER:
        return QString::fromAscii("Game Trailer");
    case STFS_CONTENT_TYPE_GAME_VIDEO:
        return QString::fromAscii("Game Video");
    case STFS_CONTENT_TYPE_INSTALLED_GAME:
        return QString::fromAscii("Installed Game");
    case STFS_CONTENT_TYPE_INSTALLER:
        return QString::fromAscii("Installer");
    case STFS_CONTENT_TYPE_IPTV_PAUSE_BUFFER:
        return QString::fromAscii("IPTV Pause Buffer");
    case STFS_CONTENT_TYPE_LICENSE_STORE:
        return QString::fromAscii("License Store");
    case STFS_CONTENT_TYPE_MARKETPLACE_CONTENT:
        return QString::fromAscii("Marketplace Content");
    case STFS_CONTENT_TYPE_MOVIE:
        return QString::fromAscii("Movie");
    case STFS_CONTENT_TYPE_MUSIC_VIDEO:
        return QString::fromAscii("Music Video");
    case STFS_CONTENT_TYPE_PODCAST_VIDEO:
        return QString::fromAscii("Podcast Video");
    case STFS_CONTENT_TYPE_PROFILE:
        return QString::fromAscii("Profile");
    case STFS_CONTENT_TYPE_PUBLISHER:
        return QString::fromAscii("Publisher");
    case STFS_CONTENT_TYPE_SAVED_GAME:
        return QString::fromAscii("Saved Game");
    case STFS_CONTENT_TYPE_STORAGE_DOWNLOAD:
        return QString::fromAscii("Storage Download");
    case STFS_CONTENT_TYPE_THEME:
        return QString::fromAscii("Theme");
    case STFS_CONTENT_TYPE_TV:
        return QString::fromAscii("TV");
    case STFS_CONTENT_TYPE_VIDEO:
        return QString::fromAscii("Video");
    case STFS_CONTENT_TYPE_VIRAL_VIDEO:
        return QString::fromAscii("Viral Video");
    case STFS_CONTENT_TYPE_XBOX_DOWNLOAD:
        return QString::fromAscii("Xbox Download");
    case STFS_CONTENT_TYPE_XBOX_ORIGINAL_GAME:
        return QString::fromAscii("Xbox Original Game");
    case STFS_CONTENT_TYPE_XBOX_SAVED_GAME:
        return QString::fromAscii("Xbox Saved Game");
    case STFS_CONTENT_TYPE_XBOX_360_TITLE:
        return QString::fromAscii("Xbox 360 Title");
    case STFS_CONTENT_TYPE_XNA:
        return QString::fromAscii("XNA");
    default:
        return QString::fromAscii("STFS Package");
    }
}
