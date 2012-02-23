#ifndef __HELPERS__HG
#define __HELPERS__HG
#include "../typedefs.h"
#include "../FATX/StaticInformation.h"
#include <QDateTime>
#include <QDate>
#include <QTime>
using namespace std;
class Helpers
{
public:
    static QDateTime IntToQDateTime( FAT_TIME_STAMP Date );
    static int   QDateTimeToInt( QDateTime Date );
	static INT64 DownToNearestSector( INT64 Offset );
	static INT64 UpToNearestSector( INT64 Offset );
    static INT64 UpToNearestX( INT64 Value, int x);
    static INT64 DownToNearestX( INT64 Value, int x);
	static string ConvertToFriendlySize( INT64 Size );
    static void split(const string &s, char delim, vector<string> &elems);
    static string QStringToStdString(QString stringData);
    static QString QStringFromStdString(string stringData);
};
#endif
