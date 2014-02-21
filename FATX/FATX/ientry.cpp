#include "ientry.h"

IEntry::IEntry(std::shared_ptr<Folder> parent, const FATXDirent &dirent) : FullPath(parent.FullPath + dirent.Name)
{
    this->volume = parent->volume;
}
