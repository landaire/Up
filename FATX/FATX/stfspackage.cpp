#include "stfspackage.h"
using namespace std;
using namespace Streams;

StfsPackage::StfsPackage( Streams::IStream *Stream )
{
    this->Stream = Stream;
    checkedIfIsStfsPackage = false;
    _isStfsPackage = false;
    _magic = 0;
    _contentType = 0;
    _titleId = 0;
    _consoleId = 0;
    _profileId = 0;
    _displayName = "";
    _description = "";
    _titleName = "";
    _thumbnailImageRead = false;
    _titleImageRead = false;
}

StfsPackage::~StfsPackage( void )
{

}

bool StfsPackage::IsStfsPackage( void )
{
    if (Stream->Length() < 0xA000)
        return false;
    if (checkedIfIsStfsPackage)
        return _isStfsPackage;
    DWORD Magic = this->Magic();
    switch (Magic)
    {
    case STFS_PACKAGE_CON:
    case STFS_PACKAGE_LIVE:
    case STFS_PACKAGE_PIRS:
        return (_isStfsPackage = true);
    default:
        return (_isStfsPackage = false);
    }
}

DWORD StfsPackage::Magic( void )
{
    if (_magic)
        return _magic;
    Stream->SetPosition(0x0);
    return (_magic = Stream->ReadUInt32());
}

DWORD StfsPackage::ContentType( void )
{
    if (_contentType)
        return _contentType;
    Stream->SetPosition(0x344);
    return (_contentType = Stream->ReadUInt32());
}

DWORD StfsPackage::TitleId( void )
{
    if (_titleId)
        return _titleId;
    Stream->SetPosition(0x360);
    return (_titleId = Stream->ReadUInt32());
}

UINT64 StfsPackage::ConsoleId( void )
{
    if (_consoleId)
        return _consoleId;
    Stream->SetPosition(0x369);
    return (_consoleId = (Stream->ReadUInt64() & 0xFFFFFF0000000000));
}

UINT64 StfsPackage::ProfileId( void )
{
    if (_profileId)
        return _profileId;
    Stream->SetPosition(0x371);
    return (_profileId = Stream->ReadUInt64());
}

QString StfsPackage::DisplayName( int Locale )
{
    if (_displayName != "")
        return _displayName;
    Stream->SetPosition(0x411 + (0x80 * Locale));
    BYTE dn[0x80] = {0};
    Stream->Read((BYTE*)&dn, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dn + i, sizeof(short), sizeof(char));
    return (_displayName = QString::fromUtf16((const ushort*)&dn));
}

QString StfsPackage::Description( int Locale )
{
    if (_description != "")
        return _description;
    Stream->SetPosition(0xD11 + (0x80 * Locale));
    BYTE dd[0x80] = {0};
    Stream->Read((BYTE*)&dd, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dd + i, sizeof(short), sizeof(char));
    return (_description = QString::fromUtf16((const ushort*)&dd));
}

QString StfsPackage::TitleName( int Locale )
{
    if (_titleName != "")
        return _titleName;
    Stream->SetPosition(0x1691 + (0x80 * Locale));
    BYTE dd[0x80] = {0};
    Stream->Read((BYTE*)&dd, 0x80);
    for (int i = 0; i < 0x80; i+=2)
        Stream->DetermineAndDoEndianSwap((BYTE*)&dd + i, sizeof(short), sizeof(char));
    return (_titleName = QString::fromUtf16((const ushort*)&dd));
}

QImage StfsPackage::ThumbnailImage( void )
{
    if (_thumbnailImageRead)
        return _thumbnailImage;
    Stream->SetPosition(0x1712);
    DWORD Size = Stream->ReadUInt32();
    Stream->SetPosition(0x171A);
    BYTE Image[0x4000] = {0};
    Stream->Read((BYTE*)&Image, Size);
    return (_thumbnailImage = QImage::fromData(QByteArray::fromRawData((const char*)&Image, Size)));
}

QImage StfsPackage::TitleImage( void )
{
    if (_titleImageRead)
        return _titleImage;
    Stream->SetPosition(0x1716);
    DWORD Size = Stream->ReadUInt32();
    Stream->SetPosition(0x571A);
    BYTE Image[0x4000] = {0};
    Stream->Read((BYTE*)&Image, Size);
    return (_titleImage = QImage::fromData(QByteArray::fromRawData((const char*)&Image, Size)));
}

QString StfsPackage::ContentType_s( void )
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
