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
		INT64 _Length;

		void* DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray );
		void* DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize, bool IsByteArray, bool Reverse );
        void DetermineAndDoArraySwap( BYTE* Data, int Length );
        void DetermineAndDoArraySwap( BYTE* Data, int Length, bool ReverseOrder );

	public:

		bool IsClosed;

        virtual INT64 Position      ( void ) =0;
        virtual void SetPosition	( INT64 Position ) =0;
        virtual INT64 Length		( void ) =0;

		int GetEndianness	( void );
		void SetEndianness	( EndianType endiantype );

                void DetermineAndDoEndianSwap( void* Data, size_t DataLength, size_t ElementSize );

		/* Writing Functions	 */
        virtual int	Write			( BYTE* Buffer,			// Default function for writing data
									int count		) =0;



		virtual void	WriteUInt16		( unsigned short	_UInt16	) =0;	// Function for writing	an unsigned Int16	
		virtual void	WriteUInt32		( unsigned int		_UInt	) =0;	// Function for writing an unsigned Int32
		virtual void	WriteUInt64		( UINT64			_UInt64	) =0;	// Function for writing an unsigned Int64

		virtual void	WriteInt16		( short	_UInt16 ) =0;				// Writes a signed Int16 (WORD)
		virtual void	WriteInt32		( int	_Int32	) =0;				// Writes a signed Int32 (DWORD)
		virtual void	WriteInt64		( INT64 _Int64	) =0;				// Writes a signed Int64 (QWORD)

		//virtual void	WriteFloat		( float			_Float	) =0;		// Function for writing a float

		virtual void	WriteByte		( BYTE	_Byte	) =0;				// Function for writing a single byte
		/* End Writing Functions */

		/* Reading Functions */
		virtual BYTE				ReadByte		( void ) =0;			// Function for reading a single byte

		//virtual float				ReadFloat		( void ) =0;			// Function for reading a float
		virtual UINT64				ReadUInt64		( void ) =0;			// Function for reading a uint64
		virtual unsigned int		ReadUInt32		( void ) =0;			// Function for reading a uint
		virtual unsigned short		ReadUInt16		( void ) =0;			// Function for reading a ushort

		virtual INT64				ReadInt64		( void ) =0;			// Reads a signed Int64 (QWORD)
		virtual int					ReadInt32		( void ) =0;			// Reads a signed Int32 (DWORD)
		virtual short				ReadInt16		( void ) =0;			// Reads a signed Int16 (WORD)

        virtual int					Read		( BYTE* DestBuff,
														int Count ) =0;	// Function for reading a byte array

        virtual std::string     ReadString		( size_t Count ) =0;				// Function for reading a string
        virtual std::wstring	ReadUnicodeString(size_t Count ) =0;				// Function for reading a unicode string
        virtual std::string     ReadCString		( void ) =0;						// Function for reading a C-Style string
		/* End Reading Functions */

		virtual void	Close			( void ) =0;						// Closes the stream
	};
}
#endif
