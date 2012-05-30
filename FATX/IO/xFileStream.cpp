#include "StdAfx.h"
#include "xFileStream.h"
#include <QDebug>
#include "../xexception.h"

namespace Streams
{
xFileStream::xFileStream(char *FilePath, int Mode)
{
    Initialize((void*)FilePath, Mode, false);
}

xFileStream::xFileStream(wchar_t *FilePath, int Mode)
{
    Initialize((void*)FilePath, Mode, true);
}


xFileStream::xFileStream(std::string FilePath, int Mode )
{
    Initialize((void*)FilePath.c_str(), Mode, false);
}

xFileStream::xFileStream(std::wstring FilePath, int Mode)
{
    Initialize((void*)FilePath.c_str(), Mode, true);
}

void xFileStream::Initialize(void *FilePath, int Mode, bool wchar)
{
    using namespace std;
    _Length = 0;
    _Endian = Big;
    IsClosed = false;
    int iosMode = std::ios::binary;
#ifndef _WIN32
    char CharPath[0x200] = {0};
    if (wchar)
    {
        wcstombs(CharPath, (wchar_t*)FilePath, wcslen((wchar_t*)FilePath));
    }
    else
    {
        strcpy(CharPath, (char*)FilePath);
    }
#endif
#ifdef _WIN32
    TCHAR *CharPath = (TCHAR*)FilePath;
#endif
    switch (Mode)
    {
    // Specifies that a new file should be created.  If it exists, it is overwritten
    case Create:
    {
        iosMode |= std::ios::trunc | std::ios::in | std::ios::out;
    }
        break;
        // Specifies that a new file should be created.  If it already exists, an exception is thrown
    case CreateNew:
    {
        ifstream temp(CharPath);
        if (temp.is_open())
        {
            temp.close();
            throw xException("Cannot create new file over existing", CreateNew);
        }
        iosMode |= std::ios::in | std::ios::out | std::ios::trunc;
    }
        break;
        // Specifies that a new file should be created.  If it exists, it is overwritten
    case Open:
    {
        std::ifstream temp(CharPath);
        if (!temp.good() || temp.fail())
        {
            throw xException("Can not open file for reading.  File does not exist", Open);
        }
        try
        {
            temp.close();
        }
        catch (...)
        {
            throw xException("Can not open file for reading.  File does not exist", Open);
        }

        iosMode |= ios::in | ios::out;
    }
        break;
        // Specifies that the file should be opened if it exsts; otherwise a new file is created
    case OpenOrCreate:
    {
        iosMode |= (ios::in | ios::out);
    }
        break;
    }
    _FileStream.open(CharPath, (std::ios::openmode)iosMode);
    try
    {
        if (!_FileStream.good())
        {
            throw xException("Can not open file.");
        }
    }
    catch(...)
    {
        throw xException("Error trying to check filestream - xFileStream::Initialize");
    }
}


xFileStream::~xFileStream(void)
{

}

void xFileStream::Close( void )
{
    if (!IsClosed)
    {
        _FileStream.close();
        IsClosed = true;
    }
}

INT64 xFileStream::Position( void )
{
    int x = _FileStream.tellg();
    return (IsClosed) ? 0 : (INT64)_FileStream.tellg();
}

void xFileStream::SetPosition( INT64 Position )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::SetPosition");
    }
    if (Position > Length())
        return;
    _FileStream.seekg((DWORD)Position);
    _FileStream.seekp((DWORD)Position);
}

INT64 xFileStream::Length( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::GetLength");
    }
    INT64 Position = _FileStream.tellg();
    _FileStream.seekg(0, std::ios::end);
    _Length = _FileStream.tellg();
    _FileStream.seekg((DWORD)Position, std::ios::beg);
    return _Length;
}

std::string xFileStream::ReadString( size_t Count )
{
    char* buff = new char[Count + 1];
    memset(buff, 0, Count + 1);
    Read((BYTE*)buff, Count);
    std::string Return(buff);

    delete[] buff;
    return Return;
}

std::wstring xFileStream::ReadUnicodeString( size_t Count)
{
    char* buff = new char[Count + 1];
    memset(buff, 0, Count + 1);
    Read((BYTE*)buff, Count);
    DetermineAndDoArraySwap((BYTE*)buff, Count, false);
    std::wstring Return((TCHAR*)buff);

    delete[] buff;
    return Return;
}

std::string xFileStream::ReadCString( void )
{
    std::vector<char> temp;
    bool Null;
    do
    {
        char c = (char)ReadByte();
        temp.push_back(c);
        if (c == 0)
        {
            Null = true;
        }
    }
    while (!Null);

    char* tempString = new char[temp.size()];
    for (int i = 0; i < (int)temp.size(); i++)
    {
        tempString[i] = temp.at(i);
    }

    DetermineAndDoEndianSwap(tempString, temp.size() - 1, sizeof(char));

    std::string Return(tempString);

    delete[] tempString;

    return Return;
}

BYTE xFileStream::ReadByte( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadByte");
    }
    BYTE Return;
    _FileStream.read((char*)&Return, 1);
    return Return;
}

short xFileStream::ReadInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt16");
    }
    short Return;
    _FileStream.read((char*)&Return, sizeof(short));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(short));

    return Return;
}

int xFileStream::ReadInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt32");
    }
    int Return;
    _FileStream.read((char*)&Return, sizeof(int));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(int));

    return Return;
}

INT64 xFileStream::ReadInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt64");
    }
    INT64 Return;
    _FileStream.read((char*)&Return, sizeof(INT64));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(INT64));

    return Return;
}

UINT16 xFileStream::ReadUInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt16");
    }
    UINT16 Return;
    _FileStream.read((char*)&Return, sizeof(UINT16));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(UINT16));

    return Return;
}

UINT32 xFileStream::ReadUInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt32");
    }
    UINT32 Return;
    _FileStream.read((char*)&Return, sizeof(UINT32));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(UINT32));

    return Return;
}

UINT64 xFileStream::ReadUInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt64");
    }
    UINT64 Return;
    _FileStream.read((char*)&Return, sizeof(UINT64));

    DetermineAndDoEndianSwap(&Return, 1, sizeof(UINT64));

    return Return;
}

int xFileStream::Read( BYTE* DestBuff, int Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::Read");
    }

    // Calculate the amount of data we can read
    if ((Position() + Count) > Length())
    {
        // We can't read beyond the end of the stream, so let's just read as much as we can
        Count = (DWORD)((Position() >= Length()) ? 0 : Length() - Position());
    }
    _FileStream.read((char*)DestBuff, Count);

    DetermineAndDoEndianSwap(DestBuff, Count, sizeof(BYTE));

    return Count;
}

void xFileStream::WriteByte( BYTE _Byte )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteByte");
    }
    _FileStream.write((char*)&_Byte, sizeof(BYTE));
}

void xFileStream::WriteInt16( short _Int16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt16");
    }

    DetermineAndDoEndianSwap(&_Int16, 1, sizeof(short));

    _FileStream.write((char*)&_Int16, sizeof(INT16));
}

void xFileStream::WriteInt32( int _Int32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt32");
    }

    DetermineAndDoEndianSwap(&_Int32, 1, sizeof(int));

    _FileStream.write((char*)&_Int32, sizeof(INT32));
}

void xFileStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt64");
    }

    DetermineAndDoEndianSwap(&_Int64, 1, sizeof(INT64));

    _FileStream.write((char*)&_Int64, sizeof(INT64));
}

void xFileStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt16");
    }

    DetermineAndDoEndianSwap(&_UInt16, 1, sizeof(UINT16));

    _FileStream.write((char*)&_UInt16, sizeof(UINT16));
}

void xFileStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt32");
    }

    DetermineAndDoEndianSwap(&_UInt32, 1, sizeof(UINT32));

    _FileStream.write((char*)&_UInt32, sizeof(UINT32));
}

void xFileStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt64");
    }

    DetermineAndDoEndianSwap(&_UInt64, 1, sizeof(UINT64));

    _FileStream.write((char*)&_UInt64, sizeof(UINT64));
}

int xFileStream::Write( BYTE* Buffer, int count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::Write");
    }

    void* temp = DetermineAndDoEndianSwap(Buffer, count, sizeof(BYTE), true);

    if (!temp)
    {
        _FileStream.write((char*)(Buffer), count);
        return count;
    }
    else
    {
        _FileStream.write((char*)temp, count);

        delete[] temp;
        return count;
    }
}

size_t xFileStream::Write( void* Buffer, size_t ElementSize, int count)
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xFileStream::Write (the one with more arguments and stuff)");
    }

    void* temp = DetermineAndDoEndianSwap((BYTE*)Buffer, count * ElementSize, sizeof(Buffer), true);

    if (!temp)
    {
        _FileStream.write((char*)Buffer, count * ElementSize);
        return count;
    }
    else
    {
        _FileStream.write((char*)temp, count * ElementSize);

        delete[] temp;
        return count;
    }
}
}
