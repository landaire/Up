#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

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

#endif // EXCEPTIONS_H
