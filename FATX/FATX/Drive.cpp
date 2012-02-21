#include "StdAfx.h"
#include "Drive.h"
#include "../IO/xDeviceFileStream.h"
#include <QMetaType>

Drive::Drive( TCHAR* Path, TCHAR* FriendlyName, bool IsUsb ) : QObject()
{
    IsDevKitDrive = false;
    if (!IsUsb)
    {
        DeviceStream = new Streams::xDeviceStream(Path);
        Type = DeviceDisk;
    }
    else
    {
        // USB
#ifdef __WINDOWS__
        QRegExp qr("*:/");
        qr.setPatternSyntax(QRegExp::Wildcard);
        qr.setCaseSensitivity(Qt::CaseInsensitive);
#else
        QRegExp qr("/Volumes/*");
        qr.setPatternSyntax(QRegExp::Wildcard);
        qr.setCaseSensitivity(Qt::CaseInsensitive);
#endif
        if (qr.exactMatch(QString::fromWCharArray(Path)))
        {
            // Find all valid Xbox 360 files
            vector<wstring> Paths;
            TCHAR TempPath[0x100] = {0};
            int PathLength = wcslen(Path);
            PathLength += 19;
            for (int i = 0; i < 1000; i++)
            {
                memset(TempPath, 0, 0x100);
                TCHAR* FilePath = 0;
#warning "Fix for windows you fucking son of a bitch cunt ass nigger"
                if (i < 10)
                {
                    FilePath = L"Xbox360/Data000";
                }
                else if (i < 100)
                {
                    FilePath = L"Xbox360/Data00";
                }
                else if (i < 1000)
                {
                    FilePath = L"Xbox360/Data0";
                }
                wcscpy(TempPath, Path);
                wcscpy(&(TempPath[0]) + wcslen(Path), FilePath);
                swprintf(&(TempPath[0]) + wcslen(Path) + wcslen(FilePath), 3, L"%d", i);

                try
                {
                    Streams::xFileStream temp(TempPath, Streams::Open);
                    // File opened with no exceptions thrown, close the stream and add the path to the vector
                    temp.Close();
                    Paths.push_back(wstring(TempPath));
                }
                catch (...)
                {
                    // file's not good, break the loop
                    break;
                }
            }
            if (Paths.size() == 0)
            {
                throw xException("No paths found for device!", ExExternalPathsNotFound);
            }
            DeviceStream = new Streams::xMultiFileStream(Paths);
            Type = DeviceUsb;
        }
        // Backup
        else
        {
            DeviceStream = new Streams::xFileStream(Path, Streams::Open);
            Type = DeviceBackup;
        }
    }
    this->FriendlyName += FriendlyName;
    this->FriendlySize = Helpers::ConvertToFriendlySize(GetLength());

    InitializePartitions();
}

Drive::~Drive(void)
{

}

void Drive::CopyFileToLocalDisk(File *dest, string Output)
{
    // Get the stream to the file
    Streams::xDeviceFileStream *xf = new Streams::xDeviceFileStream(dest, this);
    // Get a stream to the output file
    Streams::xFileStream *output = new Streams::xFileStream(Output.c_str(), Streams::Create);
    UINT64 size = xf->Length();
    BYTE Buffer[0x4000] = {0};
    Progress p;
    p.Maximium = Helpers::UpToNearestX(dest->Dirent.FileSize, dest->Volume->ClusterSize) / 0x4000;
    p.Current = 0;
    p.FilePath = std::string(dest->Dirent.Name);
    while (size > 0x4000)
    {
        size -= 0x4000;
        xf->Read(Buffer, 0x4000);
        output->Write(Buffer, 0x4000);
        p.Current++;
        emit FileProgressChanged(p);
    }
    // Read the last section of data
    xf->Read(Buffer, size);
    output->Write(Buffer, size);
    p.Current++;
    emit FileProgressChanged(p);

    xf->Close();
    output->Close();
}

void Drive::CopyFileToLocalDisk(string Path, string Output)
{
    CopyFileToLocalDisk(FileFromPath(Path), Output);
}

void Drive::DestroyFolder(Folder *Directory)
{
    while(Directory->CachedFiles.size())
    {
        File *f = Directory->CachedFiles.at(0);
        Directory->CachedFiles.erase(Directory->CachedFiles.begin());
        delete f;
    }

    while(Directory->CachedFolders.size())
    {
        Folder *f = Directory->CachedFolders.at(0);
        Directory->CachedFolders.erase(Directory->CachedFolders.begin());
        DestroyFolder(f);
    }
    delete Directory;
}

void Drive::split(const string &s, char delim, vector<string> &elems) {
    qDebug("String stream s: %s", s.c_str());
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
}

Folder *Drive::FolderFromPath(string Path)
{
    for (int i = 0; i < (int)ValidVolumes.size(); i++)
    {
        xVolume* activePartition = ValidVolumes.at(i);

        // Split the path so by the backslash
        vector<string> PathSplit;
        split(Path, '/', PathSplit);
        // Match the partition name to that of the one we were given
        QRegExp reg(PathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        if (reg.exactMatch(activePartition->Name.c_str()))
        {
            qDebug("Active volume found: %s", activePartition->Name.c_str());
            // We've found the partition, now get the root folder
            Folder *current = activePartition->Root;
            current->Dirent.ClusterStart = activePartition->RootDirectoryCluster;
            do
            {
                PathSplit.erase(PathSplit.begin());
                if (!current->FatxEntriesRead)
                {
                    qDebug("Reading dirents for %s", current->Dirent.Name);
                    ReadDirectoryEntries(current);
                    qDebug("Finished reading dirents!");
                }
                if (!PathSplit.size() )
                {
                    qDebug("Beaking loop.");
                    break;
                }
                bool Found = false;
                for (int j = 0; j < (int)current->CachedFolders.size(); j++)
                {
                    Folder *f = current->CachedFolders.at(j);
                    qDebug("f = %s", f->Dirent.Name);
                    QRegExp fReg(PathSplit.at(0).c_str());
                    fReg.setCaseSensitivity(Qt::CaseInsensitive);
                    if (fReg.exactMatch(f->Dirent.Name))
                    {
                        qDebug("Match!");
                        Found = true;
                        current = f;
                        break;
                    }
                }
                if (!Found)
                {
                    string Name = PathSplit.at(i);
                    qDebug("Folder %s not found", Name.c_str());
                    throw xException("Folder not found", ExVolumeFolderNotFound);
                }
            }
            while (PathSplit.size() > 0);
            return current;
        }
    }
    throw xException("Folder not found", ExVolumeFolderNotFound);
}

File *Drive::FileFromPath(string Path)
{
    char Friendly[0x50] = {0};
    wcstombs(Friendly, FriendlyName.c_str(), FriendlyName.size());
    string cmp = Path.substr(0, Path.find('/'));
    // Because I'm stupid I need to convert
    if (cmp == string(Friendly))
        Path = Path.substr(Path.find('/') + 1);
    // Loop through each volume
    for (int i = 0; i < (int)ValidVolumes.size(); i++)
    {
        xVolume* activePartition = ValidVolumes.at(i);

        // Split the path so by the backslash
        vector<string> PathSplit;
        split(Path, '/', PathSplit);

        // Match the partition name to that of the one we were given
        QRegExp reg(PathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        if (reg.exactMatch(activePartition->Name.c_str()))
        {
            // We've found the partition, now get the root folder
            Folder *current = activePartition->Root;
            current->Dirent.ClusterStart = activePartition->RootDirectoryCluster;
            do
            {
                // Remove this index
                PathSplit.erase(PathSplit.begin());

                // If there's nothing left to find, break the loop
                if (!PathSplit.size())
                {
                    break;
                }

                // If the dirents haven't been read
                if (!current->FatxEntriesRead)
                {
                    // Read the dirents
                    ReadDirectoryEntries(current);
                }

                bool Found = false;
                if (PathSplit.size() > 1)
                {
                    for (int j = 0; j < (int)current->CachedFolders.size(); j++)
                    {
                        // Get the subfolder
                        Folder *f = current->CachedFolders.at(j);

                        // Try to match the name to whatever we're looking for
                        QRegExp fReg(PathSplit.at(0).c_str());
                        fReg.setCaseSensitivity(Qt::CaseInsensitive);
                        if (fReg.exactMatch(f->Dirent.Name))
                        {
                            Found = true;
                            current = f;
                            break;
                        }
                    }
                }
                else
                {
                    for (int j = 0; j < (int)current->CachedFiles.size(); j++)
                    {
                        // Get the current file
                        File *f = current->CachedFiles.at(j);

                        // Try to make a match!
                        QRegExp fReg(PathSplit.at(0).c_str());
                        fReg.setCaseSensitivity(Qt::CaseInsensitive);
                        if (fReg.exactMatch(f->Dirent.Name))
                        {
                            return f;
                        }
                    }
                }
                if (!Found)
                {
                    throw xException("File not found", ExVolumeFileNotFound);
                }
            }
            while (PathSplit.size() > 0);
        }
    }
    throw xException("Folder not found", ExVolumeFolderNotFound);
}

void Drive::ReadDirectoryEntries(Folder* Directory)
{
    // If the cluster chain size is 0, we haven't read the cluster chain yet.  We better do that
    if (!Directory->ClusterChain.size())
    {
        qDebug("Reading cluster chain for %s", Directory->Dirent.Name);
        ReadClusterChain(Directory->ClusterChain, Directory->Dirent, *(Directory->Volume));
        qDebug("Cluster chain read!");
    }
    // Loop for each cluster
    for (int i = 0; i < (int)Directory->ClusterChain.size(); i++)
    {
        // Set the IO's position to the start of the cluster
        DeviceStream->SetPosition(Directory->Volume->DataStart +
                        ((UINT64)(Directory->ClusterChain.at(i) - 1)   * Directory->Volume->ClusterSize));
        // Loop for the maximum amount of entries per cluster
        for (int j = 0; j < 0x40 * Directory->Volume->ClusterSize; j++)
        {
            xDirent Entry;
            memset(&Entry, 0, sizeof(Entry));
            Entry.Offset = DeviceStream->Position();
            // Would just read the entire header here, but since there's the endian swap...
            Entry.NameSize = DeviceStream->ReadByte();

            // For now, I'm not messing with deleted stuff
            if (Entry.NameSize == FAT_DIRENT_DELETED)
            {
                DeviceStream->SetPosition(DeviceStream->Position() + 0x3F);
                continue;
            }
            if (Entry.NameSize == FAT_DIRENT_NEVER_USED || Entry.NameSize == FAT_DIRENT_NEVER_USED2)
            {
                break;
            }

            Entry.Attributes = DeviceStream->ReadByte();
            sprintf(Entry.Name, "%s", DeviceStream->ReadString(Entry.NameSize).c_str());

            DeviceStream->SetPosition((DeviceStream->Position() + 0x2A) - Entry.NameSize);

            Entry.ClusterStart              = DeviceStream->ReadUInt32();
            Entry.FileSize                  = DeviceStream->ReadUInt32();
            Entry.DateCreated.AsDWORD       = DeviceStream->ReadUInt32();
            Entry.DateLastWritten.AsDWORD   = DeviceStream->ReadUInt32();
            Entry.DateAccessed.AsDWORD      = DeviceStream->ReadUInt32();

            // All of that's done, now determine what type of entry it is
            if (Entry.Attributes & Attributes::DIRECTORY)
            {
                // Folder, sweet.
                Folder *f           = new Folder();
                f->Dirent           = Entry;
                f->FullPath         = Directory->FullPath + "/";
                f->FullPath         += Entry.Name;
                f->Parent           = Directory;
                f->FatxEntriesRead  = false;
                f->Volume           = Directory->Volume;
                f->DateAccessed     = Helpers::IntToQDateTime(Entry.DateLastWritten);
                f->DateCreated      = Helpers::IntToQDateTime(Entry.DateCreated);
                f->DateModified     = Helpers::IntToQDateTime(Entry.DateAccessed);
                Directory->CachedFolders.push_back(f);
            }
            else
            {
                File *f         = new File();
                f->Dirent       = Entry;
                f->FullPath     = Directory->FullPath + "/";
                f->FullPath     += Entry.Name;
                f->Parent       = Directory;
                f->Volume       = Directory->Volume;
                f->DateAccessed = Helpers::IntToQDateTime(Entry.DateLastWritten);
                f->DateCreated  = Helpers::IntToQDateTime(Entry.DateCreated);
                f->DateModified = Helpers::IntToQDateTime(Entry.DateAccessed);
                Directory->CachedFiles.push_back(f);
            }
        }
    }
    Directory->FatxEntriesRead = true;
}

void Drive::ReadClusterChain(std::vector<UINT32>& Chain, xDirent Entry, xVolume RelativePartition)
{
    qDebug("Clearing chain");
    // Clear the chain
    Chain.clear();
    qDebug("Setting end of chain identifier");
    // The int that signifies end of cluster chain
    UINT32 End = (RelativePartition.EntrySize == 2) ? FAT_CLUSTER16_LAST : FAT_CLUSTER_LAST;
    qDebug("Setting previous cluster");
    // The previous cluster
    UINT32 Previous = Entry.ClusterStart;
    // Add the base cluster to the chain
    Chain.push_back(Previous);
    qDebug("Cluster chain first cluster 0x%X", Previous);
    // Loop
    do
    {
        // Set the IO to the FAT index
        DeviceStream->SetPosition((RelativePartition.Offset + 0x1000) + (UINT64)(Previous * RelativePartition.EntrySize));
        // Read the int there
        if (RelativePartition.EntrySize == 2)
        {
            qDebug("Reading short");
            Previous = (UINT32)DeviceStream->ReadUInt16();
            qDebug("Short read, value: %X", Previous);
        }
        else
        {
            qDebug("Reading integer (32)");
            Previous = (UINT32)DeviceStream->ReadUInt32();
            qDebug("Integer (32) read, value: %X", Previous);
        }
        // If we haven't reached the end of the chain, add the previous cluster to our chain
        if (Previous != End)
        {
            Chain.push_back(Previous);
        }
    }
    while(Previous != FAT_CLUSTER_AVAILABLE && Previous != End);

    if (Previous == FAT_CLUSTER_AVAILABLE)
    {
        char buffer[0x46];
        sprintf(buffer, "Free block referenced in FAT cluster chain! Dirent offset: 0x%lX", Entry.Offset);
        throw xException(buffer);
    }
}

void Drive::Close( void )
{
    DeviceStream->Close();
    while (ValidVolumes.size())
    {
        xVolume *x = ValidVolumes.at(0);
        DestroyFolder(x->Root);
        ValidVolumes.erase(ValidVolumes.begin());
        delete x;
    }
}

vector<string> Drive::Partitions( void )
{
    if (!_partitions.size())
    {
        for (int i = 0; i < (int)ValidVolumes.size(); i++)
        {
            _partitions.push_back(ValidVolumes.at(i)->Name);
        }
    }
    return _partitions;
}

UINT64 Drive::PartitionGetLength( string Partition )
{
    for (int i = 0; i < (int)ValidVolumes.size(); i++)
    {
        QRegExp rgx(Partition.c_str());
        rgx.setCaseSensitivity(Qt::CaseInsensitive);
        if (rgx.exactMatch(ValidVolumes.at(i)->Name.c_str()))
        {
            return ValidVolumes.at(i)->Size;
        }
    }
    throw xException("Partition not found!", 64);
}

void Drive::SetValidPartitions( void )
{
    if (!ValidVolumes.size())
    {
        // Get the partitions

        // Dev kit partitions
        if (IsDevKitDrive)
        {
            DeviceStream->SetPosition(8);
            vector<DevPartition> DevPartitions;
            DevPartition Content;
            DevPartition Dash;

            Content.Name = "Data";
            Content.Sector = DeviceStream->ReadUInt32();
            Content.Size = DeviceStream->ReadUInt32();

            DeviceStream->SetPosition(0x10);
            Dash.Name = "Xbox 360 Dashboard";
            Dash.Sector = DeviceStream->ReadUInt32();
            Dash.Size = DeviceStream->ReadUInt32();

            DevPartitions.push_back(Content);
            DevPartitions.push_back(Dash);

            for (int i = 0; i < (int)DevPartitions.size(); i++)
            {
                DevPartition temp = DevPartitions.at(i);
                xVolume *Actual = new xVolume();
                Actual->Offset = (UINT64)temp.Sector * 0x200;
                Actual->Size = temp.Size * 0x200;
                Actual->Name = temp.Name;
                ValidVolumes.push_back(Actual);
            }
        }

        // Thumb drive partitions
        else if (Type == DeviceUsb)
        {
            xVolume* SysExt = new xVolume();
            xVolume* SysAux = new xVolume();
            xVolume* Cache  = new xVolume();
            xVolume* Data   = new xVolume();

            SysExt->Offset = UsbOffsets::SystemExtended;
            SysAux->Offset = UsbOffsets::SystemAux;
            Cache->Offset = UsbOffsets::Cache;
            Data->Offset = UsbOffsets::Data;

            SysExt->Size = UsbSizes::SystemExtended;
            SysAux->Size = UsbSizes::SystemAux;
            Cache->Size = UsbSizes::Cache;
            Data->Size = DeviceStream->Length() - Data->Offset;

            SysExt->Name = "System Extended";
            SysAux->Name = "System Auxiliary";
            Cache->Name = "Cache";
            Data->Name = "Data";

            DeviceStream->SetPosition(SysExt->Offset);
            if (DeviceStream->ReadUInt32() == FatxMagic)
            {
                ValidVolumes.push_back(SysExt);
                ValidVolumes.push_back(SysAux);
                ValidVolumes.push_back(Cache);
                ValidVolumes.push_back(Data);
            }
            else
            {
                Cache->Size = UsbSizes::CacheNoSystem;
                ValidVolumes.push_back(Cache);
                ValidVolumes.push_back(Data);
            }
        }
        // Retail disk/disk backup partitions
        else if(Type == DeviceDisk || Type == DeviceBackup)
        {
            xVolume* SysExt        = new xVolume();
            xVolume* SysAux        = new xVolume();
            xVolume* Compatibility = new xVolume();
            xVolume* Data          = new xVolume();

            SysExt->Offset = HddOffsets::SystemExtended;
            SysAux->Offset = HddOffsets::SystemAux;
            Compatibility->Offset = HddOffsets::Compatibility;
            Data->Offset = HddOffsets::Data;

            SysExt->Size = HddSizes::SystemExtended;
            SysAux->Size = HddSizes::SystemAux;
            Compatibility->Size = HddSizes::Compatibility;
            Data->Size = DeviceStream->Length() - Data->Offset;

            SysExt->Name = "System Extended";
            SysAux->Name = "System Auxiliary";
            Compatibility->Name = "Compatibility";
            Data->Name = "Data";

            ValidVolumes.push_back(SysExt);
            ValidVolumes.push_back(SysAux);
            ValidVolumes.push_back(Compatibility);
            ValidVolumes.push_back(Data);
        }

        for (int i = 0; i < (int)ValidVolumes.size(); i++)
        {
            try
            {
                FatxProcessBootSector(ValidVolumes.at(i));
                xVolume* p = ValidVolumes.at(i);
                p->Root = new Folder();
                p->Root->Volume = p;
                p->Root->FullPath += p->Name;
                p->Root->FatxEntriesRead = false;
                p->Root->Parent = 0;
                memset(p->Root->Dirent.Name, 0, 0x2B);
                sprintf(p->Root->Dirent.Name, "VolumeRoot");
            }
            catch (...)
            {
                delete ValidVolumes.at(i);
                ValidVolumes.erase(ValidVolumes.begin() + i);
                --i;
            }
        }
    }
}

void Drive::InitializePartitions( void )
{
    // First, check to see if the drive has devkit partitions
    if (Type != DeviceUsb)
    {
        DeviceStream->SetPosition(0);
        // Dev hard disks have 0x00020000 at offset 0
        UINT32 Magic = DeviceStream->ReadInt32();
        if (Magic == 0x00020000)
        {
            IsDevKitDrive = true;
        }
        else if (Type == DeviceBackup && Magic != FatxMagic)
        {
            throw xException("Not a valid FATX file.", ExInvalidFile);
        }
    }
    SetValidPartitions();
}

vector<Drive *> Drive::GetFATXDrives( bool HardDisks )
{
    vector<Drive *> Return;
    if (HardDisks)
    {
        vector<DISK_DRIVE_INFORMATION> Disks = GetPhysicalDisks();
        for (int i = 0; i < (int)Disks.size(); i++)
        {
            DISK_DRIVE_INFORMATION ddi = Disks.at(i);
            // First, try reading the disk way
            Streams::xDeviceStream* DS = NULL;
            try
            {
                char path[0x200] = {0};
                wcstombs(path, ddi.Path, wcslen(ddi.Path));
                qDebug("Attempting to open disk at %s", path);
                DS = new Streams::xDeviceStream(ddi.Path);
            }
            catch (xException& e)
            {
                qDebug("Failed to open disk");
                continue;
            }

            if (DS == NULL || DS->Length() == 0 || DS->Length() < HddOffsets::Data)
            {
                qDebug("Disk is not of valid length. Length: %lX", DS->Length());
                continue;
            }
            qDebug("Disk opened successfully.  Length: %lX", DS->Length());
            DS->SetPosition(HddOffsets::Data);

            // Read the FATX partition magic
            int Magic = DS->ReadInt32();
            // Close the stream
            DS->Close();

            // Compare the magic we read to the *actual* FATX magic
            if (Magic == FatxMagic)
            {
                Drive *d = new Drive(Disks.at(i).Path, Disks.at(i).FriendlyName, false);
                Return.push_back(d);
            }
        }
    }

    vector<Drive *> LogicalDisks = GetLogicalPartitions();
    for (int i = 0; i < (int)LogicalDisks.size(); i++)
    {
        Return.push_back(LogicalDisks.at(i));
    }

    return Return;
}

INT64 Drive::GetLength( void )
{
    return DeviceStream->Length();
}

vector<Drive *> Drive::GetLogicalPartitions( void )
{
    vector<Drive *> ReturnVector;

#ifdef __WINDOWS__
    DWORD LettersSize = (26 * 2) + 1;

    TCHAR Letters[(26 * 2) + 1] = {0}; // 26 for every leter, multiplied by 2 for each null byte, plus 1 for the last null

if (!GetLogicalDriveStrings(LettersSize, Letters))
{
    int x = GetLastError();
    return ReturnVector;
}

TCHAR Name[MAX_PATH + 1] = {0};

for (int i = 0; i < 26; i+= 4)
{
    memset(&Name, 0, sizeof(Name));
    GetxVolumeInformation(
                &Letters[i],	// Path name
                Name,			// xVolume name
                MAX_PATH + 1,	// xVolume name length
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);
    TCHAR temp[21] = {0};

    swprintf(temp, 20, L"%s%s", &Letters[i], L"Xbox360\\Data0000");
    try
    {
        Streams::xFileStream tempFile(temp, Streams::Open);
        tempFile.Close();
        // Stream opened with no exception, we're good!
        Drive *d = new Drive(&(Letters[i]), Name, true);
        ReturnVector.push_back(d);
    }
    catch(...)
    {
        // stream opened with an exception, the file doesn't exist (not a valid Xbox 360-formatted drive)
    }
}
#else
// First, enumerate the disks
DIR *dir;
dirent *ent;
dir = opendir("/Volumes/");
if (dir != NULL)
{
    // Read the shit
    while ((ent = readdir(dir)) != NULL)
    {
        DISK_DRIVE_INFORMATION curdir;
        // Set the strings in curdir to null
        memset(curdir.FriendlyName, 0, sizeof(curdir.FriendlyName));
        memset(curdir.Path, 0, sizeof(curdir.Path));

        // Create a temporary buffer to hold our disk name
        char TempName[0x100] = {0};
        sprintf(TempName, "/Volumes/%s/", ent->d_name);
        // Copy that string to the current directory's buffer
        mbstowcs(curdir.Path, TempName, ent->d_namlen + 0xA);
        try
        {
            TCHAR Path[0x100] = {0};
            wcscpy(Path, curdir.Path);
            wcscpy(&(Path[0]) + wcslen(curdir.Path), L"Xbox360/Data0000");
            Streams::xFileStream x(Path, Streams::Open);
            // Stream opened good, close it
            x.Close();
            if (curdir.Path == L"/Volumes/.")
            {
                swprintf(curdir.FriendlyName, wcslen(L"OS Root"), L"OS Root");
            }
            else
            {
                swprintf(curdir.FriendlyName, wcslen(curdir.Path) - 9, &(curdir.Path[0]) + 9);
            }
            Drive *d = new Drive(curdir.Path, curdir.FriendlyName, true);
            ReturnVector.push_back(d);
        }
        catch(...)
        {
            // Couldn't open the device, just continue
        }
    }
    closedir(dir);
}
#endif

return ReturnVector;
}

vector<Drive::DISK_DRIVE_INFORMATION> Drive::GetPhysicalDisks( void )
{
#ifdef __WINDOWS__
    unsigned i;
    DWORD dwSize, dwPropertyRegDataType = SPDRP_PHYSICAL_DEVICE_OBJECT_NAME;
    CONFIGRET r;
    HDEVINFO hDevInfo;
    SP_DEVINFO_DATA DeviceInfoData;
    SP_DEVICE_INTERFACE_DATA interfaceData;

    TCHAR szDeviceInstanceID [MAX_DEVICE_ID_LEN];
    TCHAR szDesc[1024];

    GUID HddClass;
    HddClass = GUID_DEVINTERFACE_DISK;//GUID_DEVCLASS_DISKDRIVE;
#endif

    vector<Drive::DISK_DRIVE_INFORMATION> ReturnVector;

#ifdef __WINDOWS__
    // List all connected disk drives
    hDevInfo = SetupDiGetClassDevs (&HddClass, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return ReturnVector;

    // Find the ones that are driverless
    for (i = 0; ; i++)
    {
        DeviceInfoData.cbSize = sizeof (DeviceInfoData);
        // Get the next device info
        if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInfoData))
            break;
        interfaceData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
        // Get the next device interface
        if (!SetupDiEnumInterfaceDevice(hDevInfo, NULL, &HddClass, i, &interfaceData))
        {
            break;
        }

        // Get the device ID
        r = CM_Get_Device_ID(DeviceInfoData.DevInst, szDeviceInstanceID , MAX_PATH, 0);
        if (r != CR_SUCCESS)
            continue;

        // To add to the vector
        DISK_DRIVE_INFORMATION AddToVector;

        DWORD requiredSize = 0;

        // Get the path
        SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, NULL, NULL, &requiredSize, NULL);
        SP_INTERFACE_DEVICE_DETAIL_DATA* data = (SP_INTERFACE_DEVICE_DETAIL_DATA*) malloc(requiredSize);

        data->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);


        if (!SetupDiGetDeviceInterfaceDetail(hDevInfo, &interfaceData, data, requiredSize, NULL, NULL))
        {
            continue;
        }

        _tcscpy(AddToVector.Path, data->DevicePath);

        // Friendly name (e.g. SanDisk Cruzer USB...)
        SetupDiGetDeviceRegistryProperty (hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME,
                                          &dwPropertyRegDataType, (BYTE*)szDesc,
                                          sizeof(szDesc),   // The size, in bytes
                                          &dwSize);
        _tcscpy(AddToVector.FriendlyName, (TCHAR*)szDesc);

        ReturnVector.push_back(AddToVector);
        delete data;
    }
#else
DIR *dir;
dirent *ent;
dir = opendir("/dev/");
if (dir != NULL)
{
    // Read the shit
    while ((ent = readdir(dir)) != NULL)
    {
        // Check the directory name, and if it starts with "disk" then keep it!
        QRegExp exp("disk*");
        exp.setPatternSyntax(QRegExp::Wildcard);
        exp.setCaseSensitivity(Qt::CaseInsensitive);
        if (exp.exactMatch(ent->d_name))
        {
            DISK_DRIVE_INFORMATION curdir;
            memset(curdir.FriendlyName, 0, sizeof(curdir.FriendlyName));
            memset(curdir.Path, 0, sizeof(curdir.Path));

            char diskPath[0x50] = {0};
            sprintf(diskPath, "/dev/r%s", ent->d_name);

            mbstowcs(curdir.Path, diskPath, strlen(diskPath));

            int device;
            if ((device = open(diskPath, O_RDONLY)) > 0)
            {
#ifdef __linux
                hd_driveid hd;
                if (!ioctl(device, HDIO_GET_IDENTITY, &hd))
                {
                    swprintf(curdir.FriendlyName, strlen(hd) * 2, L"%hs", hd.model);
                }
#elif defined __APPLE__
                swprintf(curdir.FriendlyName, strlen("diskx"), L"diskx");
#endif
                ReturnVector.push_back(curdir);
            }
        }
    }
}
#endif
    return ReturnVector;
}

BYTE Drive::cntlzw(unsigned int val)
{
    vector<BYTE> Consecutives;
    BYTE consec = 0;
    for (unsigned int i = 1; i <= 0x80000000; i = i << 1)
    {
        if ((val & i) == 0)
        {
            consec++;
        }
        else
        {
            Consecutives.push_back(consec);
            consec = 0;
        }
        if (i == 0x80000000)
        {
            break;
        }
    }
    Consecutives.push_back(consec);

    BYTE highest = 0;
    for (int i = 0; i < (int)Consecutives.size(); i++)
    {
        if (Consecutives.at(i) > highest)
        {
            highest = Consecutives.at(i);
        }
    }
    return highest;
}

void Drive::FatxProcessBootSector( xVolume* ref )
{
    // Get the partition size locally
    UINT64 PartitionSize = (INT64)ref->Size;

    DeviceStream->SetPosition(ref->Offset);
    ref->Magic = DeviceStream->ReadUInt32();
    if (ref->Magic != FatxMagic)
    {
        throw xException("Bad magic", ExVolumeInvalidMagic);
    }

    DeviceStream->SetPosition(ref->Offset + 0x8);
    ref->SectorsPerCluster = DeviceStream->ReadUInt32();

    if (ref->SectorsPerCluster != 0x2  && ref->SectorsPerCluster != 0x4  && ref->SectorsPerCluster != 0x8  &&
            ref->SectorsPerCluster != 0x10 && ref->SectorsPerCluster != 0x20 && ref->SectorsPerCluster != 0x40 &&
            ref->SectorsPerCluster != 0x80)
    {
        throw xException("FATX: found invalid sectors per cluster", ExVolumeInvalidSectorsPerCluster);
    }

    DeviceStream->SetPosition(ref->Offset + 0xC);
    ref->RootDirectoryCluster = DeviceStream->ReadUInt32();

    DeviceStream->SetPosition(ref->Offset + 0x4);
    ref->SerialNumber = DeviceStream->ReadUInt32();

    ref->ClusterSize = ref->SectorsPerCluster << 9;
    BYTE ConsecutiveZeroes = cntlzw(ref->ClusterSize);
    int ShiftFactor = 0x1F - ConsecutiveZeroes;

    PartitionSize >>= ShiftFactor;
    PartitionSize++;

    if (PartitionSize < FAT_CLUSTER16_RESERVED)
    {
        ref->FatEntryShift = 1;
    }
    else
    {
        ref->FatEntryShift = 2;
    }
    PartitionSize <<= ref->FatEntryShift;
    PartitionSize += 0x1000;
    PartitionSize--;

    UINT64 Clusters = ref->Size;
    Clusters -= 0x1000;

    PartitionSize &= ~0xFFF;
    PartitionSize &= 0xFFFFFFFF;

    if (Clusters < PartitionSize)
    {
        throw xException("FATX: xVolume too small to hold the FAT", ExVolumeTooSmall);
    }

    Clusters -= PartitionSize;
    Clusters >>= (ShiftFactor & 0xFFFFFFFFFFFFFF);
    if (Clusters > 0xFFFFFFF)
    {
        throw xException("FATX: too many clusters", ExVolumeTooManyClusters);
    }

    ref->Clusters = Clusters;
    ref->AllocationTableSize = PartitionSize;
    ref->EntrySize = (Clusters < FAT_CLUSTER16_RESERVED) ? 2 : 4;
    ref->DataStart = ref->Offset + 0x1000 + PartitionSize;
}
