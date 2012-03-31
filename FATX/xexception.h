#ifndef XEXCEPTION_H
#define XEXCEPTION_H

#include <stdexcept>
#include <string>
#include <qDebug>

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

class xException : public std::runtime_error
{
public:
    int Exception;
    xException(const std::string& message, int exception = 0) : std::runtime_error(message)
    {
        Exception = exception;
        qDebug("%s", message.c_str());
    }
};

#endif // XEXCEPTION_H
