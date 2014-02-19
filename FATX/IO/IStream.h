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
		void EndianSwap		( void* Data, size_t DataLength, size_t ElementSize );
		bool is_big_endian	( void );
	protected:
		int _Endian;
		uint64_t4_t _Length;

		void* DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray );
		void* DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray, bool Reverse );
        void DetermineAndDoArraySwap( uint8_t* Data, int Length );
        void DetermineAndDoArraySwap( uint8_t* Data, int Length, bool ReverseOrder );

	public:

		bool IsClosed;

        virtual uint64_t4_t Position      ( void ) =0;
        virtual void SetPosition	( uint64_t4_t Position ) =0;
        virtual uint64_t4_t Length		( void ) =0;

		int GetEndianness	( void );
		void SetEndianness	( EndianType endiantype );

                void DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize );

		/* Writing Functions	 */
        virtual int	Write			( uint8_t* Buffer,			// Default function for writing data
									int count		) =0;



		virtual void	WriteUInt16		( unsigned short	_UInt16	) =0;	// Function for writing	an unsigned Int16	
		virtual void	WriteUInt32		( unsigned int		_UInt	) =0;	// Function for writing an unsigned Int32
		virtual void	WriteUInt64		( uint64_t_t_t			_UInt64	) =0;	// Function for writing an unsigned Int64

		virtual void	WriteInt16		( short	_UInt16 ) =0;				// Writes a signed Int16 (WORD)
		virtual void	WriteInt32		( int	_Int32	) =0;				// Writes a signed Int32 (DWORD)
		virtual void	WriteInt64		( uint64_t4_t _Int64	) =0;				// Writes a signed Int64 (QWORD)

		//virtual void	WriteFloat		( float			_Float	) =0;		// Function for writing a float

		virtual void	WriteByte		( uint8_t	_Byte	) =0;				// Function for writing a single byte
		/* End Writing Functions */

		/* Reading Functions */
		virtual uint8_t				ReadByte		( void ) =0;			// Function for reading a single byte

		//virtual float				ReadFloat		( void ) =0;			// Function for reading a float
		virtual uint64_t_t_t				ReadUInt64		( void ) =0;			// Function for reading a uint64
		virtual unsigned int		ReadUInt32		( void ) =0;			// Function for reading a uint
		virtual unsigned short		ReadUInt16		( void ) =0;			// Function for reading a ushort

		virtual uint64_t4_t				ReadInt64		( void ) =0;			// Reads a signed Int64 (QWORD)
		virtual int					ReadInt32		( void ) =0;			// Reads a signed Int32 (DWORD)
		virtual short				ReadInt16		( void ) =0;			// Reads a signed Int16 (WORD)

        virtual int					Read		( uint8_t* DestBuff,
														int Count ) =0;	// Function for reading a byte array

        virtual std::string     ReadString		( size_t Count ) =0;				// Function for reading a string
        virtual std::wstring	ReadUnicodeString(size_t Count ) =0;				// Function for reading a unicode string
        virtual std::string     ReadCString		( void ) =0;						// Function for reading a C-Style string
		/* End Reading Functions */

		virtual void	Close			( void ) =0;						// Closes the stream
	};
}
#endif
