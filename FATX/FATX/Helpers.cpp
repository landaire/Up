#include "StdAfx.h"
#include "Helpers.h"
#include <QDebug>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

QDateTime Helpers::IntToQDateTime( FAT_TIME_STAMP date )
{
    QDateTime r(QDate(date.DateTime.Year, date.DateTime.Month, date.DateTime.Day),
                QTime(date.DateTime.Hour, date.DateTime.Minute, date.DateTime.Seconds * 2));
    return r;
}

int Helpers::QDateTimeToInt( QDateTime /* date */ )
{
    return 0;
}

uint64_t Helpers::DownToNearestSector( uint64_t offset )
{
    return offset -= (offset % 0x200);
}

uint64_t Helpers::UpToNearestSector( uint64_t offset )
{
    int Add = 0x200 - (offset % 0x200);
    // If add doesn't equal 0x200, return offset + add
    return (Add != 0x200) ? offset + Add : offset;
}

uint64_t Helpers::UpToNearestX(uint64_t value, int x)
{
    int remainder = value % x;
    int add = x - remainder;
    // If add doesn't equal x, return value + add
    return (remainder != 0) ? value + add : value;
}

uint64_t Helpers::DownToNearestX( uint64_t value, int x)
{
    return value -= (value % x);
}

string Helpers::ConvertToFriendlySize( uint64_t size )
{
    double size = size;
    string returnVal;
    char output[50];
    memset(output, 0, 50);
    // There's 0x400 bytes in a kilobyte, 0x400 KB in a MB, 0x400 MB in a GB
    // if the size is below 1KB
    if ((size / 0x400) < 1)
    {
        sprintf(output, "%.2lf", size);
        returnVal.clear();
        returnVal += output;
        if (size != 1)
        {
            returnVal += " bytes";
        }
        else
        {
            returnVal += " byte";
        }
    }
    // If the size is above 1KB
    if (size / 0x400 > 1)
    {
        size = size / 0x400;
        sprintf(output, "%.2lf", size);
        returnVal.clear();
        returnVal += output;
        returnVal += " KB";
    }

    // If the size is above 1MB
    if (size / 0x400 > 1)
    {
        size = size / 0x400;
        sprintf(output, "%.2lf", size);
        returnVal.clear();
        returnVal += output;
        returnVal += " MB";
    }

    // If the size is bigger than 1GB
    if (size / 0x400 > 1)
    {
        size = size / 0x400;
        sprintf(output, "%.2lf", size);
        returnVal.clear();
        returnVal += output;;
        returnVal += " GB";
    }

    return returnVal;
}
void Helpers::split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

string Helpers::QStringToStdString(QString stringData)
{
#ifdef _WIN32
    return string(stringData.toLocal8Bit().constData());
#else
    return string(stringData.toUtf8().constData());
#endif
}

QString Helpers::QStringFromStdString(string stringData)
{
#ifdef _WIN32
    return QString::fromLocal8Bit(stringData.c_str());
#else
    return QString::fromUtf8(stringData.c_str());
#endif
}

QString Helpers::QStringFromStdWString(std::wstring stringData)
{
    return QString::fromUtf16((const ushort*)stringData.c_str());
}
