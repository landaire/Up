#include "StdAfx.h"
#include "multi_file_stream.h"
#include <QDebug>
namespace Streams
{
MultiFileStream::MultiFileStream( const std::vector<std::string> InPaths )
{
    length = 0;
    endian = Big;
    IsClosed() = false;
    UserOffset = 0;
    CurrentStream = 0;

    for (int i = 0; i < (int)InPaths.size(); i++)
    {
        xFileStream *temp = new xFileStream(InPaths.at(i), Open);
        FileStreams.push_back(temp);
    }
}


MultiFileStream::~MultiFileStream(void)
{
}

void MultiFileStream::Close( void )
{
    if (IsClosed())
        return;
    while(FileStreams.size())
    {
        if (!FileStreams.size())
            break;
        FileStreams[0]->Close();

        delete FileStreams[0];
        FileStreams.erase(FileStreams.begin());
    }
    IsClosed() = true;
}

INT64 MultiFileStream::GetPosition( void )
{
    return UserOffset;
}

void MultiFileStream::SetPosition( INT64 Position )
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

INT64 MultiFileStream::Length( void )
{
    if (!length && !IsClosed())
    {
        for (int i = 0; i < (int)FileStreams.size(); i++)
        {
            length += FileStreams.at(i)->Length();
        }
    }
    return length;
}

BYTE MultiFileStream::ReadByte( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadByte", ExStreamReadByte);
    }
    else if (GetPosition() > Length() - sizeof(BYTE))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadByte", ExStreamReadByte);
    }

    BYTE Return;
    Read(&Return, 1);

    return Return;
}

short MultiFileStream::ReadInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt16", ExStreamReadInt16);
    }
    else if (GetPosition() > Length() - sizeof(INT16))
    {
        throw xException("End of file reached.  At: xMultiFileStream::ReadInt16", ExStreamReadInt16);
    }

    BYTE temp[2];
    Read((BYTE*)&temp, 2);

    short Return = (short)temp[0] << 8 | temp[1];

    return Return;
}

int MultiFileStream::ReadInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt32");
    }
    else if (GetPosition() > Length() - sizeof(INT32))
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

INT64 MultiFileStream::ReadInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadInt64");
    }
    else if (GetPosition() > Length() - sizeof(INT64))
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

UINT16 MultiFileStream::ReadUInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt16");
    }
    else if (GetPosition() > Length() - sizeof(UINT16))
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

UINT32 MultiFileStream::ReadUInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt32");
    }
    else if (GetPosition() > Length() - sizeof(UINT32))
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

UINT64 MultiFileStream::ReadUInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUInt64");
    }
    else if (GetPosition() > Length() - sizeof(UINT64))
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

int MultiFileStream::Read( BYTE* DestBuff, int Count )
{
    int Offset = 0;
    if (IsClosed())
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
            SetPosition(GetPosition() + Read);
        }
        determineAndDoArraySwap(DestBuff, Count, true);
        return Count;
    }
    else
    {
        // We can. Yay.
        DWORD Read = FileStreams.at(CurrentStream)->Read(DestBuff, Count);
        SetPosition(GetPosition() + Read);
        determineAndDoArraySwap(DestBuff, Count, true);
        return Read;
    }
}

std::string MultiFileStream::ReadString( size_t Count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadString");
    }
    if (GetPosition() + Count > Length())
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

std::string MultiFileStream::ReadCString( void )
{
    if (IsClosed())
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

    determineAndDoEndianSwap(tempString, temp.size() - 1, sizeof(char));

    std::string Return(tempString);

    delete[] tempString;
    return Return;
}

std::wstring MultiFileStream::ReadUnicodeString( size_t Count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::ReadUnicodeString");
    }
    else if (GetPosition() + Count > Length())
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

void MultiFileStream::WriteByte( BYTE _Byte )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteByte");
    }
    else if (GetPosition() + sizeof(BYTE) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteByte");
    }
    Write(&_Byte, 1);
}

void MultiFileStream::WriteInt16( short _Int16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt16");
    }
    else if (GetPosition() + sizeof(short) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt16");
    }
    determineAndDoEndianSwap((BYTE*)&_Int16, sizeof(short), sizeof(BYTE));
    Write((BYTE*)&_Int16, sizeof(short));
}

void MultiFileStream::WriteInt32( int _Int32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt32");
    }
    else if (GetPosition() + sizeof(int) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt16");
    }

    determineAndDoEndianSwap((BYTE*)&_Int32, sizeof(int), sizeof(BYTE));
    Write((BYTE*)&_Int32, sizeof(int));
}

void MultiFileStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteInt64");
    }
    else if (GetPosition() + sizeof(INT64) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteInt64");
    }

    determineAndDoEndianSwap((BYTE*)&_Int64, sizeof(INT64), sizeof(BYTE));
    Write((BYTE*)&_Int64, sizeof(INT64));
}

void MultiFileStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt16");
    }
    else if (GetPosition() + sizeof(UINT16) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt16");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt16, sizeof(UINT16), sizeof(BYTE));
    Write((BYTE*)&_UInt16, sizeof(UINT16));
}

void MultiFileStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt32");
    }
    else if (GetPosition() + sizeof(UINT32) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt32");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt32, sizeof(UINT32), sizeof(BYTE));
    Write((BYTE*)&_UInt32, sizeof(UINT32));
}

void MultiFileStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteUInt64");
    }
    else if (GetPosition() + sizeof(UINT64) > Length())
    {
        throw xException("End of file reached.  At: xMultiFileStream::WriteUInt64");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt64, sizeof(UINT64), sizeof(BYTE));
    Write((BYTE*)&_UInt64, sizeof(UINT64));
}

int MultiFileStream::Write( BYTE* Buffer, int count )
{
    int offset;
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xMultiFileStream::WriteBytes");
    }
    if (GetPosition() + count > Length())
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
            SetPosition(GetPosition() + Written);
        }
        return count;
    }
    else
    {
        // We can. Yay.
        DWORD Written = FileStreams.at(CurrentStream)->Write(Buffer, offset, count);
        SetPosition(GetPosition() + Written);
        return Written;
    }
}
}
