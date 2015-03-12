#include "pcutils.h"
#include <vector>
#include <sstream>
#include "IO/xDeviceStream.h"
#ifdef _WIN32
    #include <windows.h>
    #include <devguid.h>    // for GUID_DEVCLASS_CDROM etc
    #include <setupapi.h>
    #include <cfgmgr32.h>   // for MAX_DEVICE_ID_LEN, CM_Get_Parent and CM_Get_Device_ID
    #define INITGUID
    #include <DEVPKEY.h>

    #define ARRAY_SIZE(arr)     (sizeof(arr)/sizeof(arr[0]))

    #pragma comment (lib, "setupapi.lib")

#else
    #include <dirent.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/stat.h>

    #ifdef __APPLE__
        #include <sys/disk.h>
        #include <sys/ioctl.h>
    #endif

    // linux
    #ifdef __linux
        #include <linux/hdreg.h>
    #endif
#endif

std::vector<Drive *> PCUtils::GetFATXDrives( bool IncludeHardDisks )
{
    std::vector<Drive *> ReturnVector;
    if (IncludeHardDisks)
    {
        // Initialize the disk information vector
        std::vector<DISK_DRIVE_INFORMATION> Disks;
        // Get the hard disks
        PCUtils::GetPhysicalDisks(Disks);

        Streams::xDeviceStream* DS = nullptr;
        for (int i = 0; i < (int)Disks.size(); i++)
        {
            DISK_DRIVE_INFORMATION ddi = Disks[i];
            // First, try reading the disk way
            try
            {
                DS = new Streams::xDeviceStream(std::string(ddi.Path));
            }
            catch (...)
            {
                qDebug("Disk %s is bad", ddi.FriendlyName.c_str());
                DS = nullptr;
                continue;
            }

            if (DS == nullptr || DS->Length() == 0 || DS->Length() < HddOffsets::Data)
            {
                qDebug("Disk %s is bad", ddi.FriendlyName.c_str());
                DS = nullptr;
                // Disk is not of valid length
                continue;
            }
            DS->SetPosition(HddOffsets::Data);

            // Read the FATX partition magic
            INT32 Magic = DS->ReadInt32();
            // Close the stream
            DS->Close();
            delete DS;
            DS = NULL;

            // Compare the magic we read to the *actual* FATX magic
            if (Magic == FatxMagic)
            {
                qDebug("Disk %s is good!", ddi.FriendlyName.c_str());
                Drive* d = new Drive(ddi.Path, ddi.FriendlyName, false);
                ReturnVector.push_back(d);
            }
            else
                qDebug("Disk %s had bad magic (0x%X)", ddi.FriendlyName.c_str(), Magic);
        }
        if (DS != nullptr)
        {
            DS->Close();
            delete DS;
        }
    }

    std::vector<Drive *> LogicalDisks = GetLogicalPartitions();
    for (unsigned int i = 0; i < LogicalDisks.size(); i++)
    {
        ReturnVector.push_back(LogicalDisks[i]);
    }

    return ReturnVector;
}

void PCUtils::GetPhysicalDisks( std::vector<PCUtils::DISK_DRIVE_INFORMATION>& OutVector )
{
#ifdef _WIN32
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

    // List all connected disk drives
    hDevInfo = SetupDiGetClassDevs (&HddClass, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hDevInfo == INVALID_HANDLE_VALUE)
        return;

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

        AddToVector.Path = nowide::convert(std::wstring(data->DevicePath));
        qDebug("Disk path: %s", AddToVector.Path.c_str());

        // Friendly name (e.g. SanDisk Cruzer USB...)
        SetupDiGetDeviceRegistryProperty (hDevInfo, &DeviceInfoData, SPDRP_FRIENDLYNAME,
                                          &dwPropertyRegDataType, (BYTE*)szDesc,
                                          sizeof(szDesc),   // The size, in bytes
                                          &dwSize);
        AddToVector.FriendlyName = nowide::convert(std::wstring((TCHAR*)szDesc));
        qDebug("Friendly name: %s", AddToVector.FriendlyName.c_str());

        OutVector.push_back(AddToVector);
        delete data;
    }
#else
DIR *dir = NULL;
dirent *ent = NULL;
dir = opendir("/dev/");
if (dir != NULL)
{
    // Read the shit
    while ((ent = readdir(dir)) != NULL)
    {
        // Check the directory name, and if it starts with "disk" then keep it!
#ifdef __linux
        QRegExp exp("sd[a-z]");
#else
        QRegExp exp("disk*");
#endif
        exp.setPatternSyntax(QRegExp::Wildcard);
        exp.setCaseSensitivity(Qt::CaseInsensitive);
        if (exp.exactMatch(ent->d_name))
        {
            DISK_DRIVE_INFORMATION curdir;

            std::ostringstream ss;
#ifdef __linux
            ss << "/dev/";
#else
            ss << "/dev/r";
#endif
            ss << ent->d_name;
            std::string diskPath = ss.str();

            curdir.Path = diskPath;

            int device;
            if ((device = open(diskPath.c_str(), O_RDONLY)) > 0)
            {
#ifdef __linux
                hd_driveid hd;
                if (!ioctl(device, HDIO_GET_IDENTITY, &hd))
                {
                    curdir.FriendlyName = std::string((const char *)hd.model);
                }
#elif defined __APPLE__
                curdir.FriendlyName = ent->d_name;
#endif
                OutVector.push_back(curdir);
            }
        }
    }
}
if (dir)
    closedir(dir);
if (ent)
    delete ent;
#endif
}

std::vector<Drive *> PCUtils::GetLogicalPartitions( void )
{
    std::vector<Drive *> ReturnVector;

#ifdef _WIN32
    const DWORD LettersSize = (26 * 6) + 1;

    TCHAR Letters[LettersSize] = {0}; // 26 for every leter, multiplied by 3 to include the semi colon/backslash, then 2 for each null byte, plus 1 for the last null

if (!GetLogicalDriveStrings(LettersSize, Letters))
{
    int x = GetLastError();
    return ReturnVector;
}

TCHAR VolumeName[MAX_PATH + 1] = {0};

for (int i = 0; i < 26; i+= 4)
{
    memset(&VolumeName, 0, sizeof(VolumeName));
    GetVolumeInformation(
                &Letters[i],	// Path name
                VolumeName,		// Volume name
                MAX_PATH + 1,	// Volume name length
                NULL,
                NULL,
                NULL,
                NULL,
                NULL);
    std::ostringstream ss;
    ss << nowide::convert(&Letters[i]);
    ss << "Xbox360\\Data0000";
    std::string path(ss.str());

    try
    {
        Streams::xFileStream *tempFile = new Streams::xFileStream(path, Streams::Open);
        tempFile->Close();
        delete tempFile;
        // Stream opened with no exception, we're good!
        Drive *d = new Drive(nowide::convert(&Letters[i]), nowide::convert(std::wstring(VolumeName)), true);
        ReturnVector.push_back(d);
    }
    catch(...)
    {
        // stream opened with an exception, the file doesn't exist (not a valid Xbox 360-formatted drive)
    }
}
#else
// First, enumerate the disks
DIR *dir = NULL;
dirent *ent = NULL;
#ifdef __APPLE__
dir = opendir("/Volumes/");
#else
dir = opendir("/media/");
#endif
if (dir != NULL)
{
    // Read the shit
    while ((ent = readdir(dir)) != NULL)
    {
        DISK_DRIVE_INFORMATION curdir;

        // Create a temporary buffer to hold our disk name
        std::stringstream path;
#ifdef __APPLE__
        path << "/Volumes/";
#else
        path << "/media/";
#endif
        path << ent->d_name;
        path << "/";

        curdir.Path = path.str();
        try
        {
            // Reset the stringstream
            path.str(std::string());
            path << curdir.Path;
            path << "Xbox360/Data0000";
            Streams::xFileStream *x = new Streams::xFileStream(path.str(), Streams::Open);
            // Stream opened good, close it
            x->Close();
            delete x;
#ifdef __APPLE__
            if (curdir.Path == "/Volumes/.")
#else
            if (curdir.Path == "/media/.")
#endif
            {
                curdir.FriendlyName = "OS Root";
            }
            else
            {
                curdir.FriendlyName = curdir.Path.substr(9);
            }
            Drive *d = new Drive(curdir.Path, curdir.FriendlyName, true);
            ReturnVector.push_back(d);
        }
        catch(...)
        {
            // Couldn't open the device, just continue
        }
    }
    if (dir)
        closedir(dir);
    if (ent)
        delete ent;
}
#endif

return ReturnVector;
}
