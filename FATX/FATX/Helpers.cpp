#include "StdAfx.h"
#include "Helpers.h"
#include <QDebug>

QDateTime Helpers::IntToQDateTime( FAT_TIME_STAMP Date )
{
//    int Second, Minute, Hour, Day, Month, Year;
//    Second  = UpToNearestX(Date & 0x1F, 2);         // Bits 0-4
//    Minute  = (Date & 0x7E0) >> 5;                  // Bits 5-11
//    Hour    = (Date & 0xFC00) >> 11;                // Bits 11-15
//    Day     = (Date & 0x1F0000) >> 16;              // Bits 16-20
//    Month   = (Date & 0x1E00000) >> 21;             // Bits 21-24
//    Year    = ((Date & 0xFE000000) >> 25) + 1980;   // Bits 25-31
    QDateTime r(QDate(Date.DateTime.Year, Date.DateTime.Month, Date.DateTime.Day),
                QTime(Date.DateTime.Hour, Date.DateTime.Minute, Date.DateTime.Seconds * 2));
    return r;
}

int Helpers::QDateTimeToInt( QDateTime Date )
{

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
		char* Output = new char[5]; // 5 for 1024 + null
        memset(Output, 0, 5);
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
