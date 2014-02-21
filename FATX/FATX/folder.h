#ifndef FOLDER_H
#define FOLDER_H

class Folder : public IEntry
{
    std::vector<std::shared_ptr<Folder>>    cachedFolders;
    std::vector<std::shared_ptr<File>>      cachedFiles;

public:
    Folder();

    bool IsTitleIdFolder();
};

#endif // FOLDER_H
