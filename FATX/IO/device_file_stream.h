#ifndef __XDFS__HG
#define __XDFS__HG
#include "istream.h"
#include "../xException.h"
#include "../FATX/StaticInformation.h"
#include "../FATX/Drive.h"
#include "../FATX/Helpers.h"
#include "../typedefs.h"

namespace Streams
{
	class DeviceFileStream :
		public IStream
    {
    private:
        File *xf;
        Drive *device;
        uint64_t DiskPosition;
        uint64_t UserPosition;
        void Initialize( File *xf, Drive *device, bool ReadFullChain=true );
        uint64_t GetPhysicalPosition(int FilePosition);
	public:
        DeviceFileStream(const std::string &Path, Drive *device, bool ReadFullChain=true);
        DeviceFileStream(File* dest, Drive *device, bool ReadFullChain=true );
		~DeviceFileStream(void);

        uint64_t Position	( void );
        void SetPosition	( uint64_t Position );
        uint64_t Length		( void );

		/* Writing Functions	 */
         int	Write			( uint8_t* Buffer,			// Default function for writing data
									int count		);

		size_t	Write			( void* Buffer,			// Overload function for writing data without passing a size
                                    size_t ElementSize,
									int count);

		 void	WriteUInt16		( unsigned short	_UInt16	);	// Function for writing	an unsigned Int16	
		 void	WriteUInt32		( unsigned int		_UInt	);	// Function for writing an unsigned Int32
         void	WriteUInt64		( uint64_t			_UInt64	);	// Function for writing an unsigned Int64

		 void	WriteInt16		( short	_UInt16 );				// Writes a signed Int16 (WORD)
		 void	WriteInt32		( int	_Int32	);				// Writes a signed Int32 (DWORD)
		 void	WriteInt64		( uint64_t _Int64	);				// Writes a signed Int64 (QWORD)

		 void	WriteByte		( uint8_t	_Byte	);				// Function for writing a single byte
		/* End Writing Functions */

		/* Reading Functions */
		 uint8_t				ReadByte		( void );			// Function for reading a single byte

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

		 virtual void	Close			( void );						// Closes the stream
	};
}
#endif
