#include "StdAfx.h"
#include "Drive.h"
#include "../IO/xDeviceFileStream.h"
#include <QMetaType>
#include "stfspackage.h"
#include <sys/stat.h>
#include <QTime>

using namespace std;

Drive::Drive( TCHAR* Path, TCHAR* FriendlyName, bool IsUsb ) : QObject()
{
    ValidVolumes = 0;
    IsDevKitDrive = false;
    this->FriendlyName = L"";
    if (!IsUsb)
    {
        qDebug("This is a disk.  It is nice.");
        DeviceStream = new Streams::xDeviceStream(Path);
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
                qDebug("Exception thrown at Drive: No paths for device");
                throw xException("No paths found for device!");
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

void Drive::CreateDirent(const xDirent &d)
{
    // Get the stream to the disk
    Streams::IStream *ds = this->DeviceStream;
    // Go to the offset given by the dirent
    ds->SetPosition(d.Offset);
    // Write out the attributes
    ds->WriteByte(d.Attributes);
    // Write out the filename size
    ds->WriteByte(d.NameSize);
    // Write out the filename
    ds->Write((BYTE*)(&d.Name), d.NameSize);
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

void Drive::CopyFileToLocalDisk(File *dest, const string &Output)
{
    // Get the stream to the file
    Streams::xDeviceFileStream *xf = new Streams::xDeviceFileStream(dest, this);
#ifdef _WIN32
    TCHAR path[MAX_PATH + 1];
    mbstowcs(path, Output.c_str(), Output.size());
#else
    const char* path = Output.c_str();
#endif
    // Get a stream to the output file
    Streams::xFileStream *output = new Streams::xFileStream(path, Streams::Create);
    UINT64 size = xf->Length();
    BYTE *Buffer = new BYTE[0x10000];
    memset(Buffer, 0, 0x10000);

    Progress p;
    // Set up our progress
    p.Done = false;
    p.Maximum = Helpers::UpToNearestX(dest->Dirent.FileSize, dest->Volume->ClusterSize) / 0x4000;
    if (p.Maximum == 0)
        p.Maximum++;
    p.Current = 0;
    p.FilePath = dest->FullPath;
    p.Device = this;
    p.FileName = std::string(dest->Dirent.Name);

    // Get the package's STFS information
    StfsPackage pack(xf);
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

    xf->SetPosition(0);
    // Start the reading
    while (size > dest->Volume->SectorsPerCluster * 0x200)
    {
        INT64 read = 0;
        if (size >= 0x10000)
            read += xf->Read(Buffer, 0x10000);
        else
            read += xf->Read(Buffer, dest->Volume->SectorsPerCluster * 0x200);

        output->Write(Buffer, read);

        size -= read;
        p.Current += (read / (dest->Volume->SectorsPerCluster * 0x200));
        emit FileProgressChanged(p);
    }
    xf->SetPosition(xf->Length() - size);
    // Read the last section of data
    xf->Read(Buffer, size);
    output->Write(Buffer, size);

    int Milliseconds = qt.elapsed();

    int Hours = Milliseconds / (1000*60*60);
    int Minutes = (Milliseconds % (1000*60*60)) / (1000*60);
    int Seconds = ((Milliseconds % (1000*60*60)) % (1000*60)) / 1000;

    qDebug("%d:%d:%d", Hours, Minutes, Seconds);
    p.Current++;
    p.Done = true;
    emit FileProgressChanged(p);

    xf->Close();
    output->Close();
    delete xf;
    delete output;
    delete Buffer;
}

void Drive::CopyFileToLocalDisk(const string &Path, const string &Output)
{
    CopyFileToLocalDisk(FileFromPath(Path), Output);
}

void Drive::CopyFolderToLocalDisk(Folder *f, const std::string &Output)
{
    // Make the directory...
#ifndef _WIN32
    qDebug((((Output + "/") + f->Dirent.Name).c_str()));
    mkdir(((Output + "/") + f->Dirent.Name).c_str(), 0777);
#else
    CreateDirectory(nowide::convert((Output + "/") + f->Dirent.Name).c_str(), NULL);
#endif
    std::string Path = ((Output + "/") + f->Dirent.Name);
    // Cleanup the path string
    {
        size_t pos = 0;
#ifdef _WIN32
        while((pos = Path.find("\\/", pos)) != std::string::npos)
#else
        while((pos = Path.find("//", pos)) != std::string::npos)
#endif
        {
#ifdef _WIN32
            Path.replace(pos, 2, "\\");
#else
            Path.replace(pos, 2, "/");
#endif
            pos += 1;
        }
    }
    // Make sure that this folder's dirents are read...
    if (!f->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(f);

    // For each folder, do an extract thingy
    for (int i = 0; i < f->CachedFolders.size(); i++)
    {
        Folder *folder = f->CachedFolders[i];
        CopyFolderToLocalDisk(folder, Path);
    }

    // Extract all the files from this dir
    // FUCK MAN ARE YOU KIDDING ME?  I CAN'T LIKE... USE C++11 RANGE-BASED LOOPS?!  FUCK MAN
    for (int i = 0; i < f->CachedFiles.size(); i++)
    {
        File *file = f->CachedFiles[i];
        CopyFileToLocalDisk(file, (Path + "/") + file->Dirent.Name);
    }
}

void Drive::CopyFolderToLocalDisk(const std::string &Path, const std::string &Output)
{
    // Get the folder
    Folder *f = FolderFromPath(Path);
    CopyFolderToLocalDisk(f, Output);
}

DWORD Drive::GetFileCount(Folder *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(f);

    // Return file vector size
    return f->CachedFiles.size();
}

DWORD Drive::GetTotalFileCount(Folder *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read them!
        ReadDirectoryEntries(f);

    DWORD TotalCount = f->CachedFiles.size();

    // For each one of the folders within this folder, add that to our total count
    for (int i = 0; i < f->CachedFolders.size(); i++)
        TotalCount += GetTotalFileCount(f->CachedFolders.at(i));

    // Return the total count
    return TotalCount;
}

DWORD Drive::GetFolderCount(Folder *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read the dirents
        ReadDirectoryEntries(f);

    // Return file vector size
    return f->CachedFolders.size();
}

DWORD Drive::GetTotalFolderCount(Folder *f)
{
    // If we haven't yet read the dirents for this folder...
    if (!f->FatxEntriesRead)
        // Read them!
        ReadDirectoryEntries(f);

    DWORD TotalCount = f->CachedFolders.size();

    // For each one of the folders within this folder, add that to our total count
    for (int i = 0; i < f->CachedFolders.size(); i++)
        TotalCount += GetTotalFolderCount(f->CachedFolders.at(i));

    // Return the total count
    return TotalCount;
}

void Drive::DestroyFolder(Folder *Directory)
{
    while(Directory->CachedFiles.size())
    {
        File *f = Directory->CachedFiles.at(0);
        delete f;
        Directory->CachedFiles.erase(Directory->CachedFiles.begin());
    }

    while(Directory->CachedFolders.size())
    {
        Folder *f = Directory->CachedFolders.at(0);
        DestroyFolder(f);
        Directory->CachedFolders.erase(Directory->CachedFolders.begin());
    }
    delete Directory;
}

QString Drive::GetDiskName( void )
{
    // Get the Data/name.txt file
    File *name = FileFromPath(string("Data/name.txt"));
    // Open a stream to that file
    Streams::xDeviceFileStream *fs = new Streams::xDeviceFileStream(name, this);

    // Skip the first two bytes -- I have no idea what they're there for
    fs->SetPosition(2);

    // Create a new buffer to hold the name
    BYTE charArray[0x50] = {0};
    // Read the name
    fs->Read((BYTE*)&charArray, fs->Length() - 2);
    for (int i = 0; i < fs->Length() - 2; i+=2)
        fs->DetermineAndDoEndianSwap((BYTE*)&charArray + i, sizeof(short), sizeof(char));
    fs->Close();
    delete fs;
    return QString::fromUtf16((const ushort*)&charArray);
}

Folder *Drive::FolderFromPath(std::string Path)
{
    char Friendly[0x50] = {0};
    wcstombs(Friendly, FriendlyName.c_str(), FriendlyName.size());
    string cmp = Path.substr(0, Path.find('/'));
    if (cmp == string(Friendly))
        Path = Path.substr(Path.find('/') + 1);

    for (int i = 0; i < (int)ValidVolumes->size(); i++)
    {
        xVolume* activePartition = ValidVolumes->at(i);

        // Split the path so by the backslash
        vector<string> PathSplit;
        Helpers::split(Path, '/', PathSplit);

        // Match the partition name to that of the one we were given
        QRegExp reg(PathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        reg.setPatternSyntax(QRegExp::FixedString);
        if (reg.exactMatch(activePartition->Name.c_str()))
        {
            // We've found the partition, now get the root folder
            Folder *current = activePartition->Root;
            current->Dirent.ClusterStart = activePartition->RootDirectoryCluster;
            do
            {
                PathSplit.erase(PathSplit.begin());
                if (!current->FatxEntriesRead)
                {
                    ReadDirectoryEntries(current);
                }
                if (!PathSplit.size() )
                {
                    break;
                }
                bool Found = false;
                for (int j = 0; j < (int)current->CachedFolders.size(); j++)
                {
                    Folder *f = current->CachedFolders.at(j);
                    QRegExp fReg(PathSplit.at(0).c_str());
                    fReg.setCaseSensitivity(Qt::CaseInsensitive);
                    fReg.setPatternSyntax(QRegExp::FixedString);
                    if (fReg.exactMatch(f->Dirent.Name))
                    {
                        Found = true;
                        current = f;
                        break;
                    }
                }
                if (!Found)
                {
                    qDebug("Exception thrown at FolderFromPath: Folder not found");
                    throw xException("Folder not found");
                }
            }
            while (PathSplit.size() > 0);
            return current;
        }
    }
    qDebug("Exception thrown at FolderFromPath: Folder not found");
    throw xException("Folder not found");
}

File *Drive::FileFromPath(std::string Path)
{
    char Friendly[0x50] = {0};
    wcstombs(Friendly, FriendlyName.c_str(), FriendlyName.size());
    string cmp = Path.substr(0, Path.find('/'));
    // Because I'm stupid I need to convert
    if (cmp == string(Friendly))
        Path = Path.substr(Path.find('/') + 1);
    // Loop through each volume
    for (int i = 0; i < (int)ValidVolumes->size(); i++)
    {
        xVolume* activePartition = ValidVolumes->at(i);
        // Split the path so by the backslash
        vector<string> PathSplit;
        Helpers::split(Path, '/', PathSplit);
        // Match the partition name to that of the one we were given
        QRegExp reg(PathSplit.at(0).c_str());
        reg.setCaseSensitivity(Qt::CaseInsensitive);
        reg.setPatternSyntax(QRegExp::FixedString);
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
                        fReg.setPatternSyntax(QRegExp::FixedString);
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
                        fReg.setPatternSyntax(QRegExp::FixedString);
                        if (fReg.exactMatch(f->Dirent.Name))
                        {
                            return f;
                        }
                    }
                }
                if (!Found)
                {
                    qDebug("Exception thrown at FileFromPath: File not found");
                    throw xException("File not found");
                }
            }
            while (PathSplit.size() > 0);
        }
    }
    qDebug("Exception thrown at FileFromPath: File not found");
    throw xException("Folder not found");
}


// TODO: FINISH THIS FUNCTION AND MAKE SHIT WORK
void Drive::FindFreeClusters(DWORD StartingCluster, DWORD ClusterCount, xVolume* Partition, std::vector<DWORD>& OutChain)
{
    // Set the stream position to the free cluster range start
    Streams::IStream* Stream = DeviceStream;
    if (Partition->FreeClusterRangeStart != 0xFFFFFFFF)
        Stream->SetPosition(Partition->FreeClusterRangeStart);
    else
    {
        // Go to the FAT start
        Stream->SetPosition(Partition->Offset + 0x1000);
        // Start reading data to see when we hit a free cluster
        if (Partition->EntrySize == FAT32)
            // Keep looping until we hit a free cluster
            while (Stream->ReadUInt32() != FAT_CLUSTER_AVAILABLE){}
        else
            // Keep looping until we hit a free cluster
            while(Stream->ReadUInt16() != FAT_CLUSTER16_AVAILABLE){}
    }
}

void Drive::ReadDirectoryEntries(Folder* Directory)
{
    // If the cluster chain size is 0, we haven't read the cluster chain yet.  We better do that
    if (!Directory->ClusterChain.size())
    {
        ReadClusterChain(Directory->ClusterChain, Directory->Dirent, *(Directory->Volume));
    }
    // Loop for each cluster
    for (int i = 0; i < (int)Directory->ClusterChain.size(); i++)
    {
        // Set the IO's position to the start of the cluster
        DeviceStream->SetPosition(Directory->Volume->DataStart +
                        ((UINT64)(Directory->ClusterChain.at(i) - 1)   * Directory->Volume->ClusterSize));
        // Loop for the maximum amount of entries per cluster
        for (int j = 0; j < Directory->Volume->ClusterSize / 0x40; j++)
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
            DeviceStream->Read((BYTE*)&Entry.Name, Entry.NameSize);

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
                f->FullClusterChainRead = false;
                Directory->CachedFiles.push_back(f);
            }
        }
    }
    Directory->FatxEntriesRead = true;
}

void Drive::ReadClusterChain(std::vector<UINT32>& Chain, xDirent Entry, xVolume RelativePartition, int Count)
{
    // Clear the chain
    Chain.clear();
    // The int that signifies end of cluster chain
    UINT32 End = (RelativePartition.EntrySize == 2) ? FAT_CLUSTER16_LAST : FAT_CLUSTER_LAST;
    // The previous cluster
    UINT32 Previous = Entry.ClusterStart;
    // Add the base cluster to the chain
    Chain.push_back(Previous);
    // Loop
    do
    {
        // Set the IO to the FAT index
        DeviceStream->SetPosition((RelativePartition.Offset + 0x1000) + (UINT64)(Previous * RelativePartition.EntrySize));
        // Read the int there
        if (RelativePartition.EntrySize == 2)
        {
            Previous = (UINT32)DeviceStream->ReadUInt16();
        }
        else
        {
            Previous = (UINT32)DeviceStream->ReadUInt32();
        }
        // If we haven't reached the end of the chain, add the previous cluster to our chain
        if (Previous != End)
        {
            Chain.push_back(Previous);
        }
    }
    while(Previous != FAT_CLUSTER_AVAILABLE && Previous != End && Chain.size() != Count);

    if (Previous == FAT_CLUSTER_AVAILABLE)
    {
        char buffer[0x46];
        sprintf(buffer, "Free block referenced in FAT cluster chain! Dirent offset: 0x%lX", Entry.Offset);
        qDebug("Exception thrown at ReadClusterChain: Free block");
        throw xException(buffer);
    }
}

void Drive::Close( void )
{
    if (DeviceStream)
    {
        DeviceStream->Close();
        delete DeviceStream;
    }
    while (ValidVolumes->size())
    {
        if (!ValidVolumes->size())
            break;
        xVolume *x = ValidVolumes->at(0);
        DestroyFolder(x->Root);
        ValidVolumes->erase(ValidVolumes->begin());
        qDebug("Closing disk, destroying volumes");
        delete x;
    }
    delete ValidVolumes;
}

vector<string> Drive::Partitions( void )
{
    if (!_partitions.size())
    {
        for (int i = 0; i < (int)ValidVolumes->size(); i++)
        {
            _partitions.push_back(ValidVolumes->at(i)->Name);
        }
    }
    return _partitions;
}

UINT64 Drive::PartitionGetLength( string Partition )
{
    for (int i = 0; i < (int)ValidVolumes->size(); i++)
    {
        QRegExp rgx(Partition.c_str());
        rgx.setCaseSensitivity(Qt::CaseInsensitive);
        rgx.setPatternSyntax(QRegExp::FixedString);
        if (rgx.exactMatch(ValidVolumes->at(i)->Name.c_str()))
        {
            return ValidVolumes->at(i)->Size;
        }
    }
    qDebug("Exception thrown at PartitionGetLength: Partition not found");
    throw xException("Partition not found!");
}

void Drive::SetValidPartitions( void )
{
    if (!ValidVolumes)
    {
		vector<xVolume *> DiskVolumes;
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
            Dash.Name = "System Partition";
            Dash.Sector = DeviceStream->ReadUInt32();
            Dash.Size = DeviceStream->ReadUInt32();

            DevPartitions.push_back(Dash);
            DevPartitions.push_back(Content);

            for (int i = 0; i < (int)DevPartitions.size(); i++)
            {
                DevPartition temp = DevPartitions.at(i);
                xVolume *Actual = new xVolume();
                Actual->Offset = (UINT64)temp.Sector * 0x200;
                Actual->Size = temp.Size * 0x200;
                Actual->Name = temp.Name;
                DiskVolumes.push_back(Actual);
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
            Data->Size = (DeviceStream->Length() - Data->Offset);

            SysExt->Name = "System Extended";
            SysAux->Name = "System Auxiliary";
            Cache->Name = "Cache";
            Data->Name = "Data";

            SysExt->Disk = this;
            SysAux->Disk = this;
            Cache->Disk = this;
            Data->Disk = this;

            DeviceStream->SetPosition(SysExt->Offset);
            if (DeviceStream->ReadUInt32() == FatxMagic)
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
            xVolume* SysExt        = new xVolume();
            xVolume* SysAux        = new xVolume();
            xVolume* SystemPartition = new xVolume();
            xVolume* Data          = new xVolume();

            SysExt->Offset = HddOffsets::SystemExtended;
            SysAux->Offset = HddOffsets::SystemAux;
            SystemPartition->Offset = HddOffsets::SystemPartition;
            Data->Offset = HddOffsets::Data;

            SysExt->Size = HddSizes::SystemExtended;
            SysAux->Size = HddSizes::SystemAux;
            SystemPartition->Size = HddSizes::SystemPartition;
            Data->Size = DeviceStream->Length() - Data->Offset;

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
                xVolume* p = DiskVolumes.at(i);
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
                    throw xException("Bad partition FAT chain");
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

        ValidVolumes = new vector<xVolume *>();
        *ValidVolumes = DiskVolumes;
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
            qDebug("Exception thrown at InitializePartitions: Not a valid FATX file");
            throw xException("Not a valid FATX file.");
        }
    }
    SetValidPartitions();
}

INT64 Drive::GetLength( void )
{
    if (Type == DeviceDisk && !IsDevKitDrive)
        return ((INT64)Drive::GetSectorsFromSecuritySector()) * 0x200LL;
    return DeviceStream->Length();
}

DWORD Drive::GetSectorsFromSecuritySector( void )
{
    static DWORD s_sectors = 0;
    qDebug("s_sectors: 0x%LX", s_sectors);
    qDebug("s_sectors * 0x200: 0x%LX", (INT64)s_sectors * 0x200LL);
    if (s_sectors != 0)
        return s_sectors;

    // Set the stream position to the location of the security sector + 0x58 for the number of sectors
    DeviceStream->SetPosition(HddOffsets::SecuritySector + 0x58);
    // The integer here is little-endian, so switch to little-endian on our IO
    DeviceStream->SetEndianness(Streams::Little);
    s_sectors = DeviceStream->ReadUInt32();
    DeviceStream->SetEndianness(Streams::Big);
    qDebug("s_sectors: 0x%LX", s_sectors);
    qDebug("s_sectors * 0x200: 0x%LX", (INT64)s_sectors * 0x200LL);
    return s_sectors;
}

// Count leading zeroes word (but, we use a DWORD :) )
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
        qDebug("Exception thrown at FatxProcessBotSector: Invalid magic");
        throw xException("Bad magic");
    }

    DeviceStream->SetPosition(ref->Offset + 0x8);
    ref->SectorsPerCluster = DeviceStream->ReadUInt32();

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
        throw xException("FATX: found invalid sectors per cluster");
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
        qDebug("Exception thrown at FatxProcessBotSector: Volume too small to hold FAT");
        throw xException("FATX: Volume too small to hold the FAT");
    }

    Clusters -= PartitionSize;
    Clusters >>= (ShiftFactor & 0xFFFFFFFFFFFFFF);
    if (Clusters > 0xFFFFFFF)
    {
        qDebug("Exception thrown at FatxProcessBotSector: Too many clusters");
        throw xException("FATX: too many clusters");
    }

    ref->Clusters = Clusters;
    ref->AllocationTableSize = PartitionSize;
    ref->EntrySize = (Clusters < FAT_CLUSTER16_RESERVED) ? FAT16 : FAT32;
    ref->DataStart = ref->Offset + 0x1000 + PartitionSize;

    ref->Disk = this;
}
