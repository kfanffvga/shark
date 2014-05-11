#ifndef _DEVICE_INFO_PROVIDER_H_
#define _DEVICE_INFO_PROVIDER_H_

#include <memory>
#include <string>
#include <vector>
#include <utility>

#include "third_party/chromium/base/basictypes.h"

#include "source/transfer/ios_transfer/ios_interface.h"

struct idevice_private;
struct afc_client_private;

struct ProductInfo 
{
    std::string SerialNumber;
    std::string UniqueDefineID;
    std::string ProductType;
    std::string ProductSystemVersion;
};

struct StorageInfo 
{
    int64 FreeSpace;
    int64 TotalSpace;
};

struct SingleApplicationInfo
{
    std::string ID;
    std::string DisplayName;
    std::string Version;
    std::pair<std::shared_ptr<char>, int> Icon;
    std::string DetailInfo;
};

typedef std::vector<SingleApplicationInfo> ApplicationInfos;

struct DeviceInfo
{
    StorageInfo storageInfo;
    ProductInfo productInfo;
    ApplicationInfos applicationInfos;
    std::vector<ios_transfer::ITunesCommonFolderType> itunesCommonFolderTypes;
};

struct lockdownd_service_descriptor;
struct lockdownd_client_private;
struct afc_client_private;
struct house_arrest_client_private;

class DeviceInfoProvider
{
public:
    DeviceInfoProvider();
    ~DeviceInfoProvider();

    bool Init(const std::string& udid);

    bool GetProductInfo(ProductInfo* productInfo);
    bool GetStorageInfo(StorageInfo* storageInfo);
    bool GetApplicationInfo(ApplicationInfos* applicationInfo);
    bool GetITunesCommonFolders(
        std::vector<ios_transfer::ITunesCommonFolderType>* types);

private:
    struct FileProperties
    {
        std::string FilePath;
        uint32 Type;
        uint32 Size;
    };

    bool GetStorageInfoInAFCService(StorageInfo* storageInfo, 
                                    lockdownd_service_descriptor* desc);

    bool GetApplicationInfoInApplicationProxyService(
        ApplicationInfos* applicationInfos, lockdownd_service_descriptor* desc);

    void FillApplicationInfo(void* applicationInfoPtr, 
                             ApplicationInfos* applicationInfos);

    std::vector<std::string> GetApplicationIconFileNames(
        void* appInfoPtr, const std::string& applicationID);

    std::pair<std::unique_ptr<char>, int> GetApplicationIconByIconFileNames(
        const std::vector<std::string>& fileNames, 
        const std::string& applicationID);

    std::vector<FileProperties> GetApplicationFilesProperties(
        afc_client_private* afcClient, const std::string& directory);

    void GetApplicationFilesPropertiesRecursive(
        afc_client_private* afcClient, const std::string& directory,
        std::vector<FileProperties>* props);

    std::unique_ptr<idevice_private, int16 (*)(idevice_private*)> device_;
    bool initialzed_;
};

#endif