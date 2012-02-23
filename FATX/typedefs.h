#ifndef __TYPEDEFS__HG
#define __TYPEDEFS__HG
#ifndef _WINDOWS_
#ifndef _WIN32

typedef unsigned long       DWORD;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef unsigned short      WORD;


typedef          char       INT8, *PINT8;
typedef          short      INT16, *PINT16;
typedef          int        INT32, *PINT32;
typedef          long long  INT64, *PINT64;
typedef unsigned char       UINT8, *PUINT8;
typedef unsigned short      UINT16, *PUINT16;
typedef unsigned int        UINT32, *PUINT32;
typedef unsigned long long  UINT64, *PUINT64;
#else
#include <Windows.h>
#include <tchar.h>
typedef          short      INT16, *PINT16;
typedef unsigned short      UINT16, *PUINT16;
#endif

#ifdef UNICODE

#define _tcslen     wcslen
#define _tcscpy     wcscpy
#define _tcsncpy    wcsncpy
#define _tcscat     wcscat
#define _tcsupr     wcsupr
#define _tcslwr     wcslwr

#define _stprintf   swprintf
#define _tprintf    wprintf

#define _vstprintf      vswprintf

#define _tscanf     wscanf


#define TCHAR wchar_t

#else

#define _tcslen     strlen
#define _tcscpy     strcpy
#define _tcsncpy    strncpy
#define _tcscat     strcat
#define _tcsupr     strupr
#define _tcslwr     strlwr

#define _stprintf   sprintf
#define _tprintf    printf

#define _vstprintf      vsprintf

#define _tscanf     scanf

#define TCHAR char
#endif
#endif
#endif
