#include "stdafx.h"
#include "IStream.h"
#include <array>

using std::array;
namespace Streams
{
int IStream::GetEndianness()
{
    return endian;
}

void IStream::SetEndianness(EndianType endianType )
{
    endian = endianType;
}
bool IStream::isBigEndian( void )
{
    union {
        unsigned int i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

template<class T>
void EndianSwap(T &item)
{
    // Check if this is an array
    if (array &arr = dynamic_cast<std::array&>(item)) {
        size_t elementSize = arr.size_type;
        // Iterate through each element in the array
        for (auto &element : arr) {
            EndianSwap(element);
        }
    } else {
        uint8_t *elemPtr = &element, temp;
        // Swap every byte
        for (size_t start = 0, end = elementSize - 1; start != end; start++, end--) {
            temp = *(elemPtr + start);
            *(elemPtr + start) = *(elemPtr + end);
            *(elemPtr + end) = temp;
        }
    }
}
}
