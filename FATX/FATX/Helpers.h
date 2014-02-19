#ifndef __HELPERS__HG
#define __HELPERS__HG
#include "../typedefs.h"
#include "StaticInformation.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include "../nowide/convert.h"

class Helpers
{
public:
    static QDateTime IntToQDateTime( FAT_TIME_STAMP Date );
    static int   QDateTimeToInt(QDateTime);
	static uint64_t4_t DownToNearestSectouint64_tnt64_t Offset );
	static uint64_t4_t UpToNearestSectouint64_tnt64_t Offset );
    static uint64_t4_t UpToNearestuint64_tnt64_t Value, int x);
    static uint64_t4_t DownToNearestuint64_tnt64_t Value, int x);
    static std::string ConvertToFriendlySize( uint64_t4_t Size );
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);

    static std::string QStringToStdString(QString stringData);
    static QString QStringFromStdString(std::string stringData);

    static QString QStringFromStdWString(std::wstring stringData);
};
#endif
