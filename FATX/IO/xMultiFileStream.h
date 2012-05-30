#ifndef __XMFS__HG
#define __XMFS__HG
#include "istream.h"
#include "xFileStream.h"
#include <vector>
#include "../xException.h"
#include <exception>
#include <stdexcept>
namespace Streams
{
	class xMultiFileStream :
		public IStream
	{
	private:
		INT64 UserOffset;
        std::vector<xFileStream*> FileStreams;
		int CurrentStream;

	public:
        xMultiFileStream( std::vector<std::wstring> InPaths );
		~xMultiFileStream(void);

        INT64 Position		( void );
        void SetPosition		( INT64 Position );
        INT64 Length			( void );

		/* Writing Functions	 */
         int	Write			( BYTE* Buffer,			// Default function for writing data
									int count		);

		 void	WriteUInt16		( unsigned short	_UInt16	);	// Function for writing	an unsigned Int16	
		 void	WriteUInt32		( unsigned int		_UInt	);	// Function for writing an unsigned Int32
		 void	WriteUInt64		( UINT64			_UInt64	);	// Function for writing an unsigned Int64

		 void	WriteInt16		( short	_UInt16 );				// Writes a signed Int16 (WORD)
		 void	WriteInt32		( int	_Int32	);				// Writes a signed Int32 (DWORD)
		 void	WriteInt64		( INT64 _Int64	);				// Writes a signed Int64 (QWORD)

		 // void	WriteFloat		( float			_Float	);		// Function for writing a float

		 void	WriteByte		( BYTE	_Byte	);				// Function for writing a single byte
		/* End Writing Functions */

		/* Reading Functions */
		 BYTE				ReadByte		( void );			// Function for reading a single byte

		 // float				ReadFloat		( void );			// Function for reading a float
		 UINT64				ReadUInt64		( void );			// Function for reading a uint64
		 unsigned int		ReadUInt32		( void );			// Function for reading a uint
		 unsigned short		ReadUInt16		( void );			// Function for reading a ushort

		 INT64				ReadInt64		( void );			// Reads a signed Int64 (QWORD)
		 int				ReadInt32		( void );			// Reads a signed Int32 (DWORD)
		 short				ReadInt16		( void );			// Reads a signed Int16 (WORD)

         int				Read	( BYTE* DestBuff,
										int Count );			// Function for reading a byte array

         std::string			ReadString		( size_t Count );		// Function for reading a string
         std::wstring		ReadUnicodeString(size_t Count );		// Function for reading a unicode string
         std::string			ReadCString		( void );				// Function for reading a C-Style string
		/* End Reading Functions */

		 virtual void	Close			( void );						// Closes the stream
	};
}
#endif
