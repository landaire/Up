#ifndef STFSPACKAGE_H
#define STFSPACKAGE_H
#include "../typedefs.h"
#include "../FATX/IO/IStream.h"
#include "StaticInformation.h"
#include <QString>
#include <../QtGui/QImage>
using namespace std;
using namespace Streams;

class STFSPackage
{
private:
    Streams::IStream *Stream;
public:
    STFSPackage( Streams::IStream *Stream );
    ~STFSPackage( void );
    bool IsStfsPackage( void );
    DWORD Magic( void );
    DWORD ContentType( void );
    DWORD TitleId( void );
    UINT64 ConsoleId( void );
    UINT64 ProfileId( void );
    QString DisplayName( int Locale = 0 );
    QString Description( int Locale = 0 );
    QString TitleName( int Locale = 0 );
    QImage ThumbnailImage( void );
    QImage TitleImage( void );

    QString ContentType_s( void );
};

#endif // STFSPACKAGE_H
