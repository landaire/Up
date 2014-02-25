#ifndef IENTRY_H
#define IENTRY_H

#include "folder.h"
#include "static_information.h"

class QDateTime;
class std::string;

class IEntry
{
    bool fullClusterChainRead;
    std::vector<uint32_t> clusterChain;
    xDirent Dirent;

    bool fatxEntriesRead;

    std::shared_ptr<Folder> parent;
    std::shared_ptr<FATXVolume> volume;

    QDateTime DateCreated;
    QDateTime DateModified;
    QDateTime DateAccessed;

public:
    IEntry(std::shared_ptr<Folder> parent, const FATXDirent &dirent);

    const std::string FullPath;
    QDateTime GetDateCreated();
    QDateTime GetDateModified();
    QDateTime GetDateAccessed();
};

#endif // IENTRY_H
