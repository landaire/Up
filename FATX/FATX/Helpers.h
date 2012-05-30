#ifndef __HELPERS__HG
#define __HELPERS__HG
#include "../typedefs.h"
#include "../FATX/StaticInformation.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include "nowide/convert.h"

class Helpers
{
public:
    static QDateTime IntToQDateTime( FAT_TIME_STAMP Date );
    static int   QDateTimeToInt( QDateTime Date );
	static INT64 DownToNearestSector( INT64 Offset );
	static INT64 UpToNearestSector( INT64 Offset );
    static INT64 UpToNearestX( INT64 Value, int x);
    static INT64 DownToNearestX( INT64 Value, int x);
    static std::string ConvertToFriendlySize( INT64 Size );
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);

    static std::string QStringToStdString(QString stringData);
    static QString QStringFromStdString(std::string stringData);

    static QString QStringFromStdWString(std::wstring stringData);
};
#endif
