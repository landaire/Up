#ifndef __ISTREAM__HG
#define __ISTREAM__HG
#include "../stdafx.h"
#include "../typedefs.h"
#include <vector>

namespace Streams
{
enum FileMode
{
    // Specifies that an existing file should be opened.  If it doesn't exist, an exception is thrown
    Open,
    // Specifies that a new file should be created.  If it exists, it is overwritten
    Create,
    // Specifies that the file should be opened if it exsts; otherwise a new file is created
    OpenOrCreate,
    // Specifies that a new file should be created.  If it already exists, an exception is thrown
    CreateNew
};

enum EndianType
{
    Big,
    Little
};

class IStream
{
private:
    bool isClosed;
    bool isBigEndian ( void );
protected:
    EndianType endian;
    uint64_t length;

public:
    virtual uint64_t    GetPosition() =0;
    virtual void        SetPosition(uint64_t position ) =0;
    virtual uint64_t    Length() =0;

    int GetEndianness	( void );
    void SetEndianness	( EndianType endiantype );

    template<class T>
    void EndianSwap(T &item);

    bool IsClosed();

    /* Writing Functions	 */
    virtual int     Write( uint8_t* Buffer, int count		) =0;
    virtual void	WriteByte		(uint8_t val) =0;

    virtual void	WriteUInt16		(uint16_t val) =0;
    virtual void	WriteUInt32		(uint32_t val) =0;
    virtual void	WriteUInt64		(uint64_t val) =0;

    virtual void	WriteInt16		(int16_t val) =0;
    virtual void	WriteInt32		(int32_t val) =0;
    virtual void	WriteInt64		(int64_t val) =0;
    /* End Writing Functions */

    /* Reading Functions */

    virtual uint8_t     ReadByte() =0;
    virtual uint64_t    ReadUInt64() =0;
    virtual uint32_t    ReadUInt32() =0;
    virtual uint16_t    ReadUInt16() =0;

    virtual int64_t     ReadInt64() =0;
    virtual int32_t     ReadInt32() =0;
    virtual int16_t     ReadInt16() =0;

    virtual size_t Read(uint8_t* buff, size_t count ) =0;

    virtual std::string     ReadString( size_t count ) =0;
    virtual std::u16string	ReadUtf16String(size_t count ) =0;
    virtual std::string     ReadCString() =0;
    /* End Reading Functions */

    virtual void Close() =0;
};
}
#endif
