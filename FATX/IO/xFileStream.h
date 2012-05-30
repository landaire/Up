#ifndef __XFS__HG
#define __XFS__HG
#include "istream.h"
#include "../typedefs.h"
#include <fstream>

namespace Streams
{
	class xFileStream :
		public IStream
	{
	private:
        std::fstream _FileStream;

    public:
        xFileStream( char *FilePath, int Mode );
        xFileStream( std::string FilePath, int Mode );
        xFileStream( wchar_t *FilePath, int Mode );
        xFileStream( std::wstring FilePath, int Mode );

		~xFileStream(void);

        void Initialize(void *FilePath, int Mode, bool wchar);

        INT64 Position	( void );
        void SetPosition	( INT64 Position );
        INT64 Length		( void );

		/* Writing Functions	 */
         int	Write			( BYTE* Buffer,			// Default function for writing data
									int count		);

		size_t	Write			( void* Buffer,			// Overload function for writing data without passing a size
                                    size_t ElementSize,
									int count);

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

         std::string		ReadString		( size_t Count );		// Function for reading a string
         std::wstring		ReadUnicodeString(size_t Count );		// Function for reading a unicode string
         std::string		ReadCString		( void );				// Function for reading a C-Style string
		/* End Reading Functions */

		 void			Close			( void );
	};
}
#endif
