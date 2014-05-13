#include "source/transfer/ios_transfer/device_manager.h"

#include <algorithm>

#include "third_party/chromium/base/utf_string_conversions.h"
#include "third_party/lib_ios_mobile_device/include/libimobiledevice/libimobiledevice.h"

#include "source/transfer/ios_transfer/class_provider.h"
#include "source/transfer/ios_transfer/ios_interface.h"
#include "source/transfer/ios_transfer/device_info_provider.h"

using std::string;
using std::wstring;
using std::pair;
using std::find_if;
using std::for_each;
using std::make_pair;
using std::unique_ptr;
using std::shared_ptr;
using std::max;
using base::AutoLock;

namespace 
{

void ListenDeviceInOrOut(const idevice_event_t* e, void* userData)
{
    bool isIn = (IDEVICE_DEVICE_ADD == e->event);
    reinterpret_cast<DeviceManager*>(userData)->ReceiveDeviceInOrOutInfo(
        isIn, e->udid, e->conn_type);
}

}

DeviceManager* DeviceManager::GetInstance()
{
    return Singleton<DeviceManager>::get();
}

DeviceManager::DeviceManager()
    : deviceInfos_()
    , initialzed_(false)
    , deviceInfoLock_()
{
    
}

DeviceManager::~DeviceManager()
{
    idevice_event_unsubscribe();
}

void DeviceManager::Init()
{
    if (!initialzed_)
    {
        idevice_set_debug_level(1);
        idevice_event_subscribe(ListenDeviceInOrOut, this);
    }
    initialzed_ = true;
}

void DeviceManager::ReceiveDeviceInOrOutInfo(bool isIn, const string& udid, 
                                             int connectType)
{
    if (!isIn)
        RemoveDevice(udid);
    else
        AddDevice(udid);
}

void DeviceManager::RemoveDevice(const string& udid)
{
    typedef pair<int, shared_ptr<DeviceInfo>> DeviceItem;
    auto condition = [udid] (const DeviceItem& item) -> bool
    {
        return item.second->productInfo.UniqueDefineID == udid;
    };
    auto result = find_if(deviceInfos_.begin(), deviceInfos_.end(), condition);
    if (result != deviceInfos_.end())
    {
        auto deviceID = result->first;
        {
            AutoLock l(deviceInfoLock_);
            deviceInfos_.erase(result);
        }
        auto notifition = ClassProvider::GetInstance()->GetDeviceNotifition();
        if (notifition)
            notifition->NotifyDeviceRemove(deviceID);
    }
}

void DeviceManager::AddDevice(const string& udid)
{
    shared_ptr<DeviceInfo> info(new DeviceInfo());

    if (!GetDeviceInfo(udid, info.get()))
        return;

    typedef pair<int, shared_ptr<DeviceInfo>> DeviceItem;
    auto condition = [udid] (const DeviceItem& item) -> bool
    {
        return item.second->productInfo.UniqueDefineID == udid;
    };
    AutoLock l(deviceInfoLock_);
    auto result = find_if(deviceInfos_.begin(), deviceInfos_.end(), condition);
    if (deviceInfos_.end() == result)
    {
        int maxID = -1;
        auto procedure = [&maxID] (const DeviceItem& item) 
        { 
            maxID = max(maxID, item.first); 
        };
        for_each(deviceInfos_.begin(), deviceInfos_.end(), procedure);
        
        ++maxID;
        deviceInfos_.insert(make_pair(maxID, info));
        auto notifition = ClassProvider::GetInstance()->GetDeviceNotifition();
        if (notifition)
        notifition->NotifyDeviceArrive(maxID);
    }
}

bool DeviceManager::GetDeviceInfo(const string& udid, DeviceInfo* info)
{
    if (!info)
        return false;

    DeviceInfoProvider provider;
    if (!provider.Init(udid))
        return false;

    if (!provider.GetProductInfo(&(info->productInfo)))
        return false;

    if (!provider.GetStorageInfo(&(info->storageInfo)))
        return false;

    if (!provider.GetApplicationInfo(&(info->applicationInfos)))
        return false;

    if (!provider.GetITunesCommonFolders(&(info->itunesCommonFolderTypes)))
        return false;

    return true;
}

shared_ptr<DeviceInfo> DeviceManager::GetDeviceInfoByDeviceID(int deviceID)
{
    AutoLock l(deviceInfoLock_);
    auto result = deviceInfos_.find(deviceID);
    if (deviceInfos_.end() == result)
        return shared_ptr<DeviceInfo>();

    return result->second;
}
