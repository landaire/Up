#include "StdAfx.h"
#include "file_stream.h"
#include <QDebug>
#include "../xexception.h"
#include "../nowide/convert.h"

namespace Streams
{
FileStream::FileStream(char *FilePath, int Mode)
{
    Initialize(std::string(FilePath), Mode);
}

FileStream::FileStream(std::string FilePath, int Mode )
{
    Initialize(FilePath, Mode);
}

void FileStream::Initialize(std::string FilePath, int Mode)
{
    using namespace std;
    length = 0;
    endian = Big;
    IsClosed() = false;
    int iosMode = std::ios::binary;
    const char* CharPath = FilePath.c_str();
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


FileStream::~FileStream(void)
{

}

void FileStream::Close( void )
{
    if (!IsClosed())
    {
        _FileStream.close();
        IsClosed() = true;
    }
}

INT64 FileStream::GetPosition( void )
{
    int x = _FileStream.tellg();
    return (IsClosed()) ? 0 : (INT64)_FileStream.tellg();
}

void FileStream::SetPosition( INT64 Position )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::SetPosition");
    }
    if (Position > Length())
        return;
    _FileStream.seekg((DWORD)Position);
    _FileStream.seekp((DWORD)Position);
}

INT64 FileStream::Length( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::GetLength");
    }
    INT64 Position = _FileStream.tellg();
    _FileStream.seekg(0, std::ios::end);
    length = _FileStream.tellg();
    _FileStream.seekg((DWORD)Position, std::ios::beg);
    return length;
}

std::string FileStream::ReadString( size_t Count )
{
    char* buff = new char[Count + 1];
    memset(buff, 0, Count + 1);
    Read((BYTE*)buff, Count);
    std::string Return(buff);

    delete[] buff;
    return Return;
}

std::wstring FileStream::ReadUnicodeString( size_t Count)
{
    char* buff = new char[Count + 1];
    memset(buff, 0, Count + 1);
    Read((BYTE*)buff, Count);
    determineAndDoArraySwap((BYTE*)buff, Count, false);
    std::wstring Return((TCHAR*)buff);

    delete[] buff;
    return Return;
}

std::string FileStream::ReadCString( void )
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

    determineAndDoEndianSwap(tempString, temp.size() - 1, sizeof(char));

    std::string Return(tempString);

    delete[] tempString;

    return Return;
}

BYTE FileStream::ReadByte( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadByte");
    }
    BYTE Return;
    _FileStream.read((char*)&Return, 1);
    return Return;
}

short FileStream::ReadInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt16");
    }
    short Return;
    _FileStream.read((char*)&Return, sizeof(short));

    determineAndDoEndianSwap(&Return, 1, sizeof(short));

    return Return;
}

int FileStream::ReadInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt32");
    }
    int Return;
    _FileStream.read((char*)&Return, sizeof(int));

    determineAndDoEndianSwap(&Return, 1, sizeof(int));

    return Return;
}

INT64 FileStream::ReadInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadInt64");
    }
    INT64 Return;
    _FileStream.read((char*)&Return, sizeof(INT64));

    determineAndDoEndianSwap(&Return, 1, sizeof(INT64));

    return Return;
}

UINT16 FileStream::ReadUInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt16");
    }
    UINT16 Return;
    _FileStream.read((char*)&Return, sizeof(UINT16));

    determineAndDoEndianSwap(&Return, 1, sizeof(UINT16));

    return Return;
}

UINT32 FileStream::ReadUInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt32");
    }
    UINT32 Return;
    _FileStream.read((char*)&Return, sizeof(UINT32));

    determineAndDoEndianSwap(&Return, 1, sizeof(UINT32));

    return Return;
}

UINT64 FileStream::ReadUInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::ReadUInt64");
    }
    UINT64 Return;
    _FileStream.read((char*)&Return, sizeof(UINT64));

    determineAndDoEndianSwap(&Return, 1, sizeof(UINT64));

    return Return;
}

int FileStream::Read( BYTE* DestBuff, int Count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::Read");
    }

    // Calculate the amount of data we can read
    if ((GetPosition() + Count) > Length())
    {
        // We can't read beyond the end of the stream, so let's just read as much as we can
        Count = (DWORD)((GetPosition() >= Length()) ? 0 : Length() - GetPosition());
    }
    _FileStream.read((char*)DestBuff, Count);

    determineAndDoEndianSwap(DestBuff, Count, sizeof(BYTE));

    return Count;
}

void FileStream::WriteByte( BYTE _Byte )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteByte");
    }
    _FileStream.write((char*)&_Byte, sizeof(BYTE));
}

void FileStream::WriteInt16( short _Int16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt16");
    }

    determineAndDoEndianSwap(&_Int16, 1, sizeof(short));

    _FileStream.write((char*)&_Int16, sizeof(INT16));
}

void FileStream::WriteInt32( int _Int32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt32");
    }

    determineAndDoEndianSwap(&_Int32, 1, sizeof(int));

    _FileStream.write((char*)&_Int32, sizeof(INT32));
}

void FileStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteInt64");
    }

    determineAndDoEndianSwap(&_Int64, 1, sizeof(INT64));

    _FileStream.write((char*)&_Int64, sizeof(INT64));
}

void FileStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt16");
    }

    determineAndDoEndianSwap(&_UInt16, 1, sizeof(UINT16));

    _FileStream.write((char*)&_UInt16, sizeof(UINT16));
}

void FileStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt32");
    }

    determineAndDoEndianSwap(&_UInt32, 1, sizeof(UINT32));

    _FileStream.write((char*)&_UInt32, sizeof(UINT32));
}

void FileStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::WriteUInt64");
    }

    determineAndDoEndianSwap(&_UInt64, 1, sizeof(UINT64));

    _FileStream.write((char*)&_UInt64, sizeof(UINT64));
}

int FileStream::Write( BYTE* Buffer, int count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::Write");
    }

    void* temp = determineAndDoEndianSwap(Buffer, count, sizeof(BYTE), true);

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

size_t FileStream::Write( void* Buffer, size_t ElementSize, int count)
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xFileStream::Write (the one with more arguments and stuff)");
    }

    void* temp = determineAndDoEndianSwap((BYTE*)Buffer, count * ElementSize, sizeof(Buffer), true);

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
