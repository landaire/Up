#include "stdafx.h"
#include "IStream.h"

namespace Streams
{
int IStream::GetEndianness()
{
    return _Endian;
}

void IStream::SetEndianness(EndianType endiantype )
{
    _Endian = endiantype;
}
bool IStream::is_big_endian( void )
{
    union {
        unsigned int i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

void IStream::EndianSwap ( void* Data, size_t DataLength, size_t ElementSize )
{
    // i starts at the beginning, j starts at the end.
    // Loops until i >= j
    for (int i = 0, j = (DataLength * ElementSize) - 1; i < j; i++, j--)
    {
        // Create a temp copy of i's index
        unsigned char temp = *((unsigned char*)Data + i);
        // Replace whatever is at i with j
        *((unsigned char*)Data + i) = *((unsigned char*)Data + j);
        // Replace j with whatever was at i
        *((unsigned char*)Data + j) = temp;
    }
}

void IStream::DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize )
{
    DetermineAndDoEndianSwap( Data, DataLength, ElementSize, false );
}

void* IStream::DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray )
{
    if (!IsByteArray)
    {
        if ((!is_big_endian() && _Endian == Big) || (is_big_endian() && _Endian == Little))
        {
            EndianSwap( Data, DataLength, ElementSize );
            return 0;
        }
    }
    else if ((is_big_endian() && _Endian == Big) || (!is_big_endian() && _Endian == Little))
    {
        BYTE* NewData = new BYTE[ElementSize * DataLength];
        memcpy(NewData, (BYTE*)Data, ElementSize * DataLength);
        EndianSwap( NewData, DataLength, ElementSize );
        return NewData;
    }
    return 0;
}

void* IStream::DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray, bool Reverse)
{
    if (!Reverse)
    {
        return DetermineAndDoEndianSwap(Data, DataLength, ElementSize, IsByteArray);
    }

    if (!IsByteArray)
    {
        if ((is_big_endian() && _Endian == Big) || (!is_big_endian() && _Endian == Little))
        {
            EndianSwap( Data, DataLength, ElementSize );
            return 0;
        }
    }
    else if ((!is_big_endian() && _Endian == Big) || (is_big_endian() && _Endian == Little))
    {
        BYTE* NewData = new BYTE[ElementSize * DataLength];
        memcpy(NewData, (BYTE*)Data, ElementSize * DataLength);
        EndianSwap( NewData, DataLength, ElementSize );
        return NewData;
    }
    return 0;
}

void IStream::DetermineAndDoArraySwap( BYTE* Data, int Length )
{
    if ((is_big_endian() && _Endian == Big) || (!is_big_endian() && _Endian == Little))
    {
        EndianSwap( Data, Length, sizeof(BYTE));
    }
}

void IStream::DetermineAndDoArraySwap( BYTE* Data, int Length, bool ReverseOrder )
{
    if (!ReverseOrder)
    {
        DetermineAndDoArraySwap(Data, Length);
    }
    else if ((!is_big_endian() && _Endian == Big) || (is_big_endian() && _Endian == Little))
    {
        EndianSwap( Data, Length, sizeof(BYTE));
    }
}
}
