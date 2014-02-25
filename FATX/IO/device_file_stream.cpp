#include "StdAfx.h"
#include "device_file_stream.h"

namespace Streams
{
DeviceFileStream::DeviceFileStream(const std::string& Path, Drive *device, bool ReadFullChain)
{
    File *dest = device->FileFromPath(Path);
    Initialize(dest, device, ReadFullChain);
}

DeviceFileStream::DeviceFileStream(File *dest, Drive *device, bool ReadFullChain)
{
    Initialize(dest, device, ReadFullChain);
}

DeviceFileStream::~DeviceFileStream(void)
{
}

void DeviceFileStream::Initialize(File *xf, Drive *device, bool ReadFullChain)
{
    IsClosed() = false;
    endian = Big;
    this->xf = xf;
    this->device = device;
    UserPosition = 0;
    IsClosed() = false;
    if (!xf->ClusterChain.size() || (!xf->FullClusterChainRead & ReadFullChain))
    {
        device->ReadClusterChain(xf->ClusterChain, xf->Dirent, *xf->Volume);
    }
    SetPosition(0);
}

void DeviceFileStream::Close( void )
{
    IsClosed() = true;
}

INT64 DeviceFileStream::GetPosition( void )
{
    return UserPosition;
}

INT64 DeviceFileStream::Length( void )
{
    return (INT64)xf->Dirent.FileSize;
}

void DeviceFileStream::SetPosition( INT64 Position )
{
    device->DeviceStream->SetPosition(GetPhysicalPosition(Position));
    UserPosition = Position;
}

INT64 DeviceFileStream::GetPhysicalPosition(int FilePosition)
{
    int ClusterIndex = Helpers::DownToNearestX(FilePosition, xf->Volume->ClusterSize) / xf->Volume->ClusterSize;
    FilePosition -= (xf->Volume->ClusterSize * ClusterIndex);
    if (ClusterIndex >= xf->ClusterChain.size())
    {
        // Return the last cluster offset + one cluster more
        return ((xf->ClusterChain.at(xf->ClusterChain.size() - 1) - 1) * xf->Volume->ClusterSize) + xf->Volume->DataStart + xf->Volume->ClusterSize;
    }
    return ((xf->ClusterChain.at(ClusterIndex) - 1) * xf->Volume->ClusterSize) + xf->Volume->DataStart + FilePosition;
}

BYTE DeviceFileStream::ReadByte( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadByte");
    }
    else if (GetPosition() > Length() - sizeof(BYTE))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadByte");
    }

    BYTE Return;
    Read(&Return, 1);

    return Return;
}

short DeviceFileStream::ReadInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadInt16");
    }
    else if (GetPosition() > Length() - sizeof(INT16))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadInt16");
    }

    BYTE temp[2];
    Read((BYTE*)&temp, 2);

    short Return = (short)temp[0] << 8 | temp[1];

    return Return;
}

int DeviceFileStream::ReadInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadInt32");
    }
    else if (GetPosition() > Length() - sizeof(INT32))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadInt32");
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

INT64 DeviceFileStream::ReadInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadInt64");
    }
    else if (GetPosition() > Length() - sizeof(INT64))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadInt64");
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

UINT16 DeviceFileStream::ReadUInt16( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadUInt16");
    }
    else if (GetPosition() > Length() - sizeof(UINT16))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadUInt16");
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

UINT32 DeviceFileStream::ReadUInt32( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadUInt32");
    }
    else if (GetPosition() > Length() - sizeof(UINT32))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadUInt32");
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

UINT64 DeviceFileStream::ReadUInt64( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadUInt64");
    }
    else if (GetPosition() > Length() - sizeof(UINT64))
    {
        throw xException("End of file reached.  At: xDeviceFileStream::ReadUInt64");
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

int DeviceFileStream::Read( BYTE* DestBuff, int Count )
{
    SetPosition(GetPosition());
    // How many clusters the data we're reading is spread across
    int ClustersSpanned = Helpers::UpToNearestX(Count + (UserPosition - Helpers::DownToNearestX(UserPosition, xf->Volume->ClusterSize)), xf->Volume->ClusterSize) / xf->Volume->ClusterSize;
    // Read the first amount of data...
    int DataReadableInFirstCluster = xf->Volume->ClusterSize - (UserPosition - Helpers::DownToNearestX(UserPosition, (INT64)xf->Volume->ClusterSize));
    if (DataReadableInFirstCluster < Count)
    {
        int offset = 0;
        int tcount = 0;
        tcount += device->DeviceStream->Read(DestBuff, DataReadableInFirstCluster);
        offset += tcount;
        SetPosition(GetPosition() + tcount);
        Count -= tcount;

        int CurrentCluster = 0;
        // The number of clusters that are consecutively aligned on the disk
        int ConsecutiveClusters = 1;
        // This will hold the value of the last cluster
        int LastCluster = 0;

        for (int i = 1; i < ClustersSpanned - 1; i++)
        {
            CurrentCluster = (UserPosition - (UserPosition % xf->Volume->ClusterSize)) / xf->Volume->ClusterSize;
            ConsecutiveClusters = 1;
            LastCluster = xf->ClusterChain[CurrentCluster];
            // Starting at the next cluster, and looping up until the last cluster, minus one...
            // Current Cluster + ClustersSpanned
            for (int j = CurrentCluster + 1; j < (CurrentCluster + ClustersSpanned - i) - 1; j++)
            {
                //qDebug("%X/%X", xf->ClusterChain[j], LastCluster + 1);
                // If this cluster index is equal to the last cluster index, + 1...
                if (xf->ClusterChain[j] == LastCluster + 1 && j < (ClustersSpanned - CurrentCluster - i) - 1)
                {
                    // Shit, it's consecutive.  We better continue.
                    ConsecutiveClusters++;
                    LastCluster = xf->ClusterChain[j];
                }
                else
                    // SHIT, IT'S NOT CONSECUTIVE.  WE BETTER NOT DO STUFF TO THINGS
                    break;
            }
            tcount = device->DeviceStream->Read(DestBuff + offset, xf->Volume->ClusterSize * ConsecutiveClusters);
            offset += tcount;
            Count -= tcount;
            SetPosition(GetPosition() + tcount);
            // So that we don't re-read these clusters...
            i += ConsecutiveClusters - 1;
        }
        // Read the last bit of data
        tcount = device->DeviceStream->Read(DestBuff + offset, Count);
        SetPosition(GetPosition() + tcount);
        offset += tcount;
        return offset;
    }
    else
    {
        int c = device->DeviceStream->Read(DestBuff, Count);
        SetPosition(GetPosition() + Count);
        return c;
    }
}

std::string DeviceFileStream::ReadString( size_t Count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadString");
    }
    if ((GetPosition() + Count) > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xDeviceFileStream::ReadString");
    }
    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count);
    std::string ret((char*)Buffer);
    delete[] Buffer;

    return ret;
}

std::string DeviceFileStream::ReadCString( void )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadCString");
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

std::wstring DeviceFileStream::ReadUnicodeString( size_t Count )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::ReadUnicodeString");
    }
    else if ((GetPosition() + Count) > Length())
    {
        throw xException("Can not read beyond end of stream.  At: xDeviceFileStream::ReadUnicodeString");
    }

    BYTE* Buffer = new BYTE[Count + 1];
    memset(Buffer, 0, Count + 1);

    Read(Buffer, Count);
    for (int i = 0; i < Count; i += sizeof(wchar_t))
    {
        determineAndDoEndianSwap(Buffer + i, sizeof(wchar_t), sizeof(char));
    }
    std::wstring ret((TCHAR*)Buffer);
    delete[] Buffer;

    return ret;
}

void DeviceFileStream::WriteByte( BYTE _Byte )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteByte");
    }
    else if (GetPosition() + sizeof(BYTE) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteByte");
    }
    Write(&_Byte, 1);
}

void DeviceFileStream::WriteInt16( short _Int16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteInt16");
    }
    else if ((GetPosition() + sizeof(short)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteInt16");
    }
    determineAndDoEndianSwap((BYTE*)&_Int16, sizeof(short), sizeof(BYTE));
    Write((BYTE*)&_Int16, sizeof(short));
}

void DeviceFileStream::WriteInt32( int _Int32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteInt32");
    }
    else if ((GetPosition() + sizeof(int)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteInt16");
    }

    determineAndDoEndianSwap((BYTE*)&_Int32, sizeof(int), sizeof(BYTE));
    Write((BYTE*)&_Int32, sizeof(int));
}

void DeviceFileStream::WriteInt64( INT64 _Int64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteInt64");
    }
    else if ((GetPosition() + sizeof(INT64)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteInt64");
    }

    determineAndDoEndianSwap((BYTE*)&_Int64, sizeof(INT64), sizeof(BYTE));
    Write((BYTE*)&_Int64, sizeof(INT64));
}

void DeviceFileStream::WriteUInt16( UINT16 _UInt16 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteUInt16");
    }
    else if ((GetPosition() + sizeof(UINT16)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteUInt16");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt16, sizeof(UINT16), sizeof(BYTE));
    Write((BYTE*)&_UInt16, sizeof(UINT16));
}

void DeviceFileStream::WriteUInt32( UINT32 _UInt32 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteUInt32");
    }
    else if ((GetPosition() + sizeof(UINT32)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteUInt32");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt32, sizeof(UINT32), sizeof(BYTE));
    Write((BYTE*)&_UInt32, sizeof(UINT32));
}

void DeviceFileStream::WriteUInt64( UINT64 _UInt64 )
{
    if (IsClosed())
    {
        throw xException("Stream is closed. At: xDeviceFileStream::WriteUInt64");
    }
    else if ((GetPosition() + sizeof(UINT64)) > Length())
    {
        throw xException("End of file reached.  At: xDeviceFileStream::WriteUInt64");
    }

    determineAndDoEndianSwap((BYTE*)&_UInt64, sizeof(UINT64), sizeof(BYTE));
    Write((BYTE*)&_UInt64, sizeof(UINT64));
}

int DeviceFileStream::Write( BYTE* Buffer, int count )
{
#ifndef _WIN32
#warning "ADD HANDING FOR EXTENDING FILES"
#endif
    //SetPosition(Position());
    // How many clusters the data we're writing is spread across
    int ClustersSpanned = (count + (UserPosition - Helpers::DownToNearestX(UserPosition, xf->Volume->ClusterSize))) / xf->Volume->ClusterSize;
    ++ClustersSpanned;
    // Read the first amount of data...
    int DataWritableInFirstCluster = xf->Volume->ClusterSize - (UserPosition - Helpers::DownToNearestX(UserPosition, (INT64)xf->Volume->ClusterSize));
    if (DataWritableInFirstCluster < count)
    {
        int offset = 0;
        int tcount = 0;
        tcount += device->DeviceStream->Read(Buffer, DataWritableInFirstCluster);
        offset += tcount;
        SetPosition(GetPosition() + tcount);
        count -= tcount;
        for (int i = 1; i < ClustersSpanned - 1; i++)
        {
            tcount = device->DeviceStream->Read(Buffer + offset, count);
            offset += tcount;
            count -= tcount;
            SetPosition(GetPosition() + tcount);
        }
        // Read the last bit of data
        tcount = device->DeviceStream->Read(Buffer + offset, count);
        SetPosition(GetPosition() + tcount);
        offset += tcount;
        return offset;
    }
    else
    {
        int c = device->DeviceStream->Read(Buffer, count);
        SetPosition(GetPosition() + c);
        return c;
    }
}

}
