#ifndef __HELPERS__HG
#define __HELPERS__HG
#include "../typedefs.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
using namespace std;
class Helpers
{
public:
    static QDateTime IntToQDateTime( int Date );
    static int   QDateTimeToInt( QDateTime Date );
	static INT64 DownToNearestSector( INT64 Offset );
	static INT64 UpToNearestSector( INT64 Offset );
    static INT64 UpToNearestX( INT64 Value, int x);
    static INT64 DownToNearestX( INT64 Value, int x);
	static string ConvertToFriendlySize( INT64 Size );
};
#endif