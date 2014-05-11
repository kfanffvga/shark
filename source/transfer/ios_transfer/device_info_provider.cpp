#include "source/transfer/ios_transfer/device_info_provider.h"

#include <cassert>
#include <fstream>
#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "third_party/chromium/base/string_number_conversions.h"
#include "third_party/chromium/base/file_path.h"
#include "third_party/chromium/base/sys_string_conversions.h"
#include "third_party/common/dirent.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/libimobiledevice.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/lockdown.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/afc.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/installation_proxy.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/house_arrest.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/mobile_image_mounter.h"

#include "source/transfer/ios_transfer/ios_connector_provider.h"
#include "source/transfer/ios_transfer/ios_service_names.h"

using std::string;
using std::unique_ptr;
using std::vector;
using std::pair;
using std::make_pair;
using std::move;
using std::find_if;
using boost::assign::list_of;
using boost::replace_all;
using base::StringToInt64;
using base::SysUTF8ToWide;
using base::SysWideToUTF8;
using base::StringToInt;
using ios_transfer::ITunesCommonFolderType;

DeviceInfoProvider::DeviceInfoProvider()
    : device_(nullptr, idevice_free)
    , initialzed_(false)
{

}

DeviceInfoProvider::~DeviceInfoProvider()
{

}

bool DeviceInfoProvider::Init(const string& udid)
{
    if (udid.empty())
        return false;

    if (initialzed_)
        return false;

    device_ = IOSConnectorProvider::GetInstance()->GetDeviceConnector(udid);
    if (!device_)
        return false;

    initialzed_ = true;
    return true;
}

bool DeviceInfoProvider::GetProductInfo(ProductInfo* productInfo)
{
    if (!initialzed_ || !productInfo)
        return false;

    LockdowndClient client = 
        IOSConnectorProvider::GetInstance()->GetLockdowndCliect(
            device_.get(), "product_info");
    if (!client)
        return false;

    plist_t n = nullptr;
    auto lockDownResult = lockdownd_get_value(client.get(), nullptr, nullptr, 
                                              &n);
    if (lockDownResult != LOCKDOWN_E_SUCCESS) 
        return false;

    unique_ptr<void, void (*)(plist_t)> nodes(n, plist_free);
    if (!nodes)
        return false;

    auto getValueInNodes = [] (plist_t nodeList, const char* key) -> string
    {
        auto item = plist_dict_get_item(nodeList, key);
        if (!item)
            return "";

        char* val = nullptr;
        plist_get_string_val(item, &val);
        string result(val);
        free(val);
        return move(result);
    };

    productInfo->ProductSystemVersion = 
        getValueInNodes(nodes.get(), "ProductVersion");
    productInfo->ProductType = getValueInNodes(nodes.get(), "ProductType");
    productInfo->SerialNumber = getValueInNodes(nodes.get(), "SerialNumber");
    productInfo->UniqueDefineID = 
        getValueInNodes(nodes.get(), "UniqueDeviceID");

    return true;
}

bool DeviceInfoProvider::GetStorageInfo(StorageInfo* storageInfo)
{
    if (!initialzed_ || !storageInfo)
        return false;

    LockdowndClient client = 
        IOSConnectorProvider::GetInstance()->GetLockdowndCliect(
            device_.get(), "storage_info");
    if (!client)
        return false;

    LockdowndServiceDescriptor descriptor = 
        IOSConnectorProvider::GetInstance()->GetlockdowndServiceDescriptor(
            client.get(), IOSServiceNames::GetAFCServiceName());
    if (!descriptor)
        return false;

    return GetStorageInfoInAFCService(storageInfo, descriptor.get());
}

bool DeviceInfoProvider::GetApplicationInfo(ApplicationInfos* applicationInfo)
{
    if (!applicationInfo || !initialzed_)
        return false;

    LockdowndClient client = 
        IOSConnectorProvider::GetInstance()->GetLockdowndCliect(
            device_.get(), "application_info");
    if (!client)
        return false;

    LockdowndServiceDescriptor descriptor = 
        IOSConnectorProvider::GetInstance()->GetlockdowndServiceDescriptor(
            client.get(), IOSServiceNames::GetApplicationProxyServiceName());
    
    if (!descriptor)
        return false;


    return GetApplicationInfoInApplicationProxyService(applicationInfo, 
                                                       descriptor.get());
}

bool DeviceInfoProvider::GetStorageInfoInAFCService(
    StorageInfo* storageInfo, lockdownd_service_descriptor* desc)
{
    if (!storageInfo || !desc || (desc->port <= 0))
        return false;

    assert(device_.get());
    afc_client_t c = nullptr;
    auto afcResult = afc_client_new(device_.get(), desc, &c);
    if (afcResult != AFC_E_SUCCESS)
        return false;

    unique_ptr<afc_client_private, int16 (*)(afc_client_private*)> 
        afcClient(c, afc_client_free);

    char** info = nullptr;
    if (afc_get_device_info(afcClient.get(), &info) != AFC_E_SUCCESS)
        return false;

    if (!info)
        return false;

    for (int i = 0; i < 8; i += 2)
    {
        string key(info[i]);
        string value(info[i + 1]);
        if ("FSTotalBytes" == key)
        {
            if (!StringToInt64(value, &(storageInfo->TotalSpace)))
                storageInfo->TotalSpace = 0;
        }
        else if ("FSFreeBytes" == key)
        {
            if (!StringToInt64(value, &(storageInfo->FreeSpace)))
                storageInfo->FreeSpace = 0;
        }
    }
    free(info);
    return true;

}

bool DeviceInfoProvider::GetApplicationInfoInApplicationProxyService(
    ApplicationInfos* applicationInfos, lockdownd_service_descriptor* desc)
{
    if (!applicationInfos || !desc || (desc->port <= 0))
        return false;

    instproxy_client_t c = nullptr;
    auto result = instproxy_client_new(device_.get(), desc, &c);
    if (result != INSTPROXY_E_SUCCESS)
        return false;

    unique_ptr<instproxy_client_private, int16 (*)(instproxy_client_private*)>
        client(c, instproxy_client_free);

    unique_ptr<void, void (*)(void*)> applicationQueryOptions(
        instproxy_client_options_new(), instproxy_client_options_free);
    if (!applicationQueryOptions)
        return false;

    instproxy_client_options_add(applicationQueryOptions.get(), 
                                 "ApplicationType", "User", nullptr);
//     instproxy_client_options_add(applicationQueryOptions.get(), 
//                                  "ApplicationType", "System", nullptr);

    plist_t a = nullptr;
    result = instproxy_browse(client.get(), applicationQueryOptions.get(), &a);
    if (result != INSTPROXY_E_SUCCESS)
        return false;

    unique_ptr<void, void (*)(void*)> appInfos(a, plist_free);
    uint32 size = plist_array_get_size(appInfos.get());
    for (uint32 i = 0; i < plist_array_get_size(appInfos.get()); ++i)
    {
        auto item = plist_array_get_item(appInfos.get(), i);
        FillApplicationInfo(item, applicationInfos);
    }
    return true;
}

void DeviceInfoProvider::FillApplicationInfo(void* applicationInfoPtr, 
                                             ApplicationInfos* applicationInfos)
{
    if (!applicationInfoPtr)
        return ;

    auto fillValueByAppItem = 
        [] (string* fillIn, plist_t appInfo, const string& key)
    {
        if (!fillIn || !appInfo)
            return ;

        *fillIn = "";

        auto appItem = plist_dict_get_item(appInfo, key.c_str());
        if (!appItem)
            return;

        char* value = nullptr;
        plist_get_string_val(appItem, &value);
        if (value)
            *fillIn = value;

        free(value);
    };
    SingleApplicationInfo singleAppInfo;
    fillValueByAppItem(&(singleAppInfo.DisplayName), applicationInfoPtr, 
                       "CFBundleDisplayName");
    fillValueByAppItem(&(singleAppInfo.ID), applicationInfoPtr, 
                       "CFBundleIdentifier");
    fillValueByAppItem(&(singleAppInfo.Version), applicationInfoPtr, 
                       "CFBundleVersion");

    char* xml = nullptr;
    uint32 length = 0;
    plist_to_xml(applicationInfoPtr, &xml, &length);
    singleAppInfo.DetailInfo = string(xml, length);
    free(xml);

    // TODO fix 应用图标有问题，暂时没能想到什么好的解决方法，因此把图标部分去除，将来等修复
    // 图标的获取方式，才再把图标复原
//     auto iconFileNames = GetApplicationIconFileNames(applicationInfoPtr, 
//                                                      singleAppInfo.ID);
//     singleAppInfo.Icon = GetApplicationIconByIconFileNames(iconFileNames, 
//                                                            singleAppInfo.ID);
//     iconFileNames.push_back("acdf.png");
//     auto aa = GetApplicationIconByIconFileNames(iconFileNames, "com.apple.videos");
    applicationInfos->push_back(move(singleAppInfo));
}

vector<string> DeviceInfoProvider::GetApplicationIconFileNames(
    void* appInfoPtr, const string& applicationID)
{
    vector<string> result;
    auto iconRootDict = plist_dict_get_item(appInfoPtr, "CFBundleIcons");
    if (!iconRootDict)
        return move(result);

    auto primaryIconDict = plist_dict_get_item(iconRootDict, 
                                               "CFBundlePrimaryIcon");
    if (!primaryIconDict)
        return move(result);

    auto iconFiles = plist_dict_get_item(primaryIconDict, "CFBundleIconFiles");
    if (!iconFiles)
        return move(result);

    for (uint32 i = 0; i < plist_array_get_size(iconFiles); ++i)
    {
        auto iconItem = plist_array_get_item(iconFiles, i);
        if (!iconItem)
            continue;

        char* value = nullptr;
        plist_get_string_val(iconItem, &value);
        if (!value)
            continue;

        do {
            string fileName(value);
            if (fileName.find("png", 0) == string::npos)
                break;

            result.push_back(string(value));
        } while (false);
        free(value);
    }
    return move(result);
}

pair<unique_ptr<char>, int> 
    DeviceInfoProvider::GetApplicationIconByIconFileNames(
        const vector<string>& fileNames, const string& applicationID)
{
    auto result = make_pair(unique_ptr<char>(), 0);
    if (fileNames.empty())
        return move(result);

    LockdowndClient client = 
        IOSConnectorProvider::GetInstance()->GetLockdowndCliect(
            device_.get(), "application_icon_info");
    if (!client)
        return move(result);

    LockdowndServiceDescriptor descriptor = 
        IOSConnectorProvider::GetInstance()->GetlockdowndServiceDescriptor(
            client.get(), IOSServiceNames::GetHouseArrestServiceName());
    if (!descriptor)
        return move(result);

    HouseArrestClient houseArrestClient = 
        IOSConnectorProvider::GetInstance()->GetHouseArrestClient(
        device_.get(),  descriptor.get(), 
                                                   applicationID);
    if (!houseArrestClient.get())
        return move(result);

    afc_client_t afcc = 
        IOSConnectorProvider::GetInstance()->GetAFCClientByHouseArrsetClient(
            houseArrestClient.get());
    if (!afcc)
        return move(result);

    auto filesProperties = GetApplicationFilesProperties(afcc, "/");
    if (filesProperties.empty())
        return move(result);

    for (auto i = fileNames.begin(); i != fileNames.end(); ++i)
    {
        auto fileNameMatchCondition = 
            [i] (const DeviceInfoProvider::FileProperties& properties) -> bool
        {
            return properties.FilePath.find(*i) != string::npos;
        };
        auto prop = find_if(filesProperties.begin(), filesProperties.end(), 
                            fileNameMatchCondition);
        if (filesProperties.end() == prop)
            continue;

        uint64 fileHandle = 0;
        do {
            auto error = afc_file_open(afcc, prop->FilePath.c_str(), 
                                       AFC_FOPEN_RDONLY, &fileHandle);
            if (error != AFC_E_SUCCESS)
                break;

            uint32 totalRead = 0;
            result.first.reset(new char[prop->Size]);
            result.second = prop->Size;
            do {
                uint32 readSize = 0;
                error = afc_file_read(afcc, fileHandle, 
                                      result.first.get() + totalRead, 
                                      result.second, &readSize);
                totalRead += readSize;
            } while ((AFC_E_SUCCESS == error) && 
                     (totalRead < static_cast<uint32>(result.second)));
        } while (false);

        if (fileHandle != 0)
            afc_file_close(afcc, fileHandle);
    }

    return move(result);
}

vector<DeviceInfoProvider::FileProperties> 
    DeviceInfoProvider::GetApplicationFilesProperties(
        afc_client_private* afcClient, const string& directory)
{
    vector<FileProperties> result;
    GetApplicationFilesPropertiesRecursive(afcClient, directory, &result);
    return move(result);
}

void DeviceInfoProvider::GetApplicationFilesPropertiesRecursive(
    afc_client_private* afcClient, const string& directory, 
    vector<FileProperties>* props)
{
    if ((!props) || (!afcClient))
        return;

    char** fileList = nullptr;
    auto error = 
        afc_read_directory(afcClient, directory.c_str(), &fileList);
    if (error != AFC_E_SUCCESS)
        return ;

    for (uint32 i = 0; fileList[i]; ++i)
    {
        string name(fileList[i]);
        if ((name == ".") || (name == ".."))
            continue;

        FilePath path(SysUTF8ToWide(directory));
        path = path.Append(SysUTF8ToWide(name));
        FileProperties prop = {SysWideToUTF8(path.value()), 0, 0};
        replace_all(prop.FilePath, "\\", "/");
        char** properties = nullptr;
        error = 
            afc_get_file_info(afcClient, prop.FilePath.c_str(), &properties);
        if ((error != AFC_E_SUCCESS) || (!properties))
            continue;
        
        for (uint32 j = 0; properties[j]; j += 2)
        {
            auto keyValue = 
                make_pair(string(properties[j]), string(properties[j + 1]));
            if ("st_size" == keyValue.first)
            {
                StringToInt(keyValue.second, 
                            reinterpret_cast<int*>(&prop.Size));
            }
            else if ("st_ifmt" == keyValue.first)
            {
                const static vector<pair<string, uint32>> fileTypes = 
                    list_of(make_pair("S_IFREG", S_IFREG))
                    (make_pair("S_IFDIR", S_IFDIR)) 
                    (make_pair("S_IFLNK", S_IFLNK)) 
                    (make_pair("S_IFBLK", S_IFBLK)) 
                    (make_pair("S_IFCHR", S_IFCHR)) 
                    (make_pair("S_IFIFO", S_IFIFO)) 
                    (make_pair("S_IFSOCK", S_IFSOCK));
                for (auto fileType = fileTypes.begin(); 
                    fileType != fileTypes.end(); ++fileType)
                {
                    if (fileType->first == keyValue.second)
                    {
                        prop.Type = fileType->second;
                        break;
                    }
                }
            }
        }
        free(properties);
        props->push_back(prop);
        if (S_IFDIR == prop.Type)
            GetApplicationFilesPropertiesRecursive(afcClient, prop.FilePath, 
                                                   props);
    }
    free(fileList);
}

bool DeviceInfoProvider::GetITunesCommonFolders(
    vector<ITunesCommonFolderType>* types)
{
    LockdowndClient client = 
        IOSConnectorProvider::GetInstance()->GetLockdowndCliect(
            device_.get(), "itunes_common_folder");
    if (!client)
        return false;

    LockdowndServiceDescriptor descriptor = 
        IOSConnectorProvider::GetInstance()->GetlockdowndServiceDescriptor(
            client.get(), IOSServiceNames::GetImageMounterServiceName());
        
    if (!descriptor)
        return false;

    LockdowndServiceDescriptor afccServiceDesc = 
        IOSConnectorProvider::GetInstance()->GetlockdowndServiceDescriptor(
            client.get(), IOSServiceNames::GetAFCServiceName());
    afc_client_t afcc = nullptr;
    auto r = afc_client_new(device_.get(), afccServiceDesc.get(), &afcc);
    if (AFC_E_SUCCESS == r)
    {
//         auto files = GetApplicationFilesProperties(afcc, "/");
//         if (files.empty())
//             return false;

    }

    // 此处要将数据库取出来，才知道有什么东西
    return true;
}
