#ifndef __XDS__HG
#define __XDS__HG
#include "IStream.h"
#include "../xException.h"

#ifdef _WIN32
#include <windows.h>
#include <WinIoCtl.h>
#else
#define _FILE_OFFSET_BITS 64
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/disk.h>
#include <unistd.h>
#endif

#include "../FATX/Helpers.h"

#ifndef _WIN32
#include <sys/stat.h>
#endif

namespace Streams
{
	class DeviceStream : public IStream
	{
	private:
#ifdef _WIN32
		HANDLE DeviceHandle;
		OVERLAPPED Offset;
#else
        int Device;
        uint64_t Offset;
#endif
		uint64_t UserOffset;
		uint8_t LastReadData[0x200];
		uint64_t LastReadOffset;

		uint64_t RealPosition( void );
	public:
        DeviceStream( std::string DevicePath );
		~DeviceStream(void);

        uint64_t Position	( void );
        void SetPosition	( uint64_t Position );
        uint64_t Length		( void );

		/* Writing Functions	 */
         int	Write			( uint8_t* Buffer,			// Default function for writing data
									int count		);

		 void	WriteUInt16		( unsigned short	_UInt16	);	// Function for writing	an unsigned Int16	
		 void	WriteUInt32		( unsigned int		_UInt	);	// Function for writing an unsigned Int32
		 void	WriteUInt64		( uint64_t			_UInt64	);	// Function for writing an unsigned Int64

		 void	WriteInt16		( short	_UInt16 );				// Writes a signed Int16 (WORD)
		 void	WriteInt32		( int	_Int32	);				// Writes a signed Int32 (DWORD)
		 void	WriteInt64		( uint64_t _Int64	);				// Writes a signed Int64 (QWORD)

		 // void	WriteFloat		( float			_Float	);		// Function for writing a float

		 void	WriteByte		( uint8_t	_Byte	);				// Function for writing a single byte
		/* End Writing Functions */

		/* Reading Functions */
		 uint8_t				ReadByte		( void );			// Function for reading a single byte

		 // float				ReadFloat		( void );			// Function for reading a float
		 uint64_t				ReadUInt64		( void );			// Function for reading a uint64
		 unsigned int		ReadUInt32		( void );			// Function for reading a uint
		 unsigned short		ReadUInt16		( void );			// Function for reading a ushort

		 uint64_t				ReadInt64		( void );			// Reads a signed Int64 (QWORD)
		 int				ReadInt32		( void );			// Reads a signed Int32 (DWORD)
		 short				ReadInt16		( void );			// Reads a signed Int16 (WORD)

         int				Read	( uint8_t* DestBuff,
										int Count );			// Function for reading a byte array

         std::string			ReadString		( size_t Count );		// Function for reading a string
         std::wstring		ReadUnicodeString(size_t Count );		// Function for reading a unicode string
         std::string			ReadCString		( void );				// Function for reading a C-Style string
		/* End Reading Functions */

		 void			Close			( void );
	};
}
#endif
