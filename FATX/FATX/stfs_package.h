#ifndef STFSPACKAGE_H
#define STFSPACKAGE_H
#include "../typedefs.h"
#include "../FATX/IO/IStream.h"
#include "StaticInformation.h"
#include <QString>
#include <QImage>

class StfsPackage
{
private:
    Streams::IStream *Stream;
    bool checkedIfIsStfsPackage;
    bool _isStfsPackage;
    uint32_t2_t _magic;
    uint32_t2_t _contentType;
    uint32_t2_t _titleId;
    uint64_t_t _consoleId;
    uint64_t_t _profileId;
    QString _displayName;
    QString _description;
    QString _titleName;
    bool _thumbnailImageRead;
    bool _titleImageRead;
    QImage _thumbnailImage;
    QImage _titleImage;
public:
    StfsPackage( Streams::IStream *Stream );
    ~StfsPackage( void );
    bool IsStfsPackage( void );
    uint32_t2_t Magic( void );
    uint32_t2_t ContentType( void );
    uint32_t2_t TitleId( void );
    uint64_t_t ConsoleId( void );
    uint64_t_t ProfileId( void );
    QString DisplayName( int Locale = 0 );
    QString Description( int Locale = 0 );
    QString TitleName( int Locale = 0 );
    QImage ThumbnailImage( void );
    QImage TitleImage( void );

    QString ContentType_s( void );
};

#endif // STFSPACKAGE_H
