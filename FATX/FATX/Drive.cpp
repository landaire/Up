#include <sys/stat.h>
#include <QMetaType>
#include <QTime>
#include "Drive.h"
#include "stfspackage.h"
#include "xexception.h"

Drive::Drive( string path, string friendlyName, bool isUsb ) : QObject()
{
    IsDevKitDrive = false;
    this->FriendlyName = "";
    if (!isUsb)
    {
        qDebug("This is a disk.  It is nice.");
        deviceStream = new Streams::xdeviceStream(path);
        Type = DeviceDisk;
    }
    else
    {
        // USB
#ifdef _WIN32
        QRegExp qr("*:\\");
#else
        QRegExp qr("/Volumes/*");
#endif
        qr.setPatternSyntax(QRegExp::Wildcard);
        qr.setCaseSensitivity(Qt::CaseInsensitive);
        if (qr.exactMatch(QString::fromStdString(path)))
        {
            // Find all valid Xbox 360 files
            vector<string> paths;
            string filePath;
            for (int i = 0; i < 1000; i++)
            {

                if (i < 10)
                {
                    filePath = "Xbox360/Data000";
                }
                else if (i < 100)
                {
                    filePath = "Xbox360/Data00";
                }
                else if (i < 1000)
                {
                    filePath = "Xbox360/Data0";
                }
                stringstream pathStream;
                pathStream << path;
                pathStream << filePath;
                pathStream << i;
                filePath = pathStream.str();

                try
                {
                    Streams::FileStream temp(filePath, Streams::Open);
                    // File opened with no exceptions thrown, close the stream and add the path to the vector
                    temp.Close();
                    paths.push_back(filePath);
                }
                catch (...)
                {
                    // file's not good, break the loop
                    break;
                }
            }
            if (paths.size() == 0)
            {
                qDebug("Exception thrown at Drive: No paths for device");
                throw FatxException("No paths found for device!");
            }
            deviceStream = new Streams::MultiFileStream(paths);
            Type = DeviceUsb;
        }
        // Backup
        else
        {
            deviceStream = new Streams::FileStream(path, Streams::Open);
            Type = DeviceBackup;
        }
    }
    this->FriendlyName = friendlyName;
    this->FriendlySize = Helpers::ConvertToFriendlySize(GetLength());

    InitializePartitions();
}

Drive::~Drive(void)
{

}

void Drive::CreateDirent(const xDirent &d)
{
    // Get the stream to the disk
    Streams::IStream *ds = this->deviceStream;
    // Go to the offset given by the dirent
    ds->SetPosition(d.Offset);
    // Write out the attributes
    ds->WriteByte(d.Attributes);
    // Write out the filename size
    ds->WriteByte(d.NameSize);
    // Write out the filename
    ds->Write((uint8_t*)(&d.Name), d.NameSize);
    // Write out 0xFF if the size isn't 0x2A.  This helps when people want
    // to undelete files, because we just go until either 0x2A or 0xFF
    if (d.NameSize != 0x2A)
        ds->WriteByte(0xFF);
    // Go to where strings usually end (0x2C)
    ds->SetPosition(0x2C);
    // Write out the cluster start
    ds->WriteInt32(d.ClusterStart);
    // Write out the dates
    ds->WriteInt32(d.DateCreated.AsDWORD);
    ds->WriteInt32(d.DateLastWritten.AsDWORD);
    ds->WriteInt32(d.DateAccessed.AsDWORD);
}

void Drive::CopyFileToLocalDisk(File *dest, string const &Output)
{
    // Get the stream to the file
    std::unique_ptr<DeviceFileStream> inputFileStream(new Streams::DeviceFileStream(dest, this));
    // Get a stream to the output file
    std::unique_ptr<FileStream> outputStream(new Streams::FileStream(Output, Streams::Create));

    uint64_t size = inputFileStream->Length();
    uint8_t *buffer = new uint8_t[0x10000];
    memset(buffer, 0, 0x10000);

    Progress p;
    // Set up our progress
    p.Done = false;
    p.Maximum = Helpers::UpToNearestX(dest->Dirent.FileSize, dest->Volume->ClusterSize) / 0x4000;
    if (p.Maximum == 0)
        p.Maximum++;
    p.Current = 0;
    p.FilePath = dest->FullPath;
    p.Device = this;
    p.FileName = string(dest->Dirent.Name);

    // Get the package's STFS information
    StfsPackage pack(inputFileStream);
    p.IsStfsPackage = pack.IsStfsPackage();
    if (p.IsStfsPackage)
    {
        p.PackageImage = pack.ThumbnailImage();
        p.PackageName = pack.DisplayName();
    }
    emit FileProgressChanged(p);

    // Diagnostics and shit
    QTime qt;
    qt.setHMS(0, 0, 0, 0);
    qt.start();

    inputFileStream->SetPosition(0);
    // Start the reading
    while (size > dest->Volume->SectorsPerCluster * 0x200)
    {
        uint64_t read = 0;
        if (size >= 0x10000)
            read += inputFileStream->Read(buffer, 0x10000);
        else
            read += inputFileStream->Read(buffer, dest->Volume->SectorsPerCluster * 0x200);

        outputStream->Write(buffer, read);

        size -= read;
        p.Current += (read / (dest->Volume->SectorsPerCluster * 0x200));
        emit FileProgressChanged(p);
    }
    inputFileStream->SetPosition(inputFileStream->Length() - size);
    // Read the last section of data
    inputFileStream->Read(buffer, size);
    outputStream->Write(buffer, size);

    int Milliseconds = qt.elapsed();

    int Hours = Milliseconds / (1000*60*60);
    int Minutes = (Milliseconds % (1000*60*60)) / (1000*60);
    int Seconds = ((Milliseconds % (1000*60*60)) % (1000*60)) / 1000;

    qDebug("%d:%d:%d", Hours, Minutes, Seconds);
    p.Current++;
    p.Done = true;
    emit FileProgressChanged(p);

    inputFileStream->Close();
    inputFileStream.release();
    outputStream->Close();
    delete outputStream;
    delete buffer;
}

void Drive::CopyFileToLocalDisk(string const &path, string const &outputDirectory)
{
    CopyFileToLocalDisk(FileFromPath(path), outputDirectory);
}

void Drive::CopyFolderToLocalDisk(Folder *folder, string const &outputDirectory)
{
    // Make the directory...
#ifndef _WIN32
    qDebug((((outputDirectory + "/") + folder->Dirent.Name).c_str()));
    mkdir(((outputDirectory + "/") + folder->Dirent.Name).c_str(), 0777);
#else
    CreateDirectory(nowide::convert((outputDirectory + "/") + folder->Dirent.Name).c_str(), NULL);
#endif
    string path = ((outputDirectory + "/") + folder->Dirent.Name);
    // Cleanup the path string
    {
        size_t pos = 0;
#ifdef _WIN32
        while((pos = path.find("\\/", pos)) != string::npos)
#else
        while((pos = path.find("//", pos)) != string::npos)
#endif
        {
#ifdef _WIN32
            path.replace(pos, 2, "\\");
#else
            path.replace(pos, 2, "/");
#endif
            pos += 1;
        }
    }
    // Make sure that this folder's dirents are read...
    if (!folder->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(folder);

    // For each folder, do an extract thingy
    for (auto subFolder : folder->CachedFolders) {
        CopyFolderToLocalDisk(subFolder, path);
    }

    // Extract all the files from this dir
    for (auto file : folder->CachedFiles) {
        CopyFileToLocalDisk(file, (path + "/") + file->Dirent.Name);
    }
}

void Drive::CopyFolderToLocalDisk(string const &folderPath, string const &outputFolderPath)
{
    CopyFolderToLocalDisk(FolderFromPath(folderPath), outputFolderPath);
}

size_t Drive::GetFileCount(shared_ptr<Folder> *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(f);

    // Return file vector size
    return f->CachedFiles.size();
}

size_t Drive::GetTotalFileCount(shared_ptr<Folder> *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read them!
        ReadDirectoryEntries(f);

    size_t totalCount = f->CachedFiles.size();

    // For each one of the folders within this folder, add that to our total count
    for (size_t i = 0; i < f->CachedFolders.size(); i++)
        totalCount += GetTotalFileCount(f->CachedFolders.at(i));

    // Return the total count
    return totalCount;
}

size_t Drive::GetFolderCount(shared_ptr<Folder> folder)
{
    // If we haven't yet read the dirents for this folder...
    if (!folder->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(folder);

    // Return file vector size
    return folder->CachedFolders.size();
}

size_t Drive::GetTotalFolderCount(shared_ptr<Folder> folder)
{
    // If we haven't yet read the dirents for this folder...
    if (!folder->FatxEntriesRead)
        // Read them!
        ReadDirectoryEntries(folder);

    size_t totalCount = folder->CachedFolders.size();

    // For each one of the folders within this folder, add that to our total count
    for (size_t i = 0; i < folder->CachedFolders.size(); i++)
        totalCount += GetTotalFolderCount(folder->CachedFolders.at(i));

    // Return the total count
    return totalCount;
}

void Drive::DestroyFolder(shared_ptr<Folder> directory)
{
    while(directory->CachedFiles.size())
    {
        unique_ptr<File> f = directory->CachedFiles.at(0);
        f.release();
        directory->CachedFiles.erase(directory->CachedFiles.begin());
    }

    while(directory->CachedFolders.size())
    {
        unqiue_ptr<Folder> f = directory->CachedFolders.at(0);
        DestroyFolder(f);
        directory->CachedFolders.erase(directory->CachedFolders.begin());
    }
    directory.release();
}

QString Drive::GetDiskName( void )
{
    // Get the Data/name.txt file
    auto nameFile = FileFromPath(string("Data/name.txt"));
    // Open a stream to that file
    std::unique_ptr<DeviceFileStream> fs(new Streams::DeviceFileStream(nameFile, this));

    // Skip the first two bytes -- I have no idea what they're there for
    fs->SetPosition(2);

    // Create a new buffer to hold the name
    uint8_t charArray[0x50] = {0};
    // Read the name
    fs->Read((uint8_t*)&charArray, fs->Length() - 2);

    for (int i = 0; i < fs->Length() - 2; i+=2)
        fs->DetermineAndDoEndianSwap((uint8_t*)&charArray + i, sizeof(short), sizeof(char));

    fs->Close();
    fs.release();
    return QString::fromUtf16((const ushort*)&charArray);
}

shared_ptr<Folder> Drive::FolderFromPath(string path)
{
    string cmp = path.substr(0, path.find('/'));
    if (cmp == FriendlyName)
        path = path.substr(path.find('/') + 1);

    for (size_t i = 0; i < validVolumes.size(); i++)
    {
        FatxVolume* activePartition = validVolumes.at(i);

        // Split the path so by the backslash
        vector<string> pathSplit;
        Helpers::split(path, '/', pathSplit);

        // Match the partition name to that of the one we were given
        QRegExp reg(pathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        reg.setPatternSyntax(QRegExp::FixedString);
        if (reg.exactMatch(activePartition->Name.c_str()))
        {
            // We've found the partition, now get the root folder
            auto currentFolder = activePartition->Root;
            currentFolder->Dirent.ClusterStart = activePartition->RootDirectoryCluster;
            do
            {
                pathSplit.erase(pathSplit.begin());
                if (!currentFolder->FatxEntriesRead)
                {
                    ReadDirectoryEntries(currentFolder);
                }
                if (!pathSplit.size() )
                {
                    break;
                }
                bool Found = false;

                for (size_t j = 0; j < currentFolder->CachedFolders.size(); j++)
                {
                    auto file = currentFolder->CachedFolders.at(j);

                    QRegExp fReg(pathSplit.at(0).c_str());
                    fReg.setCaseSensitivity(Qt::CaseInsensitive);
                    fReg.setPatternSyntax(QRegExp::FixedString);

                    if (fReg.exactMatch(file->Dirent.Name))
                    {
                        Found = true;
                        currentFolder = file;
                        break;
                    }
                }
                if (!Found)
                {
                    qDebug("Exception thrown at FolderFromPath: Folder not found");
                    throw FatxException("Folder not found");
                }
            }
            while (pathSplit.size() > 0);
            return shared_ptr(currentFolder);
        }
    }
    qDebug("Exception thrown at FolderFromPath: Folder not found");
    throw FatxException("Folder not found");
}

shared_ptr<File> Drive::FileFromPath(string path)
{
    string cmp = path.substr(0, path.find('/'));
    // Because I'm stupid I need to convert
    if (cmp == FriendlyName)
        path = path.substr(path.find('/') + 1);
    // Loop through each volume
    for (size_t i = 0; i < validVolumes.size(); i++)
    {
        auto activePartition = validVolumes.at(i);

        // Split the path so by the backslash
        vector<string> pathSplit;
        Helpers::split(path, '/', pathSplit);

        // Match the partition name to that of the one we were given
        QRegExp reg(pathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        reg.setPatternSyntax(QRegExp::FixedString);

        if (reg.exactMatch(activePartition->Name.c_str()))
        {
            // We've found the partition, now get the root folder
            auto currentFolder = activePartition->Root;
            currentFolder->Dirent.ClusterStart = activePartition->RootDirectoryCluster;
            do
            {
                // Remove this index
                pathSplit.erase(pathSplit.begin());

                // If there's nothing left to find, break the loop
                if (!pathSplit.size())
                {
                    break;
                }

                // If the dirents haven't been read
                if (!currentFolder->FatxEntriesRead)
                {
                    // Read the dirents
                    ReadDirectoryEntries(currentFolder);
                }

                bool Found = false;
                if (pathSplit.size() > 1)
                {
                    for (int j = 0; j < (int)currentFolder->CachedFolders.size(); j++)
                    {
                        // Get the subfolder
                        auto subFolder = currentFolder->CachedFolders.at(j);

                        // Try to match the name to whatever we're looking for
                        QRegExp fReg(pathSplit.at(0).c_str());
                        fReg.setCaseSensitivity(Qt::CaseInsensitive);
                        fReg.setPatternSyntax(QRegExp::FixedString);

                        if (fReg.exactMatch(subFolder->Dirent.Name))
                        {
                            Found = true;
                            currentFolder = subFolder;
                            break;
                        }
                    }
                }
                else
                {
                    for (size_t j = 0; j < currentFolder->CachedFiles.size(); j++)
                    {
                        // Get the current file
                        auto file = currentFolder->CachedFiles.at(j);

                        // Try to make a match!
                        QRegExp fReg(pathSplit.at(0).c_str());
                        fReg.setCaseSensitivity(Qt::CaseInsensitive);
                        fReg.setPatternSyntax(QRegExp::FixedString);

                        if (fReg.exactMatch(file->Dirent.Name))
                        {
                            return file;
                        }
                    }
                }
                if (!Found)
                {
                    qDebug("Exception thrown at FileFromPath: File not found");
                    throw FatxException("File not found");
                }
            }
            while (pathSplit.size() > 0);
        }
    }
    qDebug("Exception thrown at FileFromPath: File not found");
    throw FatxException("Folder not found");
}


// TODO: FINISH THIS FUNCTION AND MAKE SHIT WORK
void Drive::FindFreeClusters(uint32_t startingCluster, size_t clusterCount, const shared_ptr<FatxVolume> partition, vector<uint32_t>& outChain)
{
    // Set the stream position to the free cluster range start
    if (partition->FreeClusterRangeStart != 0xFFFFFFFF)
        deviceStream->SetPosition(partition->FreeClusterRangeStart);
    else
    {
        // Go to the FAT start
        deviceStream->SetPosition(partition->Offset + 0x1000);
        // Start reading data to see when we hit a free cluster
        if (partition->EntrySize == FAT32)
            // Keep looping until we hit a free cluster
            while (deviceStream->ReadUInt32() != FAT_CLUSTER_AVAILABLE);
        else
            // Keep looping until we hit a free cluster
            while(deviceStream->ReadUInt16() != FAT_CLUSTER16_AVAILABLE);
    }
}

void Drive::ReadDirectoryEntries(shard_ptr<Folder> directory)
{
    // If the cluster chain size is 0, we haven't read the cluster chain yet.  We better do that
    if (!directory->ClusterChain.size())
    {
        ReadClusterChain(directory->ClusterChain, directory->Dirent, *(directory->Volume));
    }
    // Loop for each cluster
    for (int i = 0; i < (int)directory->ClusterChain.size(); i++)
    {
        // Set the IO's position to the start of the cluster
        deviceStream->SetPosition(directory->Volume->DataStart +
                        ((uint64_t)(directory->ClusterChain.at(i) - 1)   * directory->Volume->ClusterSize));
        // Loop for the maximum amount of entries per cluster
        for (size_t j = 0; j < directory->Volume->ClusterSize / 0x40; j++)
        {
            xDirent entry;
            memset(&entry, 0, sizeof(entry));
            entry.Offset = deviceStream->Position();
            // Would just read the entire header here, but since there's the endian swap...
            entry.NameSize = deviceStream->ReadByte();

            // For now, I'm not messing with deleted stuff
            if (entry.NameSize == FAT_DIRENT_DELETED)
            {
                deviceStream->SetPosition(deviceStream->Position() + 0x3F);
                continue;
            }
            if (entry.NameSize == FAT_DIRENT_NEVER_USED || entry.NameSize == FAT_DIRENT_NEVER_USED2)
            {
                break;
            }

            entry.Attributes = deviceStream->ReadByte();
            deviceStream->Read((uint8_t*)&entry.Name, entry.NameSize);

            deviceStream->SetPosition((deviceStream->Position() + 0x2A) - entry.NameSize);

            entry.ClusterStart              = deviceStream->ReadUInt32();
            entry.FileSize                  = deviceStream->ReadUInt32();
            entry.DateCreated.AsDWORD       = deviceStream->ReadUInt32();
            entry.DateLastWritten.AsDWORD   = deviceStream->ReadUInt32();
            entry.DateAccessed.AsDWORD      = deviceStream->ReadUInt32();

            // All of that's done, now determine what type of entry it is
            if (entry.Attributes & Attributes::DIRECTORY)
            {
                // Folder, sweet.
                shared_ptr<Folder> f(new Folder());
                f->Dirent           = entry;
                f->FullPath         = directory->FullPath + "/";
                f->FullPath         += entry.Name;
                f->Parent           = directory;
                f->FatxEntriesRead  = false;
                f->Volume           = directory->Volume;
                f->DateAccessed     = Helpers::IntToQDateTime(entry.DateLastWritten);
                f->DateCreated      = Helpers::IntToQDateTime(entry.DateCreated);
                f->DateModified     = Helpers::IntToQDateTime(entry.DateAccessed);
                directory->CachedFolders.push_back(f);
            }
            else
            {
                shared_ptr<File> f(new File());
                f->Dirent       = entry;
                f->FullPath     = directory->FullPath + "/";
                f->FullPath     += entry.Name;
                f->Parent       = directory;
                f->Volume       = directory->Volume;
                f->DateAccessed = Helpers::IntToQDateTime(entry.DateLastWritten);
                f->DateCreated  = Helpers::IntToQDateTime(entry.DateCreated);
                f->DateModified = Helpers::IntToQDateTime(entry.DateAccessed);
                f->FullClusterChainRead = false;
                directory->CachedFiles.push_back(f);
            }
        }
    }
    directory->FatxEntriesRead = true;
}

void Drive::ReadClusterChain(vector<uint32_t>& chain, xDirent entry, const shared_ptr<FatxVolume> relativePartition, int count)
{
    // Clear the chain
    chain.clear();
    // The int that signifies end of cluster chain
    uint32_t end = (relativePartition.EntrySize == 2) ? FAT_CLUSTER16_LAST : FAT_CLUSTER_LAST;
    // The previous cluster
    uint32_t previous = entry.ClusterStart;
    // Add the base cluster to the chain
    chain.push_back(previous);
    // Loop
    do
    {
        // Set the IO to the FAT index
        deviceStream->SetPosition((relativePartition.Offset + 0x1000) + (uint64_t)(previous * relativePartition.EntrySize));
        // Read the int there
        if (relativePartition.EntrySize == 2)
        {
            previous = static_cast<uint32_t>(deviceStream->ReadUInt16());
        }
        else
        {
            previous = deviceStream->ReadUInt32();
        }
        // If we haven't reached the end of the chain, add the previous cluster to our chain
        if (previous != end)
        {
            chain.push_back(previous);
        }
    }
    while(previous != FAT_CLUSTER_AVAILABLE && previous != end && chain.size() != count);

    if (previous == FAT_CLUSTER_AVAILABLE)
    {
        std::string message;
        message << "Free block referenced in FAT cluster chain! Dirent offset: 0x" << std::hex << entry.Offset;
        qDebug("Exception thrown at ReadClusterChain: Free block");
        throw FatxException(message);
    }
}

void Drive::Close( void )
{
    qDebug("Closing disk, destroying volumes");
    if (deviceStream)
    {
        deviceStream->Close();
        delete deviceStream;
    }
    while (validVolumes.size())
    {
        if (!validVolumes.size())
            break;
        FatxVolume *x = validVolumes.at(0);
        DestroyFolder(x->Root);
        validVolumes.erase(validVolumes.begin());
        delete x;
    }
    delete validVolumes;
}

vector<string> Drive::Partitions( void )
{
    if (!_partitions.size())
    {
        for (int i = 0; i < (int)validVolumes.size(); i++)
        {
            _partitions.push_back(validVolumes.at(i)->Name);
        }
    }
    return _partitions;
}

uint64_t Drive::PartitionGetLength( string Partition )
{
    for (int i = 0; i < (int)validVolumes.size(); i++)
    {
        QRegExp rgx(Partition.c_str());
        rgx.setCaseSensitivity(Qt::CaseInsensitive);
        rgx.setPatternSyntax(QRegExp::FixedString);
        if (rgx.exactMatch(validVolumes.at(i)->Name.c_str()))
        {
            return validVolumes.at(i)->Size;
        }
    }
    qDebug("Exception thrown at PartitionGetLength: Partition not found");
    throw FatxException("Partition not found!");
}

void Drive::SetValidPartitions( void )
{
    if (!validVolumes)
    {
        vector<FatxVolume *> DiskVolumes;
        // Get the partitions

        // Dev kit partitions
        if (IsDevKitDrive)
        {
            deviceStream->SetPosition(8);
            vector<DevPartition> DevPartitions;
            DevPartition Content;
            DevPartition Dash;

            Content.Name = "Data";
            Content.Sector = deviceStream->ReadUInt32();
            Content.Size = deviceStream->ReadUInt32();

            deviceStream->SetPosition(0x10);
            Dash.Name = "System Partition";
            Dash.Sector = deviceStream->ReadUInt32();
            Dash.Size = deviceStream->ReadUInt32();

            DevPartitions.push_back(Dash);
            DevPartitions.push_back(Content);

            for (int i = 0; i < (int)DevPartitions.size(); i++)
            {
                DevPartition temp = DevPartitions.at(i);
                FatxVolume *Actual = new FatxVolume();
                Actual->Offset = (uint64_t)temp.Sector * 0x200;
                Actual->Size = temp.Size * 0x200;
                Actual->Name = temp.Name;
                DiskVolumes.push_back(Actual);
            }
        }

        // Thumb drive partitions
        else if (Type == DeviceUsb)
        {
            FatxVolume* SysExt = new FatxVolume();
            FatxVolume* SysAux = new FatxVolume();
            FatxVolume* Cache  = new FatxVolume();
            FatxVolume* Data   = new FatxVolume();

            SysExt->Offset = UsbOffsets::SystemExtended;
            SysAux->Offset = UsbOffsets::SystemAux;
            Cache->Offset = UsbOffsets::Cache;
            Data->Offset = UsbOffsets::Data;

            SysExt->Size = UsbSizes::SystemExtended;
            SysAux->Size = UsbSizes::SystemAux;
            Cache->Size = UsbSizes::Cache;
            Data->Size = (deviceStream->Length() - Data->Offset);

            SysExt->Name = "System Extended";
            SysAux->Name = "System Auxiliary";
            Cache->Name = "Cache";
            Data->Name = "Data";

            SysExt->Disk = this;
            SysAux->Disk = this;
            Cache->Disk = this;
            Data->Disk = this;

            deviceStream->SetPosition(SysExt->Offset);
            if (deviceStream->ReadUInt32() == FatxMagic)
            {
                DiskVolumes.push_back(SysExt);
                DiskVolumes.push_back(SysAux);
                DiskVolumes.push_back(Cache);
                DiskVolumes.push_back(Data);
            }
            else
            {
                Cache->Size = UsbSizes::CacheNoSystem;
                DiskVolumes.push_back(Cache);
                DiskVolumes.push_back(Data);

                delete SysExt;
                delete SysAux;
            }
        }
        // Retail disk/disk backup partitions
        else if(Type == DeviceDisk || Type == DeviceBackup)
        {
            FatxVolume* SysExt        = new FatxVolume();
            FatxVolume* SysAux        = new FatxVolume();
            FatxVolume* SystemPartition = new FatxVolume();
            FatxVolume* Data          = new FatxVolume();

            SysExt->Offset = HddOffsets::SystemExtended;
            SysAux->Offset = HddOffsets::SystemAux;
            SystemPartition->Offset = HddOffsets::SystemPartition;
            Data->Offset = HddOffsets::Data;

            SysExt->Size = HddSizes::SystemExtended;
            SysAux->Size = HddSizes::SystemAux;
            SystemPartition->Size = HddSizes::SystemPartition;
            Data->Size = deviceStream->Length() - Data->Offset;

            SysExt->Name = "System Extended";
            SysAux->Name = "System Auxiliary";
            SystemPartition->Name = "System Partition";
            Data->Name = "Data";

            DiskVolumes.push_back(SysExt);
            DiskVolumes.push_back(SysAux);
            DiskVolumes.push_back(SystemPartition);
            DiskVolumes.push_back(Data);
        }


        for (int i = 0; i < (int)DiskVolumes.size(); i++)
        {
            try
            {
                FatxProcessBootSector(DiskVolumes.at(i));
                FatxVolume* p = DiskVolumes.at(i);
                p->Root = new Folder();
                p->Root->Volume = p;
                p->Root->FullPath += p->Name;
                p->Root->FatxEntriesRead = false;
                p->Root->Parent = 0;
                p->Root->Dirent.ClusterStart = p->RootDirectoryCluster;
                p->Root->Dirent.Offset = p->DataStart;

                memset(p->Root->Dirent.Name, 0, 0x2B);
                sprintf(p->Root->Dirent.Name, "VolumeRoot");

                try
                {
                    ReadClusterChain(p->Root->ClusterChain, p->Root->Dirent, *p);
                }
                catch(...)
                {
                    throw FatxException("Bad partition FAT chain");
                }
            }
            catch (...)
            {
                qDebug("Freeing bad volume");
                delete DiskVolumes.at(i);
                DiskVolumes.erase(DiskVolumes.begin() + i);
                --i;
            }
        }

        validVolumes = new vector<FatxVolume *>();
        *validVolumes = DiskVolumes;
    }
}

void Drive::InitializePartitions( void )
{
    // First, check to see if the drive has devkit partitions
    if (Type != DeviceUsb)
    {
        deviceStream->SetPosition(0);
        // Dev hard disks have 0x00020000 at offset 0
        uint32_t Magic = deviceStream->ReadInt32();
        if (Magic == 0x00020000)
        {
            IsDevKitDrive = true;
        }
        else if (Type == DeviceBackup && Magic != FatxMagic)
        {
            qDebug("Exception thrown at InitializePartitions: Not a valid FATX file");
            throw FatxException("Not a valid FATX file.");
        }
    }
    SetValidPartitions();
}

uint64_t Drive::GetLength( void )
{
    if (Type == DeviceDisk && !IsDevKitDrive)
        return ((uint64_t)Drive::GetSectorsFromSecuritySector()) * 0x200LL;
    return deviceStream->Length();
}

uint32_t Drive::GetSectorsFromSecuritySector( void )
{
    static uint32_t s_sectors = 0;
    qDebug("s_sectors: 0x%LX", s_sectors);
    qDebug("s_sectors * 0x200: 0x%LX", (uint64_t)s_sectors * 0x200LL);
    if (s_sectors != 0)
        return s_sectors;

    // Set the stream position to the location of the security sector + 0x58 for the number of sectors
    deviceStream->SetPosition(HddOffsets::SecuritySector + 0x58);
    // The integer here is little-endian, so switch to little-endian on our IO
    deviceStream->SetEndianness(Streams::Little);
    s_sectors = deviceStream->ReadUInt32();
    deviceStream->SetEndianness(Streams::Big);
    qDebug("s_sectors: 0x%LX", s_sectors);
    qDebug("s_sectors * 0x200: 0x%LX", (uint64_t)s_sectors * 0x200LL);
    return s_sectors;
}

// Count leading zeroes word (but, we use a DWORD :) )
uint8_t Drive::cntlzw(uint32_t val)
{
    vector<uint8_t> Consecutives;
    uint8_t consec = 0;
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

    uint8_t highest = 0;
    for (int i = 0; i < (int)Consecutives.size(); i++)
    {
        if (Consecutives.at(i) > highest)
        {
            highest = Consecutives.at(i);
        }
    }
    return highest;
}

void Drive::FatxProcessBootSector( FatxVolume* ref )
{
    // Get the partition size locally
    uint64_t PartitionSize = (uint64_t)ref->Size;

    deviceStream->SetPosition(ref->Offset);
    ref->Magic = deviceStream->ReadUInt32();
    if (ref->Magic != FatxMagic)
    {
        qDebug("Exception thrown at FatxProcessBotSector: Invalid magic");
        throw FatxException("Bad magic");
    }

    deviceStream->SetPosition(ref->Offset + 0x8);
    ref->SectorsPerCluster = deviceStream->ReadUInt32();

    switch(ref->SectorsPerCluster)
    {
    case 0x2:
    case 0x4:
    case 0x8:
    case 0x10:
    case 0x20:
    case 0x40:
    case 0x80:
        break;
    default:
        qDebug("Exception thrown at FatxProcessBotSector: Invalid sectors per cluster");
        throw FatxException("FATX: found invalid sectors per cluster");
    }

    deviceStream->SetPosition(ref->Offset + 0xC);
    ref->RootDirectoryCluster = deviceStream->ReadUInt32();

    deviceStream->SetPosition(ref->Offset + 0x4);
    ref->SerialNumber = deviceStream->ReadUInt32();

    ref->ClusterSize = ref->SectorsPerCluster << 9;
    uint8_t ConsecutiveZeroes = cntlzw(ref->ClusterSize);
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

    uint64_t Clusters = ref->Size;
    Clusters -= 0x1000;

    PartitionSize &= ~0xFFF;
    PartitionSize &= 0xFFFFFFFF;

    if (Clusters < PartitionSize)
    {
        qDebug("Exception thrown at FatxProcessBotSector: Volume too small to hold FAT");
        throw FatxException("FATX: Volume too small to hold the FAT");
    }

    Clusters -= PartitionSize;
    Clusters >>= (ShiftFactor & 0xFFFFFFFFFFFFFF);
    if (Clusters > 0xFFFFFFF)
    {
        qDebug("Exception thrown at FatxProcessBotSector: Too many clusters");
        throw FatxException("FATX: too many clusters");
    }

    ref->Clusters = Clusters;
    ref->AllocationTableSize = PartitionSize;
    ref->EntrySize = (Clusters < FAT_CLUSTER16_RESERVED) ? FAT16 : FAT32;
    ref->DataStart = ref->Offset + 0x1000 + PartitionSize;

    ref->Disk = this;
}
