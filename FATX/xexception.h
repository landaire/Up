#ifndef XEXCEPTION_H
#define XEXCEPTION_H
#include "stdafx.h"
#include <exception>
using namespace std;

enum Exceptions
{
    ExInvalidFile,
    ExExternalPathsNotFound,
    ExVolumeInvalidMagic,
    ExVolumeInvalidSectorsPerCluster,
    ExVolumeTooSmall,
    ExVolumeTooManyClusters,
    ExVolumeFolderNotFound,
    ExVolumeFileNotFound,
    ExStreamSetPosition,
    ExStreamReadByte,
    ExStreamReadInt16,
    ExStreamReadUInt16,
    ExStreamReadInt32,
    ExStreamReadUInt32,
    ExStreamReadInt64,
    ExStreamReadUInt64,
    ExStreamRead,
    ExStreamWrite
};

class xException : public exception
{
public:
    xException(const char Message[], int Error = 0);
    virtual const char *what() const throw()
    {
        return Message;
    }

    char Message[0x200];
    int Error;
};

#endif // XEXCEPTION_H
