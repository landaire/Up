#include "StdAfx.h"
#include "xMultiFileStream.h"
#include <QDebug>
namespace Streams
{
xMultiFileStream::xMultiFileStream( std::vector<std::wstring> InPaths )
{
    _Length = 0;
    _Endian = Big;
    IsClosed = false;
    UserOffset = 0;
    CurrentStream = 0;

    for (int i = 0; i < (int)InPaths.size(); i++)
    {
        xFileStream *temp = new xFileStream(InPaths.at(i), Open);
        FileStreams.push_back(temp);
    }
}


xMultiFileStream::~xMultiFileStream(void)
{
}

void xMultiFileStream::Close( void )
{
    if (IsClosed)
        return;
    while(FileStreams.size())
    {
        if (!FileStreams.size())
            break;
        FileStreams[0]->Close();

        delete FileStreams[0];
        FileStreams.erase(FileStreams.begin());
    }
    IsClosed = true;
}

INT64 xMultiFileStream::Position( void )
{
    return UserOffset;
}

void xMultiFileStream::SetPosition( INT64 Position )
{
    // If the position is too large, throw an exception
    if (Position > Length())
    {
        throw xException("Can not seek beyond end of stream", ExStreamSetPosition );
    }
    UserOffset = Position;

    // Find out which stream we can use
    for (int i = 0; i < (int)FileStreams.size(); i++)
    {
        if (Position >= FileStreams.at(i)->Length() && i != FileStreams.size() - 1)
        {
            Position -= FileStreams.at(i)->Length();
        }
        else
        {
            CurrentStream = i;
            FileStreams.at(i)->SetPosition(Position);
            break;
        }
    }
}

INT64 xMultiFileStream::Length( void )
{
    if (!_Length && !IsClosed)
    {
        for (int i = 0; i < (int)FileStreams.size(); i++)
        {
            _Length += FileStreams.at(i)->Length();
        }
    }
    return _Length;
}

BYTE xMultiFileStream::ReadByte( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadByte", ExStreamReadByte);
    }
    else if (Position() > Length() - sizeof(BYTE))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadByte", ExStreamReadByte);
    }

    BYTE Return;
    Read(&Return, 1);

    return Return;
}

short xMultiFileStream::ReadInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt16", ExStreamReadInt16);
    }
    else if (Position() > Length() - sizeof(INT16))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadInt16", ExStreamReadInt16);
    }

    BYTE temp[2];
    Read((BYTE*)&temp, 2);

    short Return = (short)temp[0] << 8 | temp[1];

    return Return;
}

int xMultiFileStream::ReadInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt32");
    }
    else if (Position() > Length() - sizeof(INT32))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadInt32");
    }

    // Get the size of the object
    int size = sizeof(int);

    // Create a new temporary buffer that will hold the bytes for the object
    BYTE temp[4];
    // Read the data
    Read((BYTE*)&temp, size);

    // Initialize our return value
    int Return = 0;

    /* Do a loop that starts out at size * 8 (for an Int32 that would be 24),
         * shift the data at index i by that value, OR that value.  Repeat after addition, subtraction and shit */
    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (int)temp[i] << j;
    }

    return Return;
}

INT64 xMultiFileStream::ReadInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt64");
    }
    else if (Position() > Length() - sizeof(INT64))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadInt64");
    }

    int size = sizeof(INT64);

    BYTE temp[8];
    Read((BYTE*)&temp, size);

    INT64 Return = 0;

    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (INT64)temp[i] << j;
    }

    return Return;
}

UINT16 xMultiFileStream::ReadUInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt16");
    }
    else if (Position() > Length() - sizeof(UINT16))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadUInt16");
    }

    int size = sizeof(UINT16);

    BYTE temp[2];
    Read((BYTE*)&temp, size);

    UINT16 Return = 0;

    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (UINT16)temp[i] << j;
    }

    return Return;
}

UINT32 xMultiFileStream::ReadUInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt32");
    }
    else if (Position() > Length() - sizeof(UINT32))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadUInt32");
    }

    int size = sizeof(UINT32);

    BYTE temp[4];

    Read((BYTE*)&temp, size);

    UINT32 Return = 0;

    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (UINT32)temp[i] << j;
    }

    return Return;
}

UINT64 xMultiFileStream::ReadUInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt64");
    }
    else if (Position() > Length() - sizeof(UINT64))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadUInt64");
    }

    int size = sizeof(UINT64);

    BYTE temp[8];
    Read((BYTE*)&temp, size);

    UINT64 Return = 0;

    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (UINT64)temp[i] << j;
    }

    return Return;
}

int xMultiFileStream::Read( BYTE* DestBuff, int Count )
{
    int Offset = 0;
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::Read");
    }

    //SetPosition(Position());

    // Check to see if we can read straight from the current stream
    if (FileStreams.at(CurrentStream)->Position() + Count > FileStreams.at(CurrentStream)->Length())
    {
        // Balls, we can't. For each stream, loop for how much data we can read until we've read all of it
        while (Count > 0)
        {
            DWORD Read = FileStreams.at(CurrentStream)->Read(DestBuff + Offset, Count);
            Offset += Read;
            Count -= Read;
            SetPosition(Position() + Read);
        }
        DetermineAndDoArraySwap(DestBuff, Count, true);
        return Count;
    }
    else
    {
        // We can. Yay.
        DWORD Read = FileStreams.at(CurrentStream)->Read(DestBuff, Count);
        SetPosition(Position() + Read);
        DetermineAndDoArraySwap(DestBuff, Count, true);
        return Read;
    }
}

std::string xMultiFileStream::ReadString( size_t Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadString");
    }
    if (Position() + Count > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xMultiFileStream::ReadString");
    }
    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count);

    std::string ret((char*)Buffer);

    delete[] Buffer;
    return ret;
}

std::string xMultiFileStream::ReadCString( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadCString");
    }
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

std::wstring xMultiFileStream::ReadUnicodeString( size_t Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUnicodeString");
    }
    else if (Position() + Count > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xMultiFileStream::ReadUnicodeString");
    }

    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count + 1);

    std::wstring ret((TCHAR*)Buffer);

    delete[] Buffer;
    return ret;
}

void xMultiFileStream::WriteByte( BYTE _Byte )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteByte");
    }
    else if (Position() + sizeof(BYTE) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteByte");
    }
    Write(&_Byte, 1);
}

void xMultiFileStream::WriteInt16( short _Int16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt16");
    }
    else if (Position() + sizeof(short) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt16");
    }
    DetermineAndDoEndianSwap((BYTE*)&_Int16, sizeof(short), sizeof(BYTE));
    Write((BYTE*)&_Int16, sizeof(short));
}

void xMultiFileStream::WriteInt32( int _Int32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt32");
    }
    else if (Position() + sizeof(int) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt16");
    }

    DetermineAndDoEndianSwap((BYTE*)&_Int32, sizeof(int), sizeof(BYTE));
    Write((BYTE*)&_Int32, sizeof(int));
}

void xMultiFileStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt64");
    }
    else if (Position() + sizeof(INT64) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt64");
    }

    DetermineAndDoEndianSwap((BYTE*)&_Int64, sizeof(INT64), sizeof(BYTE));
    Write((BYTE*)&_Int64, sizeof(INT64));
}

void xMultiFileStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt16");
    }
    else if (Position() + sizeof(UINT16) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt16");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt16, sizeof(UINT16), sizeof(BYTE));
    Write((BYTE*)&_UInt16, sizeof(UINT16));
}

void xMultiFileStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt32");
    }
    else if (Position() + sizeof(UINT32) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt32");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt32, sizeof(UINT32), sizeof(BYTE));
    Write((BYTE*)&_UInt32, sizeof(UINT32));
}

void xMultiFileStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt64");
    }
    else if (Position() + sizeof(UINT64) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt64");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt64, sizeof(UINT64), sizeof(BYTE));
    Write((BYTE*)&_UInt64, sizeof(UINT64));
}

int xMultiFileStream::Write( BYTE* Buffer, int count )
{
    int offset;
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteBytes");
    }
    if (Position() + count > Length())
    {
        throw xException("Can not write beyond end of stream");
    }

    //SetPosition(Position());
    // Check to see if we can write straight to the current stream
    if (FileStreams.at(CurrentStream)->Position() + count > FileStreams.at(CurrentStream)->Length())
    {
        // Balls, we can't. For each stream, loop for how much data we can write until we've written all of it
        while (count > 0)
        {
            // The amount of data that can be written for the current stream
            DWORD AmountWeCanWrite = (DWORD)(FileStreams.at(CurrentStream)->Length() - FileStreams.at(CurrentStream)->Position());
            if (AmountWeCanWrite > (DWORD)count)
            {
                AmountWeCanWrite = count;
            }
            DWORD Written = FileStreams.at(CurrentStream)->Write(Buffer, offset, AmountWeCanWrite);
            offset += Written;
            count -= Written;
            SetPosition(Position() + Written);
        }
        return count;
    }
    else
    {
        // We can. Yay.
        DWORD Written = FileStreams.at(CurrentStream)->Write(Buffer, offset, count);
        SetPosition(Position() + Written);
        return Written;
    }
}
}
