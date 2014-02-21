#ifndef __HELPERS__HG
#define __HELPERS__HG
#include "../typedefs.h"
#include "static_information.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
#include "../nowide/convert.h"

class Helpers
{
public:
    static QDateTime IntToQDateTime( TimeStamp Date );
    static int   QDateTimeToInt(QDateTime);
    static uint64_t4_t DownToNearestSector(uint64_t offset);
    static uint64_t4_t UpToNearestSector(uint64_t offset );
    static uint64_t4_t UpToNearestX(uint64_t value, int x);
    static uint64_t4_t DownToNearestX(uint64_t value, int x);
    static std::string ConvertToFriendlySize(uint64_t Size );
    static void split(const std::string &s, char delim, std::vector<std::string> &elems);

    static std::string QStringToStdString(QString stringData);
    static QString QStringFromStdString(std::string stringData);

    static QString QStringFromStdWString(std::wstring stringData);
};
#endif
