#include "folder.h"

Folder::Folder()
{
}

bool IsTitleIDFolder()
{
    std::string name(Dirent.Name);
    // Title ID's are 8 digits long; this shit isn't a Title ID folder if it's
    // longer or shorter
    if (name.length() != 8)
    {
        return false;
    }
    // It was 8 digits long, let's check if the characters are valid hex digits
    const char* acceptableChars = "0123456789ABCDEF";
    for (int i = 0; i < Dirent.NameSize; i++)
    {
        bool acceptable = false;
        for (int j = 0; j < 16; j++)
        {
            if (Dirent.Name[i] == acceptableChars[j])
            {
                acceptable = true;
                break;
            }
        }
        if (!acceptable)
        {
            return false;
        }
    }
    return true;
}
