#include "StdAfx.h"
#include "Helpers.h"
#include <QDebug>
#include <sstream>

using std::string;
using std::vector;
using std::stringstream;

QDateTime Helpers::IntToQDateTime( FAT_TIME_STAMP Date )
{
    QDateTime r(QDate(Date.DateTime.Year, Date.DateTime.Month, Date.DateTime.Day),
                QTime(Date.DateTime.Hour, Date.DateTime.Minute, Date.DateTime.Seconds * 2));
    return r;
}

int Helpers::QDateTimeToInt( QDateTime /* date */ )
{
    return 0;
}

INT64 Helpers::DownToNearestSector( INT64 Offset )
{
    return Offset -= (Offset % 0x200);
}

INT64 Helpers::UpToNearestSector( INT64 Offset )
{
    int Add = 0x200 - (Offset % 0x200);
    // If add doesn't equal 0x200, return offset + add
    return (Add != 0x200) ? Offset + Add : Offset;
}

INT64 Helpers::UpToNearestX(INT64 Value, int x)
{
    int Remainder = Value % x;
    int Add = x - Remainder;
    // If add doesn't equal x, return value + add
    return (Remainder != 0) ? Value + Add : Value;
}

INT64 Helpers::DownToNearestX( INT64 Value, int x)
{
    return Value -= (Value % x);
}

string Helpers::ConvertToFriendlySize( INT64 Size )
{
    double size = Size;
    string returnVal;
    char Output[50];
    memset(Output, 0, 50);
    // There's 0x400 bytes in a kilobyte, 0x400 KB in a MB, 0x400 MB in a GB
    // if the size is below 1KB
    if ((size / 0x400) < 1)
    {
        sprintf(Output, "%.2lf", size);
        returnVal.clear();
        returnVal += Output;
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
        sprintf(Output, "%.2lf", size);
        returnVal.clear();
        returnVal += Output;
        returnVal += " KB";
    }

    // If the size is above 1MB
    if (size / 0x400 > 1)
    {
        size = size / 0x400;
        sprintf(Output, "%.2lf", size);
        returnVal.clear();
        returnVal += Output;
        returnVal += " MB";
    }

    // If the size is bigger than 1GB
    if (size / 0x400 > 1)
    {
        size = size / 0x400;
        sprintf(Output, "%.2lf", size);
        returnVal.clear();
        returnVal += Output;;
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
