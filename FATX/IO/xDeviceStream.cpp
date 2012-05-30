#include "StdAfx.h"
#include "xDeviceStream.h"

namespace Streams
{
xDeviceStream::xDeviceStream( TCHAR* DevicePath )
{
    // Set our variables
    _Length = 0;
    _Endian = Big;
    IsClosed = false;
    UserOffset = 0;
    LastReadOffset = -1;
    IsClosed = false;
#ifdef _WIN32
    memset(&Offset, 0, sizeof(OVERLAPPED));
#else
    memset(&LastReadData, 0, 0x200);
    Offset = 0;
#endif

#ifdef _WIN32
    // Attempt to get a handle to the device
    DeviceHandle = CreateFile(
                DevicePath,							// File name (device path)
                GENERIC_READ | GENERIC_WRITE,		// Read/write access
                FILE_SHARE_READ | FILE_SHARE_WRITE,	// Read/write share
                NULL,								// Not used
                OPEN_EXISTING,						// Open the existing device -- fails if it's fucked
                FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH | FILE_ATTRIBUTE_DEVICE,	// Flags and attributes
                NULL);								// Ignored

    if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        throw xException("Could not open HANDLE for device");
    }
#endif
#if (defined __APPLE__ || defined __linux)
    // UNIX/LINUX/APPLE
    char Path[0x200] = {0};

    // Convert wchar_t string to standard ASCII string
    wcstombs(Path, DevicePath, wcslen(DevicePath));
    // Open the device
    Device = open(Path, O_RDWR);
    if (Device == -1)
    {
        throw xException("Error opening device");
    }
#endif

}


xDeviceStream::~xDeviceStream(void)
{

}

void xDeviceStream::Close( void )
{
    if (IsClosed)
        return;
#if defined _WIN32
    if (DeviceHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(DeviceHandle);
    }
#else
    close(Device);
    Device = 0;
#endif
    IsClosed = true;
}

INT64 xDeviceStream::Position( void )
{
    return UserOffset;
}

INT64 xDeviceStream::RealPosition( void )
{
#ifdef _WIN32
    return (INT64)(((INT64)Offset.OffsetHigh << 32) | Offset.Offset);
#else
    return Offset;
#endif
}

void xDeviceStream::SetPosition( INT64 Position )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::SetPosition");
    }
    UserOffset = Position;
    Position = Helpers::DownToNearestSector(Position); // Round the position down to the nearest sector offset
#ifdef _WIN32
    Offset.Offset = (DWORD)Position;
    Offset.OffsetHigh = (DWORD)(Position >> 32);
#else
    Offset = Position;
    lseek(Device, Position, SEEK_SET);
#endif
}

INT64 xDeviceStream::Length( void )
{
    if (!IsClosed && !_Length)
    {
#ifdef _WIN32
        DISK_GEOMETRY Geometry;
        DWORD BytesReturned;
        memset(&Geometry, 0, sizeof(DISK_GEOMETRY));
        DeviceIoControl(
                    DeviceHandle,					// Device to get the geometry from
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,	// Action we're taking (getting the geometry)
                    NULL,							// Not used since this requires no input data
                    0,								// No data
                    &Geometry,						// Output struct
                    sizeof(DISK_GEOMETRY),			// Output buffer size
                    &BytesReturned,					// Number of bytes returned
                    NULL);							// Not used

        INT64 Cylinders = (INT64)((INT64)Geometry.Cylinders.HighPart << 32) | Geometry.Cylinders.LowPart; // Convert the BIG_INTEGER to INT64

        _Length =
                Cylinders							*
                (INT64)Geometry.TracksPerCylinder	*
                (INT64)Geometry.SectorsPerTrack		*
                (INT64)Geometry.BytesPerSector;
#else
        unsigned int NumberOfSectors = 0;
        // Queue number of sectors
        ioctl(Device, DKIOCGETBLOCKCOUNT, &NumberOfSectors);

        unsigned int SectorSize = 0;
        ioctl(Device, DKIOCGETBLOCKSIZE, &SectorSize);

        _Length = (UINT64)NumberOfSectors * (UINT64)SectorSize;
#endif

    }
    return _Length;
}

BYTE xDeviceStream::ReadByte( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadByte");
    }
    else if (Position() > Length() - sizeof(BYTE))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadByte");
    }

    BYTE Return;
    Read(&Return, 1);

    return Return;
}

short xDeviceStream::ReadInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadInt16");
    }
    else if (Position() > Length() - sizeof(INT16))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadInt16");
    }

    BYTE temp[2];
    Read((BYTE*)&temp, 2);

    short Return = (short)temp[0] << 8 | temp[1];

    return Return;
}

int xDeviceStream::ReadInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadInt32");
    }
    else if (Position() > Length() - sizeof(INT32))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadInt32");
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

INT64 xDeviceStream::ReadInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadInt64");
    }
    else if (Position() > Length() - sizeof(INT64))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadInt64");
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

UINT16 xDeviceStream::ReadUInt16( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadUInt16");
    }
    else if (Position() > Length() - sizeof(UINT16))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadUInt16");
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

UINT32 xDeviceStream::ReadUInt32( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadUInt32");
    }
    else if (Position() > Length() - sizeof(UINT32))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadUInt32");
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

UINT64 xDeviceStream::ReadUInt64( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadUInt64");
    }
    else if (Position() > Length() - sizeof(UINT64))
    {
        throw xException("End of file reached.  At: xDeviceStream::ReadUInt64");
    }

    int size = sizeof(UINT64);

    BYTE temp[8];
    Read(temp, size);

    UINT64 Return = 0;

    for (int i = 0, j = (size * 8) - 8; i < size; i++, j -= 8)
    {
        Return |= (UINT64)temp[i] << j;
    }

    return Return;
}

int xDeviceStream::Read( BYTE* DestBuff, int Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::Read");
    }
#ifdef _WIN32
    else if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        throw xException("Error: INVALID_HANDLE_VALUE. At: xDeviceStream::Read");
    }
#endif
    //SetPosition(Position());
    // We can't read beyond the end of the stream
    if (Position() + Count > Length())
    {
        // Change the count to whatever we CAN read
        Count = (int)((Position() >= Length()) ? 0 : Length() - Position());
    }

    if (Count == 0)
    {
        return 0;
    }

    BYTE MaxSectors = (BYTE)(Helpers::UpToNearestSector(Count + (Position() - RealPosition())) / 0x200); // This is the number of sectors we have to read
    int BytesToShaveOffBeginning = (int)(Position() - RealPosition());	// Number of bytes to remove from the beginning of the buffer
    int BytesToShaveOffEnd = (int)(Helpers::UpToNearestSector(Position() + Count) - (Position() + Count));
    int BytesThatAreInLastDataRead = 0x200 - BytesToShaveOffBeginning;


    int AllDataLength = BytesToShaveOffBeginning + Count + BytesToShaveOffEnd; /* This adds the BytesToShaveOffBeginning
                                                                                              * all of these cool things to form
                                                                                              * the total size that we'll be reading
                                                                                              * (AllDataLength / 0x200 should equal MaxSectors */
    if (MaxSectors != AllDataLength / 0x200)
    {
        throw xException("Assertion fail: MaxSectors != AllDataLength / 0x200.  At: xDeviceStream::Read");
    }

    BYTE* AllData = 0;
    DWORD BytesRead;

    // If the last time we cached data wasn't at this offset
    if (LastReadOffset != RealPosition())
    {
        // Cache
#ifdef _WIN32
        ReadFile(
                    DeviceHandle,	// Device to read from
                    LastReadData,	// Output buffer
                    0x200,			// Read the last sector
                    &BytesRead,		// Pointer to the number of bytes read
                    &Offset);		// OVERLAPPED structure containing the offset to read from
#else
        read(Device, LastReadData, 0x200);
#endif
        LastReadOffset = RealPosition();
    }

    if (BytesThatAreInLastDataRead <= Count)
    {
        SetPosition(Position() + BytesThatAreInLastDataRead);
    }
    else
    {
        SetPosition(Position() + Count);
    }

    if (MaxSectors > 1)
    {
        AllData = new BYTE[AllDataLength - 0x200];

        // Read for all sectors EXCEPT the last one
#ifdef _WIN32
        ReadFile(
                    DeviceHandle,	// Device to read from
                    AllData,		// Output buffer
                    AllDataLength - 0x200,	// Read the last sector
                    &BytesRead,		// Pointer to the number of bytes read
                    &Offset);		// OVERLAPPED structure containing the offset to read from
#else
        read(Device, AllData, AllDataLength - 0x200);
#endif

        SetPosition(Position() + (Count - BytesThatAreInLastDataRead));
    }
    int countRead = ((BytesThatAreInLastDataRead <= Count) ? BytesThatAreInLastDataRead : Count);
    memcpy(DestBuff, LastReadData + BytesToShaveOffBeginning, countRead);
    if (AllData)
    {
        memcpy(DestBuff + BytesThatAreInLastDataRead, AllData, Count - BytesThatAreInLastDataRead);
        // Cache
        memcpy(&LastReadData, AllData + AllDataLength - ((0x200 * 2)), 0x200);
        //LastReadOffset = RealPosition();
        delete[] AllData;
    }

    DetermineAndDoArraySwap(DestBuff, Count);

    return Count;
}

std::string xDeviceStream::ReadString( size_t Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadString");
    }
    if ((Position() + Count) > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xDeviceStream::ReadString");
    }
    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count);
    std::string ret((char*)Buffer);
    delete[] Buffer;

    return ret;
}

std::string xDeviceStream::ReadCString( void )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadCString");
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

std::wstring xDeviceStream::ReadUnicodeString( size_t Count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::ReadUnicodeString");
    }
    else if ((Position() + Count) > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xDeviceStream::ReadUnicodeString");
    }

    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count + 1);
    std::wstring ret((TCHAR*)Buffer);
    delete[] Buffer;

    return ret;
}

void xDeviceStream::WriteByte( BYTE _Byte )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteByte");
    }
    else if (Position() + sizeof(BYTE) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteByte");
    }
    Write(&_Byte, 1);
}

void xDeviceStream::WriteInt16( short _Int16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteInt16");
    }
    else if ((Position() + sizeof(short)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteInt16");
    }
    DetermineAndDoEndianSwap((BYTE*)&_Int16, sizeof(short), sizeof(BYTE));
    Write((BYTE*)&_Int16, sizeof(short));
}

void xDeviceStream::WriteInt32( int _Int32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteInt32");
    }
    else if ((Position() + sizeof(int)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteInt16");
    }

    DetermineAndDoEndianSwap((BYTE*)&_Int32, sizeof(int), sizeof(BYTE));
    Write((BYTE*)&_Int32, sizeof(int));
}

void xDeviceStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteInt64");
    }
    else if ((Position() + sizeof(INT64)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteInt64");
    }

    DetermineAndDoEndianSwap((BYTE*)&_Int64, sizeof(INT64), sizeof(BYTE));
    Write((BYTE*)&_Int64, sizeof(INT64));
}

void xDeviceStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteUInt16");
    }
    else if ((Position() + sizeof(UINT16)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteUInt16");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt16, sizeof(UINT16), sizeof(BYTE));
    Write((BYTE*)&_UInt16, sizeof(UINT16));
}

void xDeviceStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteUInt32");
    }
    else if ((Position() + sizeof(UINT32)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteUInt32");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt32, sizeof(UINT32), sizeof(BYTE));
    Write((BYTE*)&_UInt32, sizeof(UINT32));
}

void xDeviceStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::WriteUInt64");
    }
    else if ((Position() + sizeof(UINT64)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceStream::WriteUInt64");
    }

    DetermineAndDoEndianSwap((BYTE*)&_UInt64, sizeof(UINT64), sizeof(BYTE));
    Write((BYTE*)&_UInt64, sizeof(UINT64));
}

int xDeviceStream::Write( BYTE* Buffer, int count )
{
    if (IsClosed)
    {
        throw xException("Stream is closed. At: xDeviceStream::Write");
    }
#ifdef _WIN32
    else if (DeviceHandle == INVALID_HANDLE_VALUE)
    {
        throw xException("Error: INVALID_HANDLE_VALUE. At: xDeviceStream::Write");
    }
#endif
    //SetPosition(Position());
    // We can't write beyond the end of the stream
    if (Position() + count > Length())
    {
        // Change the count to whatever we CAN write
        //Count = (int)((Position() >= Length()) ? 0 : Length() - Position());
        throw xException("Can not write beyond end of stream! At xDeviceStream::Write");
    }

    if (count == 0)
    {
        return 0;
    }

    int BytesWeNeedToRead = (int)(Helpers::UpToNearestSector(Position() + count) - Position());

    DWORD BytesRead;
    BYTE* AllData = 0;
    // If the last time we cached data wasn't at this offset, and we're only writing to this sector
    if (LastReadOffset != RealPosition())
    {
        // Cache
#ifdef _WIN32
        ReadFile(
                    DeviceHandle,	// Device to read from
                    LastReadData,	// Output buffer
                    0x200,			// Read the last sector
                    &BytesRead,		// Pointer to the number of bytes read
                    &Offset);		// OVERLAPPED structure containing the offset to read from
#else
        read(Device, LastReadData, 0x200);
#endif
        LastReadOffset = RealPosition();
    }

    if (BytesWeNeedToRead > 0x200)
    {
        AllData = new BYTE[BytesWeNeedToRead];
        // Read the data
#ifdef _WIN32
        ReadFile(
                    DeviceHandle,
                    AllData,
                    BytesWeNeedToRead,
                    &BytesRead,
                    &Offset);
#else
        read(Device, AllData, BytesWeNeedToRead);
#endif

        // Write over the data in memory
        memcpy(AllData + Position() - RealPosition(), Buffer, count);

        // Copy the last part of the data to our cached stuff
        memcpy(LastReadData, AllData + BytesWeNeedToRead - (0x200), 0x200);

        DetermineAndDoArraySwap(AllData, BytesWeNeedToRead);

        // Write the data
#ifdef _WIN32
        WriteFile(
                    DeviceHandle,		// Device to read from
                    AllData,			// Data to write
                    BytesWeNeedToRead,	// Amount of data to write
                    &BytesRead,			// Pointer to number of bytes written (ignore that it says BytesRead)
                    &Offset);			// OVERLAPPED structure containing the offset to write from
#else
        write(Device, AllData, BytesWeNeedToRead);
#endif

        delete[] AllData;
    }
    else
    {
        memcpy(LastReadData + Position() - RealPosition(), Buffer, count);

        void* Data = DetermineAndDoEndianSwap(AllData, BytesWeNeedToRead, sizeof(BYTE), true);

        if (!Data)
        {
            // Write the data
#ifdef _WIN32
            WriteFile(
                        DeviceHandle,		// Device to read from
                        LastReadData,		// Data to write
                        BytesWeNeedToRead,	// Amount of data to write
                        &BytesRead,			// Pointer to number of bytes written (ignore that it says BytesRead)
                        &Offset);			// OVERLAPPED structure containing the offset to write from
#else
            write(Device, LastReadData, BytesWeNeedToRead);
#endif
        }
        else
        {
            // Write the data
#ifdef _WIN32
            WriteFile(
                        DeviceHandle,		// Device to read from
                        Data,				// Data to write
                        BytesWeNeedToRead,	// Amount of data to write
                        &BytesRead,			// Pointer to number of bytes written (ignore that it says BytesRead)
                        &Offset);			// OVERLAPPED structure containing the offset to write from
#else
            write(Device, Data, BytesWeNeedToRead);
#endif
        }
    }

    SetPosition(Position() + count);
    return count;
}
}
